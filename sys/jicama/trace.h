
// ---------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2005  DengPingPing      All rights reserved.  
//-----------------------------------------------------------------------------------
#ifndef __trace_h__
#define __trace_h__


extern int kprintf (const char *fmt, ...);
extern int panic(const char * fmt, ...);
int syslog(int pri, const char *fmt, ...);

void halt(void);


dev_t dev_nr(int ma, u8_t mi);

void *kmalloc(unsigned int clicks, long flag);
int kfree(void *addr);
void *kcalloc(size_t elsize);
void *kzalloc(unsigned int size);
u32_t mm_malloc(u32_t clicks);
int ksize( void *address);

#define errrpt(n) do_errrpt(n,  __LINE__, __FUNCTION__)

int gfx_printf (char *fmt, ...);

inline static void do_errrpt(const char *s,  int line, char *func)
{
	kprintf("%s() Line:%d: %s ERROR\n", func, line, s);
}


#ifdef __ENABLE_TRACE__
	#define trace(format, argc...) 		kprintf(format, ##argc)	

	#define TRACE(msg) 			kprintf(msg)
	#define TRACE1(msg, x)			kprintf(msg,x)
	#define TRACE2(msg, x1, x2)		kprintf(msg, x1, x2)
	#define TRACE3(msg, x1, x2, x3) 	kprintf(msg, x1, x2, x3)
	#define TRACE4(msg, x1, x2, x3, x4) 	kprintf(msg, x1, x2, x3, x4)
	#define TRACE5(msg, x1, x2, x3, x4, x5)	kprintf(msg, x1, x2, x3, x4, x5)
	#define TRACE6(msg, x1, x2, x3, x4, x5,x6)	kprintf(msg, x1, x2, x3, x4, x5,x6)
	#define TRACE7(msg, x1, x2, x3, x4, x5,x6,x7)	kprintf(msg, x1, x2, x3, x4, x5,x6,x7)

#else
	#define trace(format, argc...) ;//kprintf(KINFOMSG format, ##argc)
	#define TRACE(msg) 			;
	#define TRACE1(msg, x)			;
	#define TRACE2(msg, x1, x2)		;
	#define TRACE3(msg, x1, x2, x3) 	;
	#define TRACE4(msg, x1, x2, x3, x4) 	;
	#define TRACE5(msg, x1, x2, x3, x4, x5)	;
	#define TRACE6(msg, x1, x2, x3, x4, x5,x6)	;
	#define TRACE7(msg, x1, x2, x3, x4, x5,x6,x7)	;
#endif



#endif

