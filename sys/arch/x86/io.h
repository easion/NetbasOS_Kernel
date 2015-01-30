#ifndef IO_H
#define IO_H

#include <type.h>
////here function all come from arch/entry.s file

extern void nop(void);
extern void delay(int sec);
extern  unsigned in(unsigned port);
extern  void out(unsigned port, u8_t value);

void outw(unsigned short port, unsigned short );
void outl(unsigned short port, unsigned long lo);
void outb(unsigned port, unsigned char val);
unsigned char inb(unsigned short port);
unsigned long inl(unsigned short port);
unsigned short inw(unsigned short port);
void enable(void);
void disable(void) ;
 void lldt (unsigned short ldt_sel);
u32_t get_cr4(void);

#define	inportb  inb
#define inportw inw
#define inportl inl

#define	outportb  outb
#define outportw outw
#define outportl outl


#define outb_(port, val) \
({ asm volatile("outb %0, %1" : : "a" ((unsigned char)(val)) , "d" ((unsigned short)(port))); })


#define io_wait()   inb(0x80)

static inline int BCD2BIN(int var){
	return (((var >> 4) & 0x0f) * 10) + (var & 0x0f);
}

static inline int BIN2BCD(int var){
	return ( (((var) / 10) << 4) + ((var) % 10) );
}

//#define BCD2BIN(n) ( ((n) >> 4) * 10 + ((n) & 0x0F) )
//#define BIN2BCD(n) ( (((n) / 10) << 4) + ((n) % 10) )

static inline int cmos_read (int pos)
  {
    outb (0x70, pos);
    asm ("jmp 1f\n1:");
    return inb (0x71);
  }

static inline void cmos_write (int pos, int value)
  {
    outb (0x70, pos);
    asm ("jmp 1f\n1:");
    outb (0x71, value);
  }

#ifdef __AMD64__

/*restore eflags*/
static __inline void restore_eflags(unsigned flags)
{
	__asm__ __volatile__("pushq %0\n"
		"popfq"
		:: "m"(flags));
}

/*save it*/
static __inline unsigned save_eflags(unsigned *flags)
{
	unsigned ret_val;

	__asm__ __volatile__("pushfq\n"
		"popq %0\n"
		"cli"
		: "=m"(ret_val)
		:);

		if (flags)
		{
		*flags = ret_val;
		}

	return ret_val;
}

#else 
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

		if (flags)
		{
		*flags = ret_val;
		}

	return ret_val;
}

#endif

inline static void _insw(int port, void* va, int count)
{
   __asm__ __volatile__ ("rep\n\t" \
               "insw" :: "d" (port), "D" (va), "c" (count));
}  

inline static void _outsw(int port, void* va, int count)
{
   __asm__ __volatile__ ("rep\n\t" \
               "outsw" :: "d" (port), "S" (va), "c" (count));
}


#endif

