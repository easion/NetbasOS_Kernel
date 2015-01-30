#include <jicama/system.h>
#include <string.h>
#include <jicama/paging.h>
#include <ibm/protect.h>
#include <amd64/msr.h>
#include <amd64/dep.h>

#define ENABLE          0x20	// code used to re-enable after an interrupt 
#define MASTER         0x20	                // 首个中断控制端口 
#define MASTER_CTLMASK     0x21	//不允许中断设置位 setting bits in this port disables ints 
#define SLAVE       0xA0	                        // 第二个中断控制器端口
#define SLAVE_CTLMASK    0xA1	    //不允许中断设置位 setting bits in this port disables ints 
#define IRQ0		0x20


__local unsigned char gdt_address [6];
#define AMD64_MAX_GDT 16
#define _GDT_SIZE_   8 * AMD64_MAX_GDT

sys_desc g_GDT[AMD64_MAX_GDT] ={
	 {0,0,0,0,0,0}, /*1*/
{ 0xFFFF, 0x0000, 0x00, 0x9A, 0xCF, 0x00 }, //0x08 kernel 32 bit
{ 0xFFFF, 0x0000, 0x00, 0x92, 0xCF, 0x00 },
{ 0xFFFF, 0x0000, 0x00, 0xFA, 0xCF, 0x00 }, //0x18 user 32 bit
{ 0xFFFF, 0x0000, 0x00, 0xF2, 0xCF, 0x00 }, 
//64 bit
{ 0xFFFF, 0x0000, 0x00, 0x9a, 0xaf, 0x00 }, //0x28 kernel 64 bit
{ 0xFFFF, 0x0000, 0x00, 0xFA, 0xaf, 0x00 }, 
};

struct desc_ptr  gdtr = {.size = sizeof(g_GDT), .address= (u64_t) g_GDT };


inline void gdtr_load(struct desc_ptr *p_gdtr)
{
	__asm__ volatile ("lgdtq %0\n" : : "m" (*p_gdtr));
}

/** Store GDTR register to memory.
 *
 * @param p_gdtr Address of memory to where to load GDTR.
 */
inline void gdtr_store(struct desc_ptr *p_gdtr)
{
	__asm__ volatile ("sgdtq %0\n" : : "m" (*p_gdtr));
}


void load_gdt()
{
	gdtr_load(&gdtr);
}

#ifndef __AMD64__
void memset16(void* dst,unsigned short v,unsigned long len)
{
/*	unsigned long n;

	for(n=0;n<len;n+=2)
		*(((usnigned short*)dst)+n)=v;*/
	if(len&2) {
		__asm volatile (	"m2_1:\n"
				"movw %%ax, -2(%%ebx,%%ecx,2)\n"
				"loop m2_1\n"
				:
				: "a" ((unsigned short)v),
				  "b" ((unsigned long)dst),
				  "c" ((unsigned long)len)
				: "memory" );
	} else {
		__asm volatile (	"m2_2:\n"
				"movl %%eax, -4(%%ebx,%%ecx,4)\n"
				"loop m2_2\n"
				:
				: "a" ((unsigned long)(((unsigned long)v<<16)|((unsigned long)v))),
				  "b" ((unsigned long)dst),
				  "c" ((unsigned long)(len>>1))
				: "memory" );
	}
}

void memset32(void* dst,unsigned long v,unsigned long len)
{
/*	unsigned long n;

	for(n=0;n<len;n+=4)
		*(((unsigned long)dst)+n)=v;*/
	__asm volatile (	"m3_1:\n"
			"movl %%eax, -4(%%ebx,%%ecx,4)\n"
			"loop m3_1\n"
			:
			: "a" ((unsigned long)v),
			  "b" ((unsigned long)dst),
			  "c" ((unsigned long)len)
			: "memory" );
}
#endif

