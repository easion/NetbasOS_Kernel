#include <jicama/system.h>

void outb(unsigned port, unsigned char val){
	__asm("outb %b0,%w1"
		:
		: "a"(val), "d"(port));
}

unsigned char inb(unsigned short port)
{
    unsigned char _tmp__; 

	__asm volatile("inb %1, %0" : "=a" (_tmp__) : "d" (port)); 
	return _tmp__; 
}

void p (unsigned char *lock)
{
	int res;

	res = 1;
	asm ("lock ; xchgb %1, %0" : "=m" (*lock), "=r" (res) : "1" (res) : "memory");
	while (res)
		asm ("lock ; xchgb %1, %0" : "=m" (*lock), "=r" (res) : "1" (res) :
			"memory");
}

void v (unsigned char *lock)
{
	asm ("lock ; btrl $0, %0" : "=m" (*lock));
}






 unsigned long inl(unsigned short port)
{
 unsigned long ans = 0;
 __asm ( "mov %1, %%dx\n"
			"inl %%dx, %%eax\n"
			"mov %%eax, %0\n"
                	       : "=g" (ans)  
			       : "g" (port)
			       : "dx","ax"
		      );
 return ans;
}

 void outl(unsigned short port, unsigned long lo)
{
 __asm ("mov %0, %%dx\n"
		"mov %1, %%eax\n"
		"outl %%eax, %%dx\n"
		:  
		: "g" (port), "g" (lo)
		: "edx","eax"
		);
}



 void outw(unsigned short port, unsigned short word)
{
 __asm ( "mov %0, %%dx\n"
			"mov %1, %%ax\n"
			"outw %%ax, %%dx\n"
                	       :  
			       : "g" (port), "g" (word)
			       : "dx","ax"
		       );
}

unsigned short inw(unsigned short port)
{
	unsigned short ans = 0;
	__asm ( "mov %1, %%dx\n"
			"inw %%dx, %%ax\n"
			"mov %%ax, %0\n"
			: "=g" (ans)  
			: "g" (port)
			: "dx","ax"
			);
	return ans;
}

 void enable(void)
{
	__asm(
		"sti"
/* 		:
		: */
		);
}
 void disable(void)
{
	__asm(
		"cli"
/*		:
		: */
		);
}

 void halt(void)
{
	kprintf("[i386] Halt called\n");
	__asm__("hlt");
	for (; ; );	
}





#ifndef __AMD64__
u16_t wswap( u16_t x )
{
    __asm (
	"xchgb %b0, %h0"
	: "=q" ( x )
	: "0" ( x ) );

    return x;	
}
#endif


#ifdef __AMD64__
void libtest()
{
	kprintf("libtest: links running...!\n");
}
#else
__local unsigned long long divandmod64(unsigned long long a, unsigned long long b, unsigned long long *remainder)
{
	unsigned long long result;
	int steps = sizeof(unsigned long long) * 8; 
	
	*remainder = 0;
	result = 0;
	
	if (b == 0) {
		/* FIXME: division by zero */
		return 0;
	}
	
	if ( a < b) {
		*remainder = a;
		return 0;
	}

	for ( ; steps > 0; steps--) {
		/* shift one bit to remainder */
		*remainder = ( (*remainder) << 1) | ((a >> 63) & 0x1);
		result <<= 1;
		
		if (*remainder >= b) {
				*remainder -= b;
				result |= 0x1;
		}
		a <<= 1;
	}

	return result;
}

/* 64bit remainder of the unsigned division */
unsigned long long __umoddi3(unsigned long long a, unsigned long long b)
{
	unsigned long long rem;
	divandmod64(a, b, &rem);
	return rem;
}

unsigned long long __udivmoddi3(unsigned long long a, unsigned long long b, unsigned long long *c)
{
	return divandmod64(a, b, c);
}

#endif
semaphore_t atomic_swap(volatile semaphore_t * _p, semaphore_t _x)
{
	semaphore_t __s = _x;

	__asm volatile ("xchgl %2, %0\n"
			: "=r" (__s)
			: "0" (__s), "m" (*_p));	

	return __s;
} 



