/*******************************************************************************/
/* This file has been modified by Sergio Perez Alcañiz <serpeal@upvnet.upv.es> */
/*            Departamento de Informática de Sistemas y Computadores           */
/*            Universidad Politécnica de Valencia                              */
/*            Valencia (Spain)                                                 */
/*            Date: April 2003                                                 */
/*******************************************************************************/

#include "lwip/ip_addr.h"

#ifndef _NETDB_H
#define _NETDB_H

/* Description of data base entry for a single service.  */
struct protoent {
    char *p_name;		/* Official protocol name.  */
    char **p_aliases;		/* Alias list.  */
    int p_proto;		/* Protocol number.  */
};

struct protoent *getprotobyname (const char *name);
struct protoent *getprotobynumber (int proto);

struct servent {
    char *s_name;
    char **s_aliases;
    int s_port;
    char *s_proto;
};

#define HOST_NOT_FOUND		ENSRNXDOMAIN
#define NO_ADDRESS		ENSRNODATA
#define NO_DATA			ENSRNODATA
#define TRY_AGAIN		ENSRTIMEOUT

#define h_addr		h_addr_list[0]
struct hostent {
    char *h_name;
    char **h_aliases;
    int h_addrtype;
    int h_length;
    char **h_addr_list;
};

extern int h_errno;
struct hostent *gethostbyname (const char *name);
struct hostent *gethostbyaddr (const char *addr, int len, int type);
#if 0
#define herror perror
#define h_errno errno
#define hstrerror strerror
#endif

void setnameservers (const struct in_addr *ns1, const struct in_addr *ns2);
void getnameservers (struct in_addr *ns1, struct in_addr *ns2);

#endif	/* _NETDB_H */
