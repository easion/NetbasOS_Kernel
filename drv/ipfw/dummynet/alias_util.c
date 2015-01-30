
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/sys/netinet/libalias/alias_util.c,v 1.18 2005/06/27 07:36:02 glebius Exp $");


/*
    Alias_util.c contains general utilities used by other functions
    in the packet aliasing module.  At the moment, there are functions
    for computing IP header and TCP packet checksums.

    The checksum routines are based upon example code in a Unix networking
    text written by Stevens (sorry, I can't remember the title -- but
    at least this is a good author).

    Initial Version:  August, 1996  (cjm)

    Version 1.7:  January 9, 1997
         Added differential checksum update function.
*/

#ifdef _KERNEL
#include <sys/param.h>
#else
#include <sys/types.h>
#include <stdio.h>
#endif

//#include <netinet/in_systm.h>
//#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#ifdef _KERNEL
//#include <netinet/libalias/alias.h>
//#include <netinet/libalias/alias_local.h>
#else
#include "alias_local.h"
#endif
#include "alias.h"

/*
 * Note: the checksum routines assume that the actual checksum word has
 * been zeroed out.  If the checksum word is filled with the proper value,
 * then these routines will give a result of zero (useful for testing
 * purposes);
 */
u_short
LibAliasInternetChecksum(struct libalias *la __unused, u_short * ptr,
        int nbytes)
{
        int sum, oddbyte;

        sum = 0;
        while (nbytes > 1) {
                sum += *ptr++;
                nbytes -= 2;
        }
        if (nbytes == 1) {
                oddbyte = 0;
                ((u_char *) & oddbyte)[0] = *(u_char *) ptr;
                ((u_char *) & oddbyte)[1] = 0;
                sum += oddbyte;
        }
        sum = (sum >> 16) + (sum & 0xffff);
        sum += (sum >> 16);
        return (~sum);
}

#ifndef _KERNEL
u_short
IpChecksum(struct ip *pip)
{
        return (LibAliasInternetChecksum(NULL, (u_short *) pip,
            (pip->ip_hl << 2)));

}

u_short
TcpChecksum(struct ip *pip)
{
        u_short *ptr;
        struct tcphdr *tc;
        int nhdr, ntcp, nbytes;
        int sum, oddbyte;

        nhdr = pip->ip_hl << 2;
        ntcp = ntohs(pip->ip_len) - nhdr;

        tc = (struct tcphdr *)ip_next(pip);
        ptr = (u_short *) tc;

/* Add up TCP header and data */
        nbytes = ntcp;
        sum = 0;
        while (nbytes > 1) {
                sum += *ptr++;
                nbytes -= 2;
        }
        if (nbytes == 1) {
                oddbyte = 0;
                ((u_char *) & oddbyte)[0] = *(u_char *) ptr;
                ((u_char *) & oddbyte)[1] = 0;
                sum += oddbyte;
        }
/* "Pseudo-header" data */
        ptr = (u_short *) & (pip->ip_dst);
        sum += *ptr++;
        sum += *ptr;
        ptr = (u_short *) & (pip->ip_src);
        sum += *ptr++;
        sum += *ptr;
        sum += htons((u_short) ntcp);
        sum += htons((u_short) pip->ip_p);

/* Roll over carry bits */
        sum = (sum >> 16) + (sum & 0xffff);
        sum += (sum >> 16);

/* Return checksum */
        return ((u_short) ~ sum);
}
#endif  /* not _KERNEL */

void
DifferentialChecksum(u_short * cksum, void *newp, void *oldp, int n)
{
        int i;
        int accumulate;
        u_short *new = newp;
        u_short *old = oldp;

        accumulate = *cksum;
        for (i = 0; i < n; i++) {
                accumulate -= *new++;
                accumulate += *old++;
        }

        if (accumulate < 0) {
                accumulate = -accumulate;
                accumulate = (accumulate >> 16) + (accumulate & 0xffff);
                accumulate += accumulate >> 16;
                *cksum = (u_short) ~ accumulate;
        } else {
                accumulate = (accumulate >> 16) + (accumulate & 0xffff);
                accumulate += accumulate >> 16;
                *cksum = (u_short) accumulate;
        }
}

void
in_delayed_cksum(struct mbuf *m)
{
        struct ip *ip;
        u_short csum, offset;

        ip = mtod(m, struct ip *);
        offset = ip->ip_hl << 2 ;
        csum = in_cksum_skip(m, ip->ip_len, offset);
        if (m->m_pkthdr.csum_flags & CSUM_UDP && csum == 0)
                csum = 0xffff;
        offset += m->m_pkthdr.csum_data;        /* checksum offset */

        if (offset + sizeof(u_short) > m->m_len) {
                printf("delayed m_pullup, m->len: %d  off: %d  p: %d\n",
                    m->m_len, offset, ip->ip_p);
                /*
                 * XXX
                 * this shouldn't happen, but if it does, the
                 * correct behavior may be to insert the checksum
                 * in the appropriate next mbuf in the chain.
                 */
                return;
        }
        *(u_short *)(m->m_data + offset) = csum;
}

/*
 * m_megapullup() function is a big hack.
 *
 * It allocates an mbuf with cluster and copies the whole
 * chain into cluster, so that it is all contigous and the
 * whole packet can be accessed via char pointer.
 *
 * This is required, because libalias doesn't have idea
 * about mbufs.
 */
struct mbuf *
m_megapullup(struct mbuf *m, int len)
{
        struct mbuf *mcl;
        caddr_t cp;
		printf("%s called\n", __FUNCTION__);
#if 0
        if (len > MCLBYTES)
                goto bad;

        if ((mcl = m_getcl(M_DONTWAIT, MT_DATA, M_PKTHDR)) == NULL)
                goto bad;

        cp = mtod(mcl, caddr_t);
        m_copydata(m, 0, len, cp);
        m_move_pkthdr(mcl, m);
        mcl->m_len = mcl->m_pkthdr.len;
        m_freem(m);

        return (mcl);
#endif
bad:
        m_freem(m);

        return (NULL);
}


#define IDX(c)  ((u_char)(c) / LONG_BIT)
#define BIT(c)  ((u_long)1 << ((u_char)(c) % LONG_BIT))

#define UCHAR_MAX 0xffU
#define LONG_BIT 32
size_t
strspn(const char *s, const char *charset)
{
        /*
         * NB: idx and bit are temporaries whose use causes gcc 3.4.2 to
         * generate better code.  Without them, gcc gets a little confused.
         */
        const char *s1;
        u_long bit;
        u_long tbl[(UCHAR_MAX + 1) / LONG_BIT];
        int idx;

        if(*s == '\0')
                return (0);

#if LONG_BIT == 64      /* always better to unroll on 64-bit architectures */
        tbl[3] = tbl[2] = tbl[1] = tbl[0] = 0;
#else
        for (idx = 0; idx < sizeof(tbl) / sizeof(tbl[0]); idx++)
                tbl[idx] = 0;
#endif
        for (; *charset != '\0'; charset++) {
                idx = IDX(*charset);
                bit = BIT(*charset);
                tbl[idx] |= bit;
        }

        for(s1 = s; ; s1++) {
                idx = IDX(*s1);
                bit = BIT(*s1);
                if ((tbl[idx] & bit) == 0)
                        break;
        }
        return (s1 - s);
}

/*
 * Convert a string to an unsigned long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
unsigned long
strtoul(nptr, endptr, base)
        const char *nptr;
        char **endptr;
        int base;
{
        const char *s = nptr;
        unsigned long acc;
        unsigned char c;
        unsigned long cutoff;
        int neg = 0, any, cutlim;

        /*
         * See strtol for comments as to the logic used.
         */
        do {
                c = *s++;
        } while (isspace(c));
        if (c == '-') {
                neg = 1;
                c = *s++;
        } else if (c == '+')
                c = *s++;
        if ((base == 0 || base == 16) &&
            c == '0' && (*s == 'x' || *s == 'X')) {
                c = s[1];
                s += 2;
                base = 16;
        }
        if (base == 0)
                base = c == '0' ? 8 : 10;
        cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
        cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;
        for (acc = 0, any = 0;; c = *s++) {
                if (!isascii(c))
                        break;
                if (isdigit(c))
                        c -= '0';
                else if (isalpha(c))
                        c -= isupper(c) ? 'A' - 10 : 'a' - 10;
                else
                        break;
                if (c >= base)
                        break;
                if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
                        any = -1;
                else {
                        any = 1;
                        acc *= base;
                        acc += c;
                }
        }
        if (any < 0) {
                acc = ULONG_MAX;
        } else if (neg)
                acc = -acc;
        if (endptr != 0)
                *((const char **)endptr) = any ? s - 1 : nptr;
        return (acc);
}

/*
 * Get next token from string *stringp, where tokens are possibly-empty
 * strings separated by characters from delim.
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, strsep returns NULL.
 */
char *
strsep(stringp, delim)
        char **stringp;
        const char *delim;
{
        char *s;
        const char *spanp;
        int c, sc;
        char *tok;

        if ((s = *stringp) == NULL)
                return (NULL);
        for (tok = s;;) {
                c = *s++;
                spanp = delim;
                do {
                        if ((sc = *spanp++) == c) {
                                if (c == 0)
                                        s = NULL;
                                else
                                        s[-1] = 0;
                                *stringp = s;
                                return (tok);
                        }
                } while (sc != 0);
        }
        /* NOTREACHED */
}


__inline u_short
in_pseudo(u_int sum, u_int b, u_int c)
{
        /* __volatile is necessary because the condition codes are used. */
        __asm __volatile ("addl %1, %0" : "+r" (sum) : "g" (b));
        __asm __volatile ("adcl %1, %0" : "+r" (sum) : "g" (c));
        __asm __volatile ("adcl $0, %0" : "+r" (sum));

        sum = (sum & 0xffff) + (sum >> 16);
        if (sum > 0xffff)
                sum -= 0xffff;
        return (sum);
}

struct mbuf *
m_free(struct mbuf *m)
{
        struct mbuf *n = m->m_next;
		printf("%s called\n", __FUNCTION__);
#if 0
        if (m->m_flags & M_EXT)
                mb_free_ext(m);
        else
                uma_zfree(zone_mbuf, m);
#endif
        return n;
}
