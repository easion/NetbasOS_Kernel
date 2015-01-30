
#ifndef __PROTECT_H__
#define __PROTECT_H__


#include <type.h>
#include <arch/x86/regs.h>
#include <arch/x86/tss.h>



#define KERNEL_VIDEO 12
#define KERNEL_TSS 13
#define VM86_TSS_POS 14
#define VM86_TASK_TSS_SEL 14*DESC_SIZE
#define FIRST_LDT 16

//#define PROC_LDT_SEL(n) ((FIRST_LDT+n*2)*DESC_SIZE)
//#define PROC_TSS_SEL(n) ((FIRST_LDT+n*2+1)*DESC_SIZE)

#define CS_LDT_INDEX     	1	/* process CS */
#define DS_LDT_INDEX     	2	/* process DS=ES=FS=GS=SS */

#define LDT_CS		((CS_LDT_INDEX<<3)|0x07)
#define LDT_DS		((DS_LDT_INDEX<<3)|0x07)

/* process (n)'s tss and ldt selector */
#define PROC_TSS(n) 	((((unsigned long) n)<<4)+(FIRST_TSS<<3))
#define PROC_LDT(n)		((((unsigned long) n)<<4)+(FIRST_LDT<<3))

#define KERNEL_CS  0x08  //0x0f
#define KERNEL_DS  0x10  //0x17

#define USER_CS  0x0f
#define USER_DS  0x17

#define KTHREAD_CS  0x38
#define KTHREAD_DS  0x40


#define USER_CODE_SEG 0xfa
#define USER_DATA_SEG  0xf2
#define KRNL_CODE_SEG 0x9a
#define KRNL_DATA_SEG  0x92

#define MAX_GDT 8192
#define MAX_IDT    256
#define DESC_SIZE 8

#define INT_GATE_TYPE	14
#define TSS_TYPE	    9
#define PRESENT      0x80	/* Present                                 */

#define LIMIT_1M 0xfffff
#define LIMIT_8M 0x7fffff
#define LIMIT_4G 0xffffffff

//#define DATA_BASE   0
//#define vir2phys(vir)	(DATA_BASE + (unsigned int) (vir))

typedef struct
{
  u16_t limit_low;  //3fff, 64M
  u16_t base_low;  //in kernel cs is 0

  u8_t base_middle;
  u8_t access;  //Attribute Bit: 9a (k: cs)
 
  u8_t granularity;  //c0
  u8_t base_high;
}sys_desc __attr_packet;

typedef struct 
{
	u16_t	offset_0;
	u16_t	selector;
	u16_t	type;
	u16_t	offset_16;
}gate_desc __attr_packet;

  void set_desc (sys_desc *_seg, u32_t   base, u32_t   size, u8_t type);
 u32_t  get_base(u16_t sel);

 unsigned long get_cr2 (void);
unsigned long get_cr3 (void);
 unsigned short get_cs (void);
 unsigned short get_ds (void);
 void get_gs (unsigned short *);
 void put_gs (unsigned short);
 void lldt (unsigned short ldt_sel);
 void ltr( int gdt_sel );
//system other
extern struct tss_struct ktss;

extern sys_desc gdt[MAX_GDT];//, idt;
extern gate_desc idt[MAX_IDT];

/*tss utils*/
void tss_copyto(struct tss_struct *to, struct tss_struct * from);
void tss_movein(int nr, u32_t* from);


#define get_seg_limit(segment) ({ \
unsigned long __limit; \
__asm__("lsll %1,%0\n\tincl %0":"=r" (__limit):"r" (segment)); \
__limit;})

static inline unsigned long  get_desc_base(sys_desc *_seg)
{
	unsigned long  base;

	base = _seg->base_low | ((unsigned long) _seg->base_middle << 16);
	base |= ((unsigned long) _seg->base_high << 24);

	return base;
}

#define RUN_RING   0      /* Not too sure */

#define DESCRIPTOR_TABLE(i)         gdt[i]
#define DESCRIPTOR_TABLE_INF        80
#define DESCRIPTOR_TABLE_SUP        256
#define INDEX_TO_SELECTOR(x)        (((x)*8) | RUN_RING)



#endif  //__PROTECT_H__


