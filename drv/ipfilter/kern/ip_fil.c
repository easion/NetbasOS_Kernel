/*
 * Copyright (C) 1993-2001 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 */

 #include "ipf-netbas.h"





#if !defined(lint)
static const char sccsid[] = "@(#)ip_fil.c	2.41 6/5/96 (C) 1993-2000 Darren Reed";
static const char rcsid[] = "@(#)$Id$";
#endif

#ifndef	SOLARIS
#define	SOLARIS	(defined(sun) && (defined(__svr4__) || defined(__SVR4)))
#endif



#include "md5.h"


#if !defined(__osf__) && !defined(__netbas__)
extern	struct	protosw	inetsw[];
#endif

#include "ipt.h"
static	struct	ifnet **ifneta = NULL;
static	int	nifs = 0;

static	void	fr_setifpaddr __P((struct ifnet *, char *));
void	init_ifp __P((void));
#if defined(__sgi) && (IRIX < 60500)
static int 	no_output __P((struct ifnet *, struct mbuf *,
			       struct sockaddr *));
static int	write_output __P((struct ifnet *, struct mbuf *,
				  struct sockaddr *));
#else
# if TRU64 >= 1885
static int 	no_output __P((struct ifnet *, struct mbuf *,
			       struct sockaddr *, struct rtentry *, char *));
static int	write_output __P((struct ifnet *, struct mbuf *,
				  struct sockaddr *, struct rtentry *, char *));
# else
static int 	no_output __P((struct ifnet *, struct mbuf *,
			       struct sockaddr *, struct rtentry *));
static int	write_output __P((struct ifnet *, struct mbuf *,
				  struct sockaddr *, struct rtentry *));
# endif
#endif


int ipfattach()
{
	fr_running = 1;
	return 0;
}


int ipfdetach()
{
	fr_running = -1;
	return 0;
}


/*
 * Filter ioctl interface.
 */
int iplioctl(dev, cmd, data, mode)
int dev;
ioctlcmd_t cmd;
caddr_t data;
int mode;
{
	int error = 0, unit = 0, uid;
	SPL_INT(s);

	uid = getuid();
	unit = dev;

	SPL_NET(s);

	error = fr_ioctlswitch(unit, data, cmd, mode, uid, NULL);
	if (error != -1) {
		SPL_X(s);
		return error;
	}

	SPL_X(s);
	return error;
}


void fr_forgetifp(ifp)
void *ifp;
{
	register frentry_t *f;

	WRITE_ENTER(&ipf_mutex);
	for (f = ipacct[0][fr_active]; (f != NULL); f = f->fr_next)
		if (f->fr_ifa == ifp)
			f->fr_ifa = (void *)-1;
	for (f = ipacct[1][fr_active]; (f != NULL); f = f->fr_next)
		if (f->fr_ifa == ifp)
			f->fr_ifa = (void *)-1;
	for (f = ipfilter[0][fr_active]; (f != NULL); f = f->fr_next)
		if (f->fr_ifa == ifp)
			f->fr_ifa = (void *)-1;
	for (f = ipfilter[1][fr_active]; (f != NULL); f = f->fr_next)
		if (f->fr_ifa == ifp)
			f->fr_ifa = (void *)-1;
#ifdef	USE_INET6
	for (f = ipacct6[0][fr_active]; (f != NULL); f = f->fr_next)
		if (f->fr_ifa == ifp)
			f->fr_ifa = (void *)-1;
	for (f = ipacct6[1][fr_active]; (f != NULL); f = f->fr_next)
		if (f->fr_ifa == ifp)
			f->fr_ifa = (void *)-1;
	for (f = ipfilter6[0][fr_active]; (f != NULL); f = f->fr_next)
		if (f->fr_ifa == ifp)
			f->fr_ifa = (void *)-1;
	for (f = ipfilter6[1][fr_active]; (f != NULL); f = f->fr_next)
		if (f->fr_ifa == ifp)
			f->fr_ifa = (void *)-1;
#endif
	RWLOCK_EXIT(&ipf_mutex);
	fr_natsync(ifp);
}


#if defined(__sgi) && (IRIX < 60500)
static int no_output(ifp, m, s)
#else
# if TRU64 >= 1885
static int no_output (ifp, m, s, rt, cp)
char *cp;
# else
static int no_output(ifp, m, s, rt)
# endif
struct rtentry *rt;
#endif
struct ifnet *ifp;
struct mbuf *m;
struct sockaddr *s;
{
	return 0;
}


#if defined(__sgi) && (IRIX < 60500)
static int write_output(ifp, m, s)
#else
# if TRU64 >= 1885
static int write_output (ifp, m, s, rt, cp)
char *cp;
# else
static int write_output(ifp, m, s, rt)
# endif
struct rtentry *rt;
#endif
struct ifnet *ifp;
struct mbuf *m;
struct sockaddr *s;
{
	char fname[32];
	mb_t *mb;
	ip_t *ip;
	int fd;

	mb = (mb_t *)m;
	ip = MTOD(mb, ip_t *);

#if (defined(NetBSD) && (NetBSD <= 1991011) && (NetBSD >= 199606)) || \
    (defined(OpenBSD) && (OpenBSD >= 199603)) || defined(__netbas__) || \
    (defined(__FreeBSD__) && (__FreeBSD_version >= 501113))
	sprintf(fname, "/tmp/%s", ifp->if_xname);
#else
	sprintf(fname, "/tmp/%s%d", ifp->if_name, ifp->if_unit);
#endif
	fd = open(fname, O_WRONLY|O_APPEND);
	if (fd == -1) {
		perror("open");
		return -1;
	}
	write(fd, (char *)ip, ntohs(ip->ip_len));
	close(fd);
	return 0;
}


static void fr_setifpaddr(ifp, addr)
struct ifnet *ifp;
char *addr;
{
#ifdef __sgi
	struct in_ifaddr *ifa;
#else
	struct ifaddr *ifa;
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__)
	if (ifp->if_addrlist.tqh_first != NULL)
#else
# ifdef __sgi
	if (ifp->in_ifaddr != NULL)
# else
	if (ifp->if_addrlist != NULL)
# endif
#endif
		return;

	ifa = (struct ifaddr *)malloc(sizeof(*ifa));
#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__)
	ifp->if_addrlist.tqh_first = ifa;
#else
# ifdef __sgi
	ifp->in_ifaddr = ifa;
# else
	ifp->if_addrlist = ifa;
# endif
#endif

	if (ifa != NULL) {
		struct sockaddr_in *sin;

#ifdef __sgi
		sin = (struct sockaddr_in *)&ifa->ia_addr;
#else
		sin = (struct sockaddr_in *)&ifa->ifa_addr;
#endif
		sin->sin_addr.s_addr = inet_addr(addr);
		if (sin->sin_addr.s_addr == 0)
			abort();
	}
}

struct ifnet *get_unit(name, v)
char *name;
int v;
{
	struct ifnet *ifp, **ifpp, **old_ifneta;
	char *addr;
#if (defined(NetBSD) && (NetBSD <= 1991011) && (NetBSD >= 199606)) || \
    (defined(OpenBSD) && (OpenBSD >= 199603)) || defined(__netbas__) || \
    (defined(__FreeBSD__) && (__FreeBSD_version >= 501113))

	if (name == NULL)
		name = "anon0";

	addr = strchr(name, '=');
	if (addr != NULL)
		*addr++ = '\0';

	for (ifpp = ifneta; ifpp && (ifp = *ifpp); ifpp++) {
		if (!strcmp(name, ifp->if_xname)) {
			if (addr != NULL)
				fr_setifpaddr(ifp, addr);
			return ifp;
		}
	}
#else
	char *s, ifname[LIFNAMSIZ+1];

	if (name == NULL)
		name = "anon0";

	addr = strchr(name, '=');
	if (addr != NULL)
		*addr++ = '\0';

	for (ifpp = ifneta; ifpp && (ifp = *ifpp); ifpp++) {
		COPYIFNAME(v, ifp, ifname);
		if (!strcmp(name, ifname)) {
			if (addr != NULL)
				fr_setifpaddr(ifp, addr);
			return ifp;
		}
	}
#endif

	if (!ifneta) {
		ifneta = (struct ifnet **)malloc(sizeof(ifp) * 2);
		if (!ifneta)
			return NULL;
		ifneta[1] = NULL;
		ifneta[0] = (struct ifnet *)calloc(1, sizeof(*ifp));
		if (!ifneta[0]) {
			free(ifneta);
			return NULL;
		}
		nifs = 1;
	} else {
		old_ifneta = ifneta;
		nifs++;
		ifneta = (struct ifnet **)realloc(ifneta,
						  (nifs + 1) * sizeof(ifp));
		if (!ifneta) {
			free(old_ifneta);
			nifs = 0;
			return NULL;
		}
		ifneta[nifs] = NULL;
		ifneta[nifs - 1] = (struct ifnet *)malloc(sizeof(*ifp));
		if (!ifneta[nifs - 1]) {
			nifs--;
			return NULL;
		}
	}
	ifp = ifneta[nifs - 1];

#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__)
	TAILQ_INIT(&ifp->if_addrlist);
#endif
#if (defined(NetBSD) && (NetBSD <= 1991011) && (NetBSD >= 199606)) || \
    (defined(OpenBSD) && (OpenBSD >= 199603)) || defined(__netbas__) || \
    (defined(__FreeBSD__) && (__FreeBSD_version >= 501113))
	(void) strncpy(ifp->if_xname, name, sizeof(ifp->if_xname));
#else
	for (s = name; *s && !ISDIGIT(*s); s++)
		;
	if (*s && ISDIGIT(*s)) {
		ifp->if_unit = atoi(s);
		ifp->if_name = (char *)malloc(s - name + 1);
		(void) strncpy(ifp->if_name, name, s - name);
		ifp->if_name[s - name] = '\0';
	} else {
		ifp->if_name = strdup(name);
		ifp->if_unit = -1;
	}
#endif
	ifp->if_output = (void *)no_output;

	if (addr != NULL) {
		fr_setifpaddr(ifp, addr);
	}

	return ifp;
}


char *get_ifname(ifp)
struct ifnet *ifp;
{
	static char ifname[LIFNAMSIZ];

#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__netbas__) || \
    (defined(__FreeBSD__) && (__FreeBSD_version >= 501113))
	sprintf(ifname, "%s", ifp->if_xname);
#else
	sprintf(ifname, "%s%d", ifp->if_name, ifp->if_unit);
#endif
	return ifname;
}



void init_ifp()
{
	struct ifnet *ifp, **ifpp;
	char fname[32];
	int fd;

#if (defined(NetBSD) && (NetBSD <= 1991011) && (NetBSD >= 199606)) || \
    (defined(OpenBSD) && (OpenBSD >= 199603)) || defined(__netbas__) || \
    (defined(__FreeBSD__) && (__FreeBSD_version >= 501113))
	for (ifpp = ifneta; ifpp && (ifp = *ifpp); ifpp++) {
		ifp->if_output = (void *)write_output;
		sprintf(fname, "/tmp/%s", ifp->if_xname);
		fd = open(fname, O_WRONLY|O_CREAT|O_EXCL|O_TRUNC, 0600);
		if (fd == -1)
			perror("open");
		else
			close(fd);
	}
#else

	for (ifpp = ifneta; ifpp && (ifp = *ifpp); ifpp++) {
		ifp->if_output = write_output;
		sprintf(fname, "/tmp/%s%d", ifp->if_name, ifp->if_unit);
		fd = open(fname, O_WRONLY|O_CREAT|O_EXCL|O_TRUNC, 0600);
		if (fd == -1)
			perror("open");
		else
			close(fd);
	}
#endif
}


int fr_fastroute(m, mpp, fin, fdp)
mb_t *m, **mpp;
fr_info_t *fin;
frdest_t *fdp;
{
	struct ifnet *ifp = fdp->fd_ifp;
	ip_t *ip = fin->fin_ip;
	int error = 0;
	frentry_t *fr;
	void *sifp;

	if (!ifp)
		return 0;	/* no routing table out here */

	fr = fin->fin_fr;
	ip->ip_sum = 0;

	if (fin->fin_out == 0) {
		sifp = fin->fin_ifp;
		fin->fin_ifp = ifp;
		fin->fin_out = 1;
		(void) fr_acctpkt(fin, NULL);
		fin->fin_fr = NULL;
		if (!fr || !(fr->fr_flags & FR_RETMASK)) {
			u_32_t pass;

			(void) fr_checkstate(fin, &pass);
		}

		switch (fr_checknatout(fin, NULL))
		{
		case 0 :
			break;
		case 1 :
			ip->ip_sum = 0;
			break;
		case -1 :
			error = -1;
			goto done;
			break;
		}

		fin->fin_ifp = sifp;
		fin->fin_out = 0;
	}

#if defined(__sgi) && (IRIX < 60500)
	(*ifp->if_output)(ifp, (void *)ip, NULL);
# if TRU64 >= 1885
	(*ifp->if_output)(ifp, (void *)m, NULL, 0, 0);
# else
	(*ifp->if_output)(ifp, (void *)m, NULL, 0);
# endif
#endif
done:
	return error;
}


int fr_send_reset(fin)
fr_info_t *fin;
{
	verbose("- TCP RST sent\n");
	return 0;
}


int fr_send_icmp_err(type, fin, dst)
int type;
fr_info_t *fin;
int dst;
{
	verbose("- ICMP unreachable sent\n");
	return 0;
}


void frsync(ifp)
void *ifp;
{
	return;
}


void m_freem(m)
mb_t *m;
{
	return;
}


void m_copydata(m, off, len, cp)
mb_t *m;
int off, len;
caddr_t cp;
{
	bcopy((char *)m + off, cp, len);
}


int ipfuiomove(buf, len, rwflag, uio)
caddr_t buf;
int len, rwflag;
struct uio *uio;
{
	int left, ioc, num, offset;
	struct iovec *io;
	char *start;

	if (rwflag == UIO_READ) {
		left = len;
		ioc = 0;

		offset = uio->uio_offset;

		while ((left > 0) && (ioc < uio->uio_iovcnt)) {
			io = uio->uio_iov + ioc;
			num = io->iov_len;
			if (num > left)
				num = left;
			start = (char *)io->iov_base + offset;
			if (start > (char *)io->iov_base + io->iov_len) {
				offset -= io->iov_len;
				ioc++;
				continue;
			}
			bcopy(buf, start, num);
			uio->uio_resid -= num;
			uio->uio_offset += num;
			left -= num;
			if (left > 0)
				ioc++;
		}
		if (left > 0)
			return EFAULT;
	}
	return 0;
}


u_32_t fr_newisn(fin)
fr_info_t *fin;
{
	static int iss_seq_off = 0;
	u_char hash[16];
	u_32_t newiss;
	MD5_CTX ctx;

	/*
	 * Compute the base value of the ISS.  It is a hash
	 * of (saddr, sport, daddr, dport, secret).
	 */
	MD5Init(&ctx);

	MD5Update(&ctx, (u_char *) &fin->fin_fi.fi_src,
		  sizeof(fin->fin_fi.fi_src));
	MD5Update(&ctx, (u_char *) &fin->fin_fi.fi_dst,
		  sizeof(fin->fin_fi.fi_dst));
	MD5Update(&ctx, (u_char *) &fin->fin_dat, sizeof(fin->fin_dat));

	/* MD5Update(&ctx, ipf_iss_secret, sizeof(ipf_iss_secret)); */

	MD5Final(hash, &ctx);

	memcpy(&newiss, hash, sizeof(newiss));

	/*
	 * Now increment our "timer", and add it in to
	 * the computed value.
	 *
	 * XXX Use `addin'?
	 * XXX TCP_ISSINCR too large to use?
	 */
	iss_seq_off += 0x00010000;
	newiss += iss_seq_off;
	return newiss;
}


/* ------------------------------------------------------------------------ */
/* Function:    fr_nextipid                                                 */
/* Returns:     int - 0 == success, -1 == error (packet should be droppped) */
/* Parameters:  fin(I) - pointer to packet information                      */
/*                                                                          */
/* Returns the next IPv4 ID to use for this packet.                         */
/* ------------------------------------------------------------------------ */
INLINE u_short fr_nextipid(fin)
fr_info_t *fin;
{
	static u_short ipid = 0;
	u_short id;

	MUTEX_ENTER(&ipf_rw);
	id = ipid++;
	MUTEX_EXIT(&ipf_rw);

	return id;
}


INLINE void fr_checkv4sum(fin)
fr_info_t *fin;
{
	if (fr_checkl4sum(fin) == -1)
		fin->fin_flx |= FI_BAD;
}


#ifdef	USE_INET6
INLINE void fr_checkv6sum(fin)
fr_info_t *fin;
{
	if (fr_checkl4sum(fin) == -1)
		fin->fin_flx |= FI_BAD;
}
#endif


/*
 * See above for description, except that all addressing is in user space.
 */
int copyoutptr(src, dst, size)
void *src, *dst;
size_t size;
{
	caddr_t ca;

	bcopy(dst, (char *)&ca, sizeof(ca));
	bcopy(src, ca, size);
	return 0;
}


/*
 * See above for description, except that all addressing is in user space.
 */
int copyinptr(src, dst, size)
void *src, *dst;
size_t size;
{
	caddr_t ca;

	bcopy(src, (char *)&ca, sizeof(ca));
	bcopy(ca, dst, size);
	return 0;
}


/*
 * return the first IP Address associated with an interface
 */
int fr_ifpaddr(v, atype, ifptr, inp, inpmask)
int v, atype;
void *ifptr;
struct in_addr *inp, *inpmask;
{
	struct ifnet *ifp = ifptr;
#ifdef __sgi
	struct in_ifaddr *ifa;
#else
	struct ifaddr *ifa;
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD__)
	ifa = ifp->if_addrlist.tqh_first;
#else
# ifdef __sgi
	ifa = (struct in_ifaddr *)ifp->in_ifaddr;
# else
	ifa = ifp->if_addrlist;
# endif
#endif
	if (ifa != NULL) {
		struct sockaddr_in *sin, mask;

		mask.sin_addr.s_addr = 0xffffffff;

#ifdef __sgi
		sin = (struct sockaddr_in *)&ifa->ia_addr;
#else
		sin = (struct sockaddr_in *)&ifa->ifa_addr;
#endif

		return fr_ifpfillv4addr(atype, sin, &mask, inp, inpmask);
	}
	return 0;
}


int ipfsync()
{
	return 0;
}


/*    
 * This function is not meant to be random, rather just produce a
 * sequence of numbers that isn't linear to show "randomness".
 */
u_32_t
ipf_random() 
{
	static int last = 0xa5a5a5a5;
	static int calls = 0;
	int number;

	calls++;

	/*
	 * These are deliberately chosen to ensure that there is some
	 * attempt to test whether the output covers the range in test n18.
	 */
	switch (calls)
	{
	case 1 :
		number = 0;
		break;
	case 2 :
		number = 4;
		break;
	case 3 :
		number = 3999;
		break;
	case 4 :
		number = 4000;
		break;
	case 5 :
		number = 48999;
		break;
	case 6 :
		number = 49000;
		break;
	default :
		/*
		 * So why not use srand/rand/srandom/random?  Because the
		 * actual values returned vary from platform to platform
		 * and what is needed is seomthing that is the same everywhere
		 * so that regression tests can work.  Well, they could be
		 * built on each platform to suit but that's a whole lot of
		 * work for little gain given that we don't actually need
		 * random numbers here, just a spread to test the NAT code
		 * with.
		 */
		number = last;
		last *= calls;
		last++;
		number ^= last;
		break;
	}
	return number;
}
