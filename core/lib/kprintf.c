
#include <type.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h> 
#include <jicama/system.h> 
#include <jicama/log.h> 
#include <jicama/proc_entry.h>
void kernel_debugger(void);

typedef int (*fnptr_t)(unsigned c, void **helper);
int do_printf(const char *fmt, va_list args, fnptr_t fn,int, void *ptr);

/*****************************************************************************
Stripped-down printf()
Chris Giese <geezer@execpc.com>	http://www.execpc.com/~geezer
Release date: Dec 12, 2003
This code is public domain (no copyright).
You can do whatever you want with it.

Revised May 12, 2000
- math in DO_NUM is now unsigned, as it should be
- %0 flag (pad left with zeroes) now works
- actually did some TESTING, maybe fixed some other bugs

%[flag][width][.prec][mod][conv]
flag:	-	left justify, pad right w/ blanks	DONE
	0	pad left w/ 0 for numerics		DONE
	+	always print sign, + or -		no
	' '	(blank)					no
	#	(???)					no

width:		(field width)				DONE

prec:		(precision)				no

conv:	d,i	decimal int				DONE
	u	decimal unsigned			DONE
	o	octal					DONE
	x,X	hex					DONE
	f,e,g,E,G float					no
	c	char					DONE
	s	string					DONE
	p	ptr					DONE

mod:	N	near ptr				DONE
	F	far ptr					no
	h	short (16-bit) int			DONE
	l	long (32-bit) int			DONE
	L	long long (64-bit) int			no
*****************************************************************************/
__asmlink int puts(const unsigned char *str);
__asmlink int log_vprintf(int pri, const char *fmt,int n, va_list args);

__local int vsprintf_help(unsigned c, void **ptr)
{
	char *dst;

	dst = (char *)*ptr;
	*dst++ = (char)c;
	*ptr = dst;
	return 0 ;
}
/*********************************************************/
int vsprintf(char *buf, int max, const char *fmt, va_list args)
{
	int rv;

	rv = do_printf(fmt, args, vsprintf_help,max, (void *)buf);
	buf[rv] = '\0';
	return rv;
}

int sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int rv;

	va_start(args, fmt);
	rv = vsprintf(buf,4096, fmt, args);
	va_end(args);
	return rv;
}

int pprintf(struct proc_entry *pf, const char *fmt, ...)
{
	va_list args;
	int rv;
	char *buf;
	int left = 0;

	if (!pf->read_buffer)
	{
		return 0;
	}

	left = pf->read_size - pf->read_pos;

	buf = pf->read_buffer+pf->read_pos;
	va_start(args, fmt);
	rv = vsprintf(buf,left, fmt, args);
	va_end(args);

	pf->read_pos += rv;
	return rv;
}


size_t snprintf (char *buf, size_t len, const char *fmt, ...)
{
	va_list args;
	int rv;


	va_start(args, fmt);
	rv = vsprintf(buf, len, fmt, args);
	va_end(args);
	
	return rv;
}

int vprintf_help(unsigned c, void **ptr)
{
	tty_putchar(c);
	return 0 ;
}



int vprintf(int pri, const char *fmt,int max, va_list args)
{
	return do_printf(fmt, args, vprintf_help,max, NULL);
}

 int kprintf(const char *fmt, ...)
{
	va_list args;
	int rv;

	va_start(args, fmt);
	log_vprintf(LOG_NOTICE,fmt,4096, args);
	rv = vprintf(LOG_NOTICE,fmt,4096, args);
	va_end(args);
	return rv;
}


int panic(const char *fmt, ...)
{
	va_list args;
	int rv;

	puts("\x1B[36mKernel Panic:");

	va_start(args, fmt);
	rv = vprintf(LOG_EMERG, fmt,4096, args);
	va_end(args);

	{

	void *array[15];
	size_t size;
	char **strings;
	int i;

	size = backtrace (array, 10);
	strings = backtrace_symbols (array, size);

	//backtrace_symbols_fd(array, size,1);
	}


	disable();

	kernel_debugger();

	puts("\x1B[34;0m");

#ifdef PANIC_SEC
	puts("Reboot Kernel After % sec ...", PANIC_SEC);
	enable();
	mdelay(PANIC_SEC*1000);
	do_reboot();
#else	
	halt();
#endif	
	return rv;
}

#if 0/* testing */

int print_demo(void)
{
	char buf[64];

	sprintf(buf, "%u score and %i years ago...\n", 4, -7);
	puts(buf); /* puts() adds newline */

	sprintf(buf, "-1L == 0x%lX == octal %lo\n", -1L, -1L);
	puts(buf); /* puts() adds newline */

	kprintf("<%-08s> and <%08s> justified strings\n", "left", "right");
	return 0;
}
#endif
