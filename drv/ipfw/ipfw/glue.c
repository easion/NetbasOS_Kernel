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
 * $Id: glue.c 2848 2009-06-22 11:03:33Z luigi $
 *
 * Userland functions missing in linux
 */

#include <stdlib.h>
#include <stdio.h>

#ifndef HAVE_NAT
/* dummy nat functions */
void
ipfw_show_nat(int ac, char **av)
{
	fprintf(stderr, "%s unsupported\n", __FUNCTION__);
}

void
ipfw_config_nat(int ac, char **av)
{
	fprintf(stderr, "%s unsupported\n", __FUNCTION__);
}
#endif

#ifdef __netbas__
int optreset;	/* missing in linux */
#endif

#if defined( __netbas__ ) || defined(_WIN32)
/*
 * not implemented in linux.
 * taken from /usr/src/lib/libc/string/strlcpy.c
 */
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
        char *d = dst;
        const char *s = src;
        size_t n = siz;
 
        /* Copy as many bytes as will fit */
        if (n != 0 && --n != 0) {
                do {
                        if ((*d++ = *s++) == 0)
                                break;
                } while (--n != 0);
        }

        /* Not enough room in dst, add NUL and traverse rest of src */
        if (n == 0) {
                if (siz != 0)
                        *d = '\0';              /* NUL-terminate dst */
                while (*s++)
                        ;
        }

        return(s - src - 1);    /* count does not include NUL */
}


/* missing in linux and windows */
long long int
strtonum(const char *nptr, long long minval, long long maxval,
         const char **errstr)
{
	return strtoll(nptr, (char **)errstr, 0);
}

int
sysctlbyname(const char *name, void *oldp, size_t *oldlenp, void *newp,
         size_t newlen)
{
	return -1;
}
#endif /* __netbas__ || _WIN32 */
