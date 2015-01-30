

#ifndef __IA_H__
#define __IA_H__

#ifdef __cplusplus
extern "C" {
#endif

#if 0

void outw(unsigned short port, unsigned short );
void outl(unsigned short port, unsigned long lo);
void outb(unsigned port, unsigned val);
unsigned char inb(unsigned short port);
unsigned long inl(unsigned short port);
unsigned short inw(unsigned short port);
#endif

#ifndef __ARM__
 static inline void intel_outb(unsigned port, unsigned val)
{
	__asm(
		"outb %b0,%w1"
		:
		: "a"(val), "d"(port));
}

 static inline  unsigned char inportb(unsigned short port)
{
    unsigned char _tmp__; 

	__asm volatile("inb %1, %0" : "=a" (_tmp__) : "d" (port)); 
	return _tmp__; 
}

static __inline void wait_on(void)
{
	int i;

	for (i = 0; i < 0xfff; i++)
	{  /*nothing to do.*/
	}
}

extern inline int BCD2BIN(int var){
	return (((var >> 4) & 0x0f) * 10) + (var & 0x0f);
}

extern inline int cmos_read (int pos)
  {
    intel_outb (0x70, pos);
    asm ("jmp 1f\n1:");
    return inportb (0x71);
  }

extern inline void cmos_write (int pos, int value)
  {
    intel_outb (0x70, pos);
    asm ("jmp 1f\n1:");
    intel_outb (0x71, value);
  }

/*restore eflags*/
static __inline void restore_eflags(unsigned flags)
{
	__asm__ __volatile__("pushl %0\n"
		"popfl"
		:: "m"(flags));
}

/*save it*/
static __inline unsigned save_eflags(unsigned *flags)
{
	unsigned ret_val;

	__asm__ __volatile__("pushfl\n"
		"popl %0\n"
		"cli"
		: "=a"(ret_val)
		:);
		*flags = ret_val;

	return ret_val;
}
 // #define io_wait()   inb(0x80)
#define LOCK_SCHED(s) ((s)=save_eflags(NULL))
#define UNLOCK_SCHED(s) (restore_eflags(s))

inline static void nop()
{
	asm volatile ("nop");
}
#endif

#ifdef __cplusplus
}
#endif

#endif


