#ifndef regs_h
#define regs_h
typedef struct 
{
	unsigned long content[20];
}regs_t;

typedef struct 
{
	unsigned long r0;
	unsigned long r1;
	unsigned long r2;
	unsigned long r3;
	unsigned long r4;
	unsigned long r5;
	unsigned long r6;
	unsigned long r7;
	unsigned long r8;
	unsigned long r9;
	unsigned long r10;
	unsigned long fp;
	unsigned long ip;
	unsigned long sp;
	unsigned long lr;
	unsigned long pc;
	unsigned long cpsr;
	unsigned long ORIG_r0;
}interrupt_regs_t;

unsigned save_eflags(unsigned *flags);
void restore_eflags(unsigned flags);

#define proc_vir2phys(p, vir) \
	(u32_t)((vir))

#endif


