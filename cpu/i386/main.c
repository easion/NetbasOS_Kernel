/*
**     (R)Jicama OS
**      Intel i386 Architecture Main
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/process.h>
#include <arch/x86/io.h>
#include <string.h>
#include <arch/x86/keyboard.h>
#include <jicama/paging.h>
//////////////////////////////system init/////////
#include <jicama/grub.h>

__public grub_info_t sys_boot_info;
u32_t init_cr0=0, init_cr3=0;

void start64();
void draw_logo();
__asmlink int core_start( void* args);

#define _GDT_SIZE_   DESC_SIZE * MAX_GDT
#define _IDT_SIZE_     DESC_SIZE * MAX_IDT

__local unsigned char gdt_address [6];
__local int process_tss_init();
__local void enable_gate_a20( void );

__local void load_idt();

__asmlink void idt_init(void);
__asmlink void i8259_init(void);
void memory_init();
void tss_init();
void load_gdt();

void machine_start()
{
	//LOAD GDT TABLE
     tss_init();
}

//NOTE:TSS Access Value is0x89, But LDT is 0x82
//use eg.	init_tssdesc(TSS_SEL, (unsigned long)(void*)&ktss);
void set_tss_desc(int i, unsigned long tss_addr)
{
	gdt[i].base_low = tss_addr;
	gdt[i].base_middle = (tss_addr >> 16);
	gdt[i].base_high = (tss_addr >> 24);
	gdt[i].limit_low = sizeof(struct tss_struct);
	gdt[i].access = 0x89;  //dpl=0xe9, why? 
	gdt[i].granularity = 0x00;  //
}

__local void enable_gate_a20( void )
{
   // now we enable A20
   // keyboard controller (8042) port at 0060-0063
	//read keyboard status: wait empty
	waitempty ();  

    while ( inb( 0x64 ) & 0x01 ) 
	inb( 0x60 );  //it has output buffer?
    
	//command: double write output port,
    outb( 0x64, 0xd1 );  //send command, enable a20.
	waitempty ();  

	outb( 0x60, 0xdf );  //a20 on.
	waitempty ();  
}

__asmlink char *gdt_16bit_cs;
__asmlink char *gdt_16bit_ds, *gdt_kernel_code, *gdt_kernel_data;
__asmlink void vm86_get(u32_t *addr, int *sz);

void set_video_seg(u32_t base, u32_t sz)
{
    set_desc (&gdt[KERNEL_VIDEO], base, sz, USER_DATA_SEG); //VIDEO: 0x38
}

__asmlink unsigned int g_tss_end;

struct my_gdt
{
	u16_t w1;
	u16_t w2;
	u8_t b1;
	u8_t b2;
	u8_t b3;
	u8_t b4;
}__attr_packet;


 void system_init(void)
{
	struct my_gdt *gdtptr;

	memset((void*)idt, 0, _IDT_SIZE_ );

	i8259_init(); //or the system will die!

	//BEGIN LOAD IDT
	load_idt();

	memset((void*)gdt, 0, _GDT_SIZE_ );
    set_desc (&gdt[1], 0, LIMIT_4G, KRNL_CODE_SEG); //kernel code: 0x08
	set_desc (&gdt[2], 0, LIMIT_4G, KRNL_DATA_SEG);  //kernel data: 0x10
	set_desc (&gdt[3], 0, LIMIT_4G, USER_CODE_SEG);  //user code: 0x1b
	set_desc (&gdt[4], 0, LIMIT_4G, USER_DATA_SEG);  //user data: 0x23

 /*base on address 0*/
	gdtptr=(struct my_gdt  *)&gdt[5];
	gdtptr->w1=0xFFFF;
	gdtptr->w2=0;
	gdtptr->b1=0;
	gdtptr->b2=0x9E;
	gdtptr->b3=0;
	gdtptr->b4=0;

	gdtptr=(struct my_gdt  *)&gdt[6];
	gdtptr->w1=0xFFFF;
	gdtptr->w2=0;
	gdtptr->b1=0;
	gdtptr->b2=KRNL_DATA_SEG;
	gdtptr->b3=0;
	gdtptr->b4=0;

	set_desc (&gdt[7], 0, LIMIT_4G, KRNL_CODE_SEG);  //user code: 0x1b
	set_desc (&gdt[8], 0, LIMIT_4G, KRNL_DATA_SEG);  //user data: 0x23	
	//set_desc (&gdt[5], 0, LIMIT_1M, KRNL_CODE_SEG);  //kernel data: 0x1b
	//set_desc (&gdt[6], 0, LIMIT_1M, KRNL_DATA_SEG);  //kernel data: 0x23

    set_desc (&gdt[KERNEL_TSS], (unsigned long)&ktss, ((unsigned)&g_tss_end-(unsigned)&ktss-1), 0x89); //0X30
    set_desc (&gdt[KERNEL_VIDEO], 0xb8000, (0XC0000-0XA0000), USER_DATA_SEG); //VIDEO: 0x38

    gdt_address [0] = (_GDT_SIZE_ - 1) & 255;	/*  Low byte of limit  */
    gdt_address [1] = (_GDT_SIZE_ - 1) / 256;	/*  High byte of limit  */
    gdt_address [2] = (unsigned long)gdt & 255;	/*  Low byte of address  */
    gdt_address [3] = ((unsigned long)gdt >> 8) & 255;
    gdt_address [4] = ((unsigned long)gdt >> 16) & 255;
    gdt_address [5] = ((unsigned long)gdt >> 24) & 255;  /*  Highest byte  */

   __asm ("lgdt 	(%0)\n"
             "ljmp $0x08, $__flush\n"
	         "__flush:\n"      
    		 "movw 	%%ax,%%ds\n"
    		 "movw	%%ax,%%es\n"
		     "movw	%%ax,%%fs\n"
		     "movw	%%ax,%%gs\n"
		     "movw	%%ax,%%ss\n"
		     "movl	%1,%%esp\n" 
               "jmp 1f\n"
	           "1:\n"
			   "sgdt 	(%0)\n":: "r" ( &gdt_address ),"r" (KERN_PG_DIR_ADDR-1),"a" (0x10));
}

__local void load_idt(void)
{
     unsigned idt_desc[2];

	 //NOTE: DESCRIPTOR MUST ADD 2, WHY?
     idt_desc[0] = (_IDT_SIZE_ - 1)<<16;
     idt_desc[1] = (unsigned)idt;

     __asm ("lidt (%0)" : : "r" (((unsigned char *)idt_desc)+2));
}



__public int start_kernel(u32_t cpu_features, u32_t mmn, grub_info_t* boot_jicama)
{
	unsigned cur_flag;

	save_eflags(&cur_flag);

	enable_gate_a20();	
	cp_grub_info(boot_jicama);

	if (CHECK_GRUB(boot_jicama->flags,11)){
		//return -1;
		grub_read_lfb(boot_jicama);   
	}

	if ( mmn != MULTIBOOT_BOOTLOADER_MAGIC )	{
		return -1;
	}

	//memset(bss, 0, end-bss);

	init_cr0=get_cr0();
	init_cr3=get_cr3();

	set_cr0(init_cr0);
	restore_eflags(cur_flag);
    return core_start((void *)boot_jicama);
}

void fill_userdef_call(void);
void vm86_tss_init(void);
extern  void low_mem_init(grub_info_t* info);
int userspace_environmental_init(void);
extern void cpuinfo_init(void);
void nopcode(void);
int ser_init(void);
void set_idle_hook(void (*hook)());
void vm86_low_buffer_alloc(void);
void sym_init();


void arch_init2()
{
	unsigned long base_mm;

	disable();

	/*read args from grub*/
	//grub_get_kernel_boot_args();
	/*memory init etc ...*/
	machine_start();  //内存初始，一定要先初始表格后分页

	/*fill user define system call  table ...*/
	fill_userdef_call();
	/*dos memory manager init ...*/
	low_mem_init(&sys_boot_info);
	/*see apm table ...*/
	grub_apm();

	grub_mmap(&sys_boot_info);

	/*see grub verion ...*/
	grub_version();    //grub的版本
	grub_get_boot(&boot_parameters.bp_bootdev);//启动设备

	/*load module files form grub ...*/
	grub_load_modules(&sys_boot_info);

	/*see cpu informaton ...*/
	cpuinfo_init();




}

static void idle()
{
	int i=0;
	char *p=(char*)0xb8000+160-4;
	char *str = "/-|\\";
	
	for (i=0; i<400; i++)
	{
		nopcode();
		*p = str[i%4];
		//*(p+1) = 0x20;
	}
	
}

void arch_init3()
{
	/*int86() call fuction init ...*/
	vm86_tss_init();
	/*int86() buffer , but we not used now*/
	vm86_low_buffer_alloc();

	sym_init();
	ser_init(); //init device driver


	set_idle_hook(NULL);

	userspace_environmental_init();

}

#define _JEB_GLOBAL
 sys_desc gdt[MAX_GDT];
/*
 ={
	 {0,0,0,0,0,0}, //0
	{ 0xFFFF, 0x0000, 0x00, 0x9A, 0xCF, 0x00 },
	{ 0xFFFF, 0x0000, 0x00, KRNL_DATA_SEG, 0xCF, 0x00 },
	{ 0xFFFF, 0x0000, 0x00, 0xFA, 0xCF, 0x00 },
	{ 0xFFFF, 0x0000, 0x00, 0xF2, 0xCF, 0x00 }, 
	{0,0,0,0,0,0}, //*0
};
*/
 gate_desc idt[MAX_IDT];


#include <arch/amd64/asm.h>
#include <arch/amd64/msr.h>

void entry_long_mode()
{
	unsigned __eax, __edx;

	__eax=get_cr4(); 
	//__eax|=0x2a0;//bit 5,7,9 set
	__eax|=0xa0; //bit 5.7 set
	set_cr4(__eax); 

	rdmsr(MSR_K6_EFER, __eax, __edx);
	__eax|=0x0901;
	wrmsr(MSR_K6_EFER, __eax, __edx);	

	set_cr3(KERN_PG_DIR_ADDR);

	__eax=get_cr0();
	__eax|=(1<<31);/* Enable paging and in turn activate Long Mode */
	set_cr0(__eax);
}

