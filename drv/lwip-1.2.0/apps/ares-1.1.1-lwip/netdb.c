/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * CHANGELOG: this file has been modified by Sergio Perez Alcañiz <serpeal@upvnet.upv.es> 
 *            Departamento de Informática de Sistemas y Computadores          
 *            Universidad Politécnica de Valencia                             
 *            Valencia (Spain)    
 *            Date: April 2003                                          
 *
 */

#include "ares_private.h"
#include "lwip/arch.h"
#define	ENSROK		0
#define rtl_printf printk

static struct {
  unsigned short errno2, ares_errno;
} ares_errno_trans[] = {
    {
    ENSROK, ARES_SUCCESS}, {
    ENSRNODATA, ARES_ENODATA}, {
    ENSRFORMERR, ARES_EFORMERR}, {
    ENSRSERVFAIL, ARES_ESERVFAIL}, {
    ENSRNOTFOUND, ARES_ENOTFOUND}, {
    ENSRNOTIMP, ARES_ENOTIMP}, {
    ENSRREFUSED, ARES_EREFUSED}, {
    ENSRBADQUERY, ARES_EBADQUERY}, {
    ENSRBADNAME, ARES_EBADNAME}, {
    ENSRBADFAMILY, ARES_EBADFAMILY}, {
    ENSRBADRESP, ARES_EBADRESP}, {
    ENSRCONNREFUSED, ARES_ECONNREFUSED}, {
    ENSRTIMEOUT, ARES_ETIMEOUT}, {
    ENSROF, ARES_EOF}, {
    ENSRFILE, ARES_EFILE}, {
    ENSRNOMEM, ARES_ENOMEM}, {
    ENSRDESTRUCTION, ARES_EDESTRUCTION}
};

static int ares_error_to_errno (int e)
{
    int i;
    for (i = 0; i < (sizeof(ares_errno_trans) / sizeof(ares_errno_trans[0])); i++)
	if (ares_errno_trans[i].ares_errno == e)
	    return ares_errno_trans[i].errno2;
    rtl_printf ("no such ares error");
    //abort ();	/* prevents warning */
    return -1;
}

struct gethost_result {
    struct hostent host;
    int status;
    char *cname;
};

#define array_dup(r,a,len_func) 					\
    do {								\
	int __i;							\
	for (__i = 0; (a)[__i]; __i++);					\
	(r) = malloc ((__i + 1) * sizeof (char *));			\
	(r)[__i] = 0;							\
	while (__i--) {							\
	    char *__p;							\
	    int __l;							\
	    __p = (a)[__i];						\
	    __l = len_func;						\
	    (r)[__i] = memcpy ((char *) malloc (__l), __p, (__l));	\
	}								\
    } while (0)

#define array_free(a) 							\
    do {								\
	int __i;							\
	for (__i = 0; (a)[__i]; __i++)					\
	    free ((a)[__i]);						\
	free (a);							\
    } while (0)

/* copy the host entry */
static void callback (void *arg, int status, struct hostent *host)
{
    struct gethost_result *r = (struct gethost_result *) arg;

    if (host) {
	if (status == ARES_SUCCESS)
	    if (r->cname)	/* we got back a cname, so retry the query with the cname */
		if (strcasecmp (r->cname, host->h_name)) {
		    free (r->cname);
		    r->cname = strdup (host->h_name);
		    r->status = ARES_SUCCESS;
		    return;
		}

/* clear any old cname */
	if (r->cname) {
	    free (r->cname);
	    r->cname = 0;
	}

/* copy alias list */
	array_dup (r->host.h_aliases, host->h_aliases, strlen (__p) + 1);

/* copy address list */
	array_dup (r->host.h_addr_list, host->h_addr_list, host->h_length);

/* copy tidbits */
	r->host.h_name = (char *) strdup (host->h_name);
	r->host.h_addrtype = host->h_addrtype;
	r->host.h_length = host->h_length;
    }

/* get status */
    r->status = status;
}

static struct in_addr __nameservers[2];

void setnameservers (const struct in_addr *ns1, const struct in_addr *ns2)
{
    __nameservers[0].s_addr = ns1->s_addr;
    __nameservers[1].s_addr = ns2->s_addr;
}

void getnameservers (struct in_addr *ns1, struct in_addr *ns2)
{
     ns1->s_addr = __nameservers[0].s_addr;
     ns2->s_addr = __nameservers[1].s_addr;
}

#ifndef DNS_MAXDOMAIN
#define DNS_MAXDOMAIN 255
#endif

static struct hostent *gethostbynameaddr (const char *name, struct in_addr addr, struct gethost_result *r)
{
    char cname[DNS_MAXDOMAIN + 1];
    ares_channel channel;
    int status, nfds, cname_loops = 0;
    fd_set read_fds, write_fds;
    struct timeval *tvp, tv;
    struct ares_options o;

    if (!__nameservers[0].s_addr) {
	rtl_printf ("The nameservers have not been set - use the setnameservers() function\n");
	errno = EINVAL;
	return NULL;
    }

    o.servers = __nameservers;
    o.nservers = 1;
    if (__nameservers[1].s_addr)
	o.nservers++;

    status = ares_init_options (&channel, &o, ARES_OPT_SERVERS);
    if (status != ARES_SUCCESS) {
	errno = ares_error_to_errno (status);
	return NULL;
    }

    r->status = -1;
    if (r->cname)
	free (r->cname);
    r->cname = strdup (name);
    if (name) {
	if (strlen (name) > DNS_MAXDOMAIN) {
	    ares_destroy (channel);
	    errno = ENSRQUERYDOMAINTOOLONG;
	    return NULL;
	}
	ares_gethostbyname (channel, name, AF_INET, callback, (void *) r);
    } else {
	ares_gethostbyaddr (channel, &addr, sizeof (addr), AF_INET, callback, (void *) r);
    }

    /* Wait for all queries to complete. */
    while (1) {
	FD_ZERO (&read_fds);
	FD_ZERO (&write_fds);
	nfds = ares_fds (channel, &read_fds, &write_fds);
	if (nfds == 0) {
	    if (name && r->cname && r->status == ARES_SUCCESS) {
		if (++cname_loops > 10) {
		    errno = ENSRCNAMELOOP;
		    ares_destroy (channel);
		    return NULL;
		}
		strncpy (cname, r->cname, DNS_MAXDOMAIN);
		cname[DNS_MAXDOMAIN] = '\0';
		name = cname;
		ares_gethostbyname (channel, name, AF_INET, callback, (void *) r);
		continue;
	    }
	    break;
	}
	tvp = ares_timeout (channel, NULL, &tv);
	select (nfds, &read_fds, &write_fds, NULL, tvp);
	ares_process (channel, &read_fds, &write_fds);
    }

    ares_destroy (channel);
    if (r->status != ARES_SUCCESS || r->status == -1) {
	errno = r->status == -1 ? ARES_ENOTFOUND : ares_error_to_errno (r->status);
	return NULL;
    }
    return &r->host;
}

struct hostent *gethostbyname (const char *name)
{
    struct in_addr addr;
    static struct gethost_result r;
    static int init = 0;
    if (!init) {
	init = 1;
	memset (&r, 0, sizeof (r));
    }
    addr.s_addr = 0;
    if (r.host.h_aliases)
	array_free (r.host.h_aliases);
    if (r.host.h_addr_list)
	array_free (r.host.h_addr_list);
    if (r.host.h_name)
	free (r.host.h_name);
    return gethostbynameaddr (name, addr, &r);
}

struct hostent *gethostbyaddr (const char *addr, int len, int type)
{
    static struct gethost_result r;
    static int init = 0;
    if (!init) {
	init = 1;
	memset (&r, 0, sizeof (r));
    }
    if (len != sizeof (struct in_addr) || type != AF_INET) {
	errno = EINVAL;
	return 0;
    }
    if (r.host.h_aliases)
	array_free (r.host.h_aliases);
    if (r.host.h_addr_list)
	array_free (r.host.h_addr_list);
    if (r.host.h_name)
	free (r.host.h_name);
    return gethostbynameaddr (0, *((struct in_addr *) addr), &r);
}

