
#include <jicama/system.h>
#include <jicama/process.h>
#include <arch/x86/io.h>
#include <string.h>
#include <arch/x86/keyboard.h>
#include <jicama/paging.h>
#include <jicama/msgport.h>
#include <arch/x86/traps.h>
#include <assert.h>

#define V86_TASK_STACK_START	0x90000
#define V86_START_ADDRESS	0x100

#include "vm86.h"

__asmlink u32_t init_cr0 ;
unsigned short vm86_current_ctx_sel;

__local struct {
     struct tss_struct  ctx_regs;
    u8_t ctx_fpu[FPU_CONTEXT_SIZE] __attr_packet;
    u32_t ctx_iomap[2048];
} vm86_task;

__local u8_t  vm86_stack0[VM86_STACK_SIZE];
regs_t *vmback_regs;


__local u16_t vm86_context_save(void);

 void vm86_context_load(u16_t c);

//! The page size.
#define VM86_PAGE_SIZE	( 1UL << 12 )
//__local void* vm86_stack;

static int int86_sem_id;
void vm86_tss_init(void)
{
    register int  i;

    set_desc (&gdt[VM86_TSS_POS], (unsigned long)&vm86_task, sizeof(vm86_task), 0x89); //0X14
  
    vm86_task.ctx_regs.sp1 = 0;
    vm86_task.ctx_regs.sp2 = 0;
    vm86_task.ctx_regs.ss1 = 0;
    vm86_task.ctx_regs.ss2 = 0;
    vm86_task.ctx_regs.ldt = 0;
    vm86_task.ctx_regs.cr3 = 0;
    vm86_task.ctx_regs.trap = 0;
   
    vm86_task.ctx_regs.iobase =
		(u32_t)(&(vm86_task.ctx_iomap)) -	(u32_t)(&(vm86_task));

    for (i = 0; i < 2047; i++)
		vm86_task.ctx_iomap[i] = 0;
    vm86_task.ctx_iomap[2047] = 0xFF000000;	

	//s->spin.lock = SPINLOCK_UNLOCKED;

	int86_sem_id = create_semaphore("iomux",0,1);
	ASSERT(int86_sem_id>0);
}


struct tss_struct *vm86_get_tss(void)
{
    return &(vm86_task.ctx_regs);
}
__local u16_t vm86_context_save(void)
{
	vm86_current_ctx_sel = get_TR();
	return vm86_current_ctx_sel;
}

void entry_proc(u16_t);
 void vm86_context_load(u16_t c)
{
	disable();
	//kprintf("vm86_context_load");
	entry_proc(c);
}


void vm86_get(u32_t *addr, int *sz)
{
	*addr = (unsigned long)&vm86_task;
	*sz = sizeof(vm86_task);
}

void v86_enable_port(unsigned port)
{
	unsigned mask;

	mask = 0x01 << (port & 7);
	port >>= 3;
	vm86_task.ctx_iomap[port] &= ~mask;
}


/*
FLAGS - Intel 8086 Family Flags Register
      |11|10|F|E|D|C|B|A|9|8|7|6|5|4|3|2|1|0|
        |  | | | | | | | | | | | | | | | | +---  CF Carry Flag
        |  | | | | | | | | | | | | | | | +---  1
        |  | | | | | | | | | | | | | | +---  PF Parity Flag
        |  | | | | | | | | | | | | | +---  0
        |  | | | | | | | | | | | | +---  AF Auxiliary Flag
        |  | | | | | | | | | | | +---  0
        |  | | | | | | | | | | +---  ZF Zero Flag
        |  | | | | | | | | | +---  SF Sign Flag
        |  | | | | | | | | +---  TF Trap Flag  (Single Step)
        |  | | | | | | | +---  IF Interrupt Flag
        |  | | | | | | +---  DF Direction Flag
        |  | | | | | +---  OF Overflow flag
        |  | | | +-----  IOPL I/O Privilege Level  (286+ only)
        |  | | +-----  NT Nested Task Flag  (286+ only)
        |  | +-----  0
        |  +-----  RF Resume Flag (386+ only)
        +------  VM  Virtual Mode Flag (386+ only)
        - see   PUSHF  POPF  STI  CLI  STD  CLD

*/

CREATE_SPINLOCK( x8086_lock_sem );
CREATE_SPINLOCK( biosint_lock_sem );
void flush_tbl_one(pte_t pgaddr);
void reset_regs_stat();

int exec_8086(long vm_addr, long code_from, int sz)
{
	int i;
	long run_point;
	reg_t flags;

	lock_semaphore(int86_sem_id);
	flush_tbl_one(0);

	if (vm_addr<V86_START_ADDRESS||vm_addr>0x100000)	{
	run_point=V86_START_ADDRESS;
	}
	else{
	run_point=vm_addr;
	}
	kprintf("run_point at 0x%x\n", run_point);

	// It is not necessary to null the stack to enforce page	//
	// mapping, because V86-mode tasks run permanently at CPL3.	//
	// So a stack-fault is managed like a normal page-fault.	//
	vm86_task.ctx_regs.ss  = SEGMENT((u32_t)V86_TASK_STACK_START-sizeof(u32_t));
	vm86_task.ctx_regs.esp = OFFSET((u32_t)(V86_TASK_STACK_START-sizeof(u32_t)));

	vm86_task.ctx_regs.eflags = EFLAGS_VM | EFLAGS_IOPL | EFLAGS_IF|0x02;
	vm86_task.ctx_regs.ss0 = KERNEL_DS;
	vm86_task.ctx_regs.sp0 = (u32_t)&(vm86_stack0[VM86_STACK_SIZE-1]); 
	/////////////////////
	vm86_task.ctx_regs.cr3 = get_cr3();
	///////////////////
	vm86_task.ctx_regs.ds =    vm86_task.ctx_regs.es =
	vm86_task.ctx_regs.fs = vm86_task.ctx_regs.gs= SEGMENT((u32_t)run_point);

	vm86_task.ctx_regs.cs= SEGMENT((u32_t)run_point);
	vm86_task.ctx_regs.eip = OFFSET((u32_t)run_point);

	vm86_task.ctx_regs.backlink = vm86_context_save();

	reset_regs_stat();

	memcpy((void *)run_point, (void *)code_from, (u16_t)sz);

	vm86_context_load(VM86_TASK_TSS_SEL);

	if(!vmback_regs)
		panic("foo: vmback_regs NULL!\n");

	unlock_semaphore(int86_sem_id);
	return (vmback_regs->eflags&1);
}

//////////////////////////////////////////////////////////////////////////////////
u32_t get_int86_sp0()
{
	ASSERT(vm86_task.ctx_regs.sp0);
	return vm86_task.ctx_regs.sp0;
}

int do_int86(int service,vm86regs_t *inregs, vm86regs_t *outregs)
{
	reg_t flags;
	int ret=0;

	if (service < 0x10 || inregs == NULL){
		kprintf("do_int86 error: no input args!\n");
		return -1;
	}

	lock_semaphore(int86_sem_id);
	flush_tbl_one(0);

	vm86_task.ctx_regs.ss = 0;
	vm86_task.ctx_regs.esp = vm86_task.ctx_regs.ebp = V86_START_ADDRESS;
	vm86_task.ctx_regs.cr3 = get_cr3();

	vm86_task.ctx_regs.eflags = EFLAGS_VM | EFLAGS_IOPL | EFLAGS_IF;
	vm86_task.ctx_regs.ss0 = KERNEL_DS;
	vm86_task.ctx_regs.sp0 = (u32_t)&(vm86_stack0[VM86_STACK_SIZE-1]); 

	vm86_task.ctx_regs.ds = (u16_t)inregs->x.ds;
	vm86_task.ctx_regs.es = (u16_t)inregs->x.es;
	/*not used*/
	vm86_task.ctx_regs.fs = vm86_task.ctx_regs.gs= 0; 

	vm86_task.ctx_regs.eax = (u32_t)inregs->x.ax;/*fill data regs*/
	vm86_task.ctx_regs.ebx = (u32_t)inregs->x.bx;
	vm86_task.ctx_regs.ecx = (u32_t)inregs->x.cx;
	vm86_task.ctx_regs.edx = (u32_t)inregs->x.dx;
	vm86_task.ctx_regs.esi = (u32_t)inregs->x.si;
	vm86_task.ctx_regs.edi = (u32_t)inregs->x.di;

	#define INTEL86_INT_CODE 0xcd
	#define INTEL86_HALT_CODE 0xF4

	poke8(V86_START_ADDRESS, INTEL86_INT_CODE);// INT x
	poke8(V86_START_ADDRESS+1, service);
	poke8(V86_START_ADDRESS+2, INTEL86_HALT_CODE); // HLT

	vm86_task.ctx_regs.cs = 0;
	vm86_task.ctx_regs.eip = V86_START_ADDRESS;

	vm86_task.ctx_regs.backlink = vm86_context_save();

	reset_regs_stat();
	//kprintf("vm86_context_load begin\n");

	vm86_context_load(VM86_TASK_TSS_SEL);

	//kprintf("vm86_context_load() 0x%x eax_org 0x%x flg %x\n", vmback_regs->eax, vmback_regs->eax_org,vmback_regs->eflags);

	if(!vmback_regs)
		kprintf("do_int86: vmback_regs NULL!\n");

	if (outregs)
	{
		outregs->d.eax = vmback_regs->eax;
		outregs->d.ebx = vmback_regs->ebx;
		outregs->d.ecx = vmback_regs->ecx;
		outregs->d.edx = vmback_regs->edx;
		outregs->d.esi = vmback_regs->esi;
		outregs->d.edi = vmback_regs->edi;
		outregs->x.flags = (u16_t)vmback_regs->eflags;
		outregs->x.es = vm86_task.ctx_regs.es;
		outregs->x.ds = vm86_task.ctx_regs.ds;	
		ret = (outregs->x.flags & 1);
	}

	unlock_semaphore(int86_sem_id);
	return ret;
}

////////////////////////


typedef struct VESA_INFO {
	 unsigned char VESASignature[4] __attr_packet;
	 unsigned short VESAVersion __attr_packet;
	 unsigned long OEMStringPtr __attr_packet;
	 unsigned char Capabilities[4] __attr_packet;
	 unsigned long VideoModePtr __attr_packet;
	 unsigned short TotalMemory __attr_packet;
	 unsigned short OemSoftwareRev __attr_packet;
	 unsigned long OemVendorNamePtr __attr_packet;
	 unsigned long OemProductNamePtr __attr_packet;
	 unsigned long OemProductRevPtr __attr_packet;
	 unsigned char Reserved[222] __attr_packet;
	 unsigned char OemData[256] __attr_packet;
} VESA_INFO;

typedef struct MODE_INFO {
	 unsigned short ModeAttributes __attr_packet;
	 unsigned char WinAAttributes __attr_packet;
	 unsigned char WinBAttributes __attr_packet;
	 unsigned short WinGranularity __attr_packet;
	 unsigned short WinSize __attr_packet;
	 unsigned short WinASegment __attr_packet;
	 unsigned short WinBSegment __attr_packet;
	 unsigned long WinFuncPtr __attr_packet;
	 unsigned short BytesPerScanLine __attr_packet;

	 unsigned short XResolution __attr_packet;
	 unsigned short YResolution __attr_packet;
	 unsigned char XCharSize __attr_packet;
	 unsigned char YCharSize __attr_packet;
	 unsigned char NumberOfPlanes __attr_packet;
	 unsigned char BitsPerPixel __attr_packet;

	 unsigned char NumberOfBanks __attr_packet;
	 unsigned char MemoryModel __attr_packet;
	 unsigned char BankSize __attr_packet;
	 unsigned char NumberOfImagePages __attr_packet;
	 unsigned char Reserved_page __attr_packet;
	 unsigned char RedMaskSize __attr_packet;
	 unsigned char RedMaskPos __attr_packet;
	 unsigned char GreenMaskSize __attr_packet;
	 unsigned char GreenMaskPos __attr_packet;
	 unsigned char BlueMaskSize __attr_packet;
	 unsigned char BlueMaskPos __attr_packet;
	 unsigned char ReservedMaskSize __attr_packet;
	 unsigned char ReservedMaskPos __attr_packet;
	 unsigned char DirectColorModeInfo __attr_packet;
	 unsigned long PhysBasePtr __attr_packet;
	 unsigned long OffScreenMemOffset __attr_packet;
	 unsigned short OffScreenMemSize __attr_packet;
	 unsigned char Reserved[206] __attr_packet;
} MODE_INFO;

#define REAL_VM_MEM (unsigned)0X9000

int get_vbe_info(VESA_INFO *info)
{
	vm86regs_t reg86;

	reg86.x.ax = 0x4F00;

	reg86.x.di = FP_OFF(REAL_VM_MEM);
	reg86.x.es = FP_SEG(REAL_VM_MEM);

	memcpy((void*)REAL_VM_MEM, "VBE2", 4);

	do_int86( 0x10, &reg86,&reg86 );

	/* Mode info did not work */
	if (reg86.x.ax!=0x004f)	  return -1;

	memcpy((char *)info, (void*)REAL_VM_MEM, sizeof(VESA_INFO));

	if (strncmp((const char *) info->VESASignature, "VESA", 4) != 0){
	kprintf("No vesa mode Found!\n");
	return -1;
	}

	kprintf("%s Found!\n", info->VESASignature);
	return 0;
}

