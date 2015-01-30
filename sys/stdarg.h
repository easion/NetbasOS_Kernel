/*
 *  Yoctix stdarg.h
 *  Copyright (C) 1999 by Anders Gavare.  All rights reserved.
 *
 *  History:
 *	18 Oct 1999	Original from NetBSD
 */

/*	$NetBSD: stdarg.h,v 1.12 1995/12/25 23:15:31 mycroft Exp $	*/

/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)stdarg.h	8.1 (Berkeley) 6/10/93
 */


#ifndef __STDARG_H_
#define	__STDARG_H_

#ifdef __ARM__

typedef void *__sys_va_list;
typedef __sys_va_list va_list;
#define __va_rounded_size(type)	\
	(((sizeof (type) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
#define va_start(ap, lastarg)	\
	(ap = ((char *) &(lastarg) + __va_rounded_size(lastarg))) 
#define va_end(ap)	((void)0)
/*	little endian */
#define va_arg(ap, type)	\
	(ap = (__sys_va_list) ((char *) (ap) + __va_rounded_size (type)),  \
	*((type *) (void *) ((char *) (ap) - __va_rounded_size (type))))

#else 

typedef char *	va_list;

#define	__va_size(type) \
	(((sizeof(type) + sizeof(long) - 1) / sizeof(long)) * sizeof(long))

#define	va_start(ap, last) \
	((ap) = (va_list)&(last) + __va_size(last))

#define	va_arg(ap, type) \
	(*(type *)((ap) += __va_size(type), (ap) - __va_size(type)))

#define	va_end(ap)	((void)0)

int vsprintf(char *buf, int max, const char *fmt, va_list args);

#endif

#endif	

