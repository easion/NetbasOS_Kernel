/*
 * Copyright (C) 2009 Luigi Rizzo, Marta Carbone, Universita` di Pisa
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * $Id: bsd_compat.c 2849 2009-06-22 11:11:01Z luigi $
 *
 * kernel variables and functions that are not available in linux.
 */

#include <sys/cdefs.h>
//#include <asm/div64.h>	/* do_div on 2.4 */
//#include <linux/random.h>	/* get_random_bytes on 2.4 */
#include "missing.h"

/*
 * gettimeofday would be in sys/time.h but it is not
 * visible if _KERNEL is defined
 */
int gettimeofday(struct timeval *, struct timezone *);

int ticks;		/* kernel ticks counter */
//int hz = 1000;		/* default clock time */
//long tick = 1000;	/* XXX is this 100000/hz ? */
int bootverbose = 0;
time_t time_uptime = 0;
struct timeval boottime;
int hz = 100;		/* default clock time */
long tick = 100;	/* XXX is this 100000/hz ? */
int     ip_defttl;
int fw_one_pass = 1;
u_long  in_ifaddrhmask;                         /* mask for hash table */
struct  in_ifaddrhashhead *in_ifaddrhashtbl;    /* inet addr hash table  */

u_int rt_numfibs = RT_NUMFIBS;

/*
 * pfil hook support.
 * We make pfil_head_get return a non-null pointer, which is then ignored
 * in our 'add-hook' routines.
 */
struct pfil_head;
typedef int (pfil_hook_t)
	(void *, struct mbuf **, struct ifnet *, int, struct inpcb *);

struct pfil_head *
pfil_head_get(int proto, u_long flags)
{
	static int dummy;
	return (struct pfil_head *)&dummy;
}
 
int
pfil_add_hook(pfil_hook_t *func, void *arg, int dir, struct pfil_head *h)
{
	return 0;
}

int
pfil_remove_hook(pfil_hook_t *func, void *arg, int dir, struct pfil_head *h)
{
	return 0;
}

/* define empty body for kernel function */
int
priv_check(struct thread *td, int priv)
{
	return 0;
}

int
securelevel_ge(struct ucred *cr, int level)
{
	return 0;
}

int
sysctl_handle_int(SYSCTL_HANDLER_ARGS)
{
	return 0;
}

int
sysctl_handle_long(SYSCTL_HANDLER_ARGS)
{
	return 0;
}

void
ether_demux(struct ifnet *ifp, struct mbuf *m)
{
	return;
}

int
ether_output_frame(struct ifnet *ifp, struct mbuf *m)
{
	return 0;
}

void
in_rtalloc_ign(struct route *ro, u_long ignflags, u_int fibnum)
{
	return;
}

void
icmp_error(struct mbuf *n, int type, int code, uint32_t dest, int mtu)
{
	return;
}

u_short
in_cksum_skip(struct mbuf *m, int len, int skip)
{
	return 0;
}

u_short
in_cksum_hdr(struct ip *ip)
{
	return 0;
}

struct mbuf *
ip_reass(struct mbuf *clone)
{
	return clone;
}
#ifdef INP_LOCK_ASSERT
#undef INP_LOCK_ASSERT
#define INP_LOCK_ASSERT(a)
#endif

int
jailed(struct ucred *cred)
{
	return 0;
}

/*
* Return 1 if an internet address is for a ``local'' host
* (one to which we have a connection).  If subnetsarelocal
* is true, this includes other subnets of the local net.
* Otherwise, it includes only the directly-connected (sub)nets.
*/
int
in_localaddr(struct in_addr in)
{
	return 1;
}

int
sooptcopyout(struct sockopt *sopt, const void *buf, size_t len)
{
	size_t valsize = sopt->sopt_valsize;

	if (len < valsize)
		sopt->sopt_valsize = valsize = len;
	bcopy(buf, sopt->sopt_val, valsize);
	return 0;
}

/*
 * copy data from userland to kernel
 */
int
sooptcopyin(struct sockopt *sopt, void *buf, size_t len, size_t minlen)
{
	size_t valsize = sopt->sopt_valsize;

	if (valsize < minlen)
		return EINVAL;
	if (valsize > len)
		sopt->sopt_valsize = valsize = len;
	bcopy(sopt->sopt_val, buf, valsize);
	return 0;
}

void
getmicrouptime(struct timeval *tv)
{
#ifdef _WIN32
#else
	//do_gettimeofday(tv);
	tv->tv_sec = get_unix_time();
	tv->tv_usec = (startup_ticks() %HZ)*10000;
#endif
}


//#include <arpa/inet.h>

char *
inet_ntoa_r(struct in_addr ina, char *buf)
{
#ifdef _WIN32
#else
	unsigned char *ucp = (unsigned char *)&ina;

	sprintf(buf, "%d.%d.%d.%d",
	ucp[0] & 0xff,
	ucp[1] & 0xff,
	ucp[2] & 0xff,
	ucp[3] & 0xff);
#endif
	return buf;
}

char *
inet_ntoa(struct in_addr ina)
{
	static char buf[16];
	return inet_ntoa_r(ina, buf);
}

int
random(void)
{
#ifdef __NETBAS__
	return 0x123456;
#else
	int r;
	get_random_bytes(&r, sizeof(r));
	return r & 0x7fffffff; 
#endif
}


/*
 * do_div really does a u64 / u32 bit division.
 * we save the sign and convert to uint befor calling.
 * We are safe just because we always call it with small operands.
 */
int64_t
div64(int64_t a, int64_t b)
{
#ifdef __NETBAS__
        int a1 = a, b1 = b;
	return a1/b1;
#else
	uint64_t ua, ub;
	int sign = ((a>0)?1:-1) * ((b>0)?1:-1);

	ua = ((a>0)?a:-a);
	ub = ((b>0)?b:-b);
        do_div(ua, ub);
	return sign*ua;
#endif
}

/*
 * compact version of fnmatch.
 */
int
fnmatch(const char *pattern, const char *string, int flags)
{
	char s;

	if (!string || !pattern)
		return 1;	/* no match */
	while ( (s = *string++) ) {
		char p = *pattern++;
		if (p == '\0')		/* pattern is over, no match */
			return 1;
		if (p == '*')		/* wildcard, match */
			return 0;
		if (p == '.' || p == s)	/* char match, continue */
			continue;
		return 1;		/* no match */
	}
	/* end of string, make sure the pattern is over too */
	if (*pattern == '\0' || *pattern == '*')
		return 0;
	return 1;	/* no match */
}

#ifdef _WIN32
/*
 * as good as anywhere, place here the missing calls
 */

void *
my_alloc(int size)
{
	void *_ret = ExAllocatePoolWithTag(0, size, 'wfpi');
	if (_ret)
		memset(_ret, 0, size);
	return _ret;
}

void
panic(const char *fmt, ...)
{
	printf("%s", fmt);
	for (;;);
}

#include <stdarg.h>

extern int _vsnprintf(char *buf, int buf_size, char * fmt, va_list ap);

/*
 * Windows' _snprintf doesn't terminate buffer with zero if size > buf_size
 */
int
snprintf(char *buf, int buf_size, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (_vsnprintf(buf, buf_size, fmt, ap) < 0)
        buf[buf_size - 1] = '\0';
    va_end(ap);

    return 0;
}
#endif
