
#include <arch/x86/io.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <errno.h>
#include <string.h>
#include <arch/x86/traps.h>
#include <assert.h>

#define ARGS_OFFSET 8

int sel_to_ldt_index(u16_t Selector, u16_t *Index);
void set_userdata(sys_desc *_seg, u32_t  base,u32_t   size)  ;
void set_usercode(sys_desc *_seg, u32_t  base, u32_t  size)  ;
proc_t* alloc_pcb(int n);
__local int build_kernel_ldt_space(proc_t *rp);
 int free_gdt_entry(int _entry);
void set_debug_regs( long regs[8] );
long reset_db7_status();


 /* NOTE:interrupt flag ENABLED. Needed when all kernel
     tasks are sleeping, cause else no IRQ0, no one
     can wake up, sleep forever, people call it
     a deadlock. I call it bad luck.

	 If eflag is 0x2;  interrupts disabled 
    */
 void set_desc_base (u16_t mydesc,   u32_t   base)
{
	sys_desc *_seg;
	u16_t idx;

	sel_to_ldt_index(mydesc, &idx);

	_seg=&gdt[idx];

	_seg->base_low = base;
	_seg->base_middle = base >> 16;
	_seg->base_high = base >> 24;
}

 void set_desc_limit (u16_t mydesc,  u32_t   size)
{
	sys_desc *_seg;
	u16_t idx;

	sel_to_ldt_index(mydesc, &idx);

	_seg=&gdt[idx];

	--size;
  	if (size > 0xFFFFFL) {  //if used visual memory.
		_seg->limit_low = size >> 12;
		_seg->granularity = 0x80 | (size >> (12 + 16));
  	} else {                             //used physical memory
		_seg->limit_low = size;
		_seg->granularity = size >> 16;
  	}

  	_seg->granularity |= 0x40;	/* means BIG for data seg */
}

 u32_t get_desc_seg_limit (sys_desc *_seg)
 {
	 u32_t   size;

	size =_seg->granularity<<16;
	size |=	_seg->limit_low << 12;
	return size;
 }

u32_t get_desc_limit (u16_t mydesc)
 {
	 u32_t   size;
	sys_desc *_seg;
	u16_t idx;

	sel_to_ldt_index(mydesc, &idx);

	_seg=&gdt[idx];

	size =_seg->granularity<<16;
	size |=	_seg->limit_low << 12;
	return size;
 }

void set_desc (sys_desc *_seg, u32_t   base, u32_t   size, u8_t type)
{
	_seg->base_low = base;
	_seg->base_middle = base >> 16;
	_seg->base_high = base >> 24;

	--size;
  	if (size > 0xFFFFFL) {  //if used visual memory.
		_seg->limit_low = size >> 12;
		_seg->granularity = 0x80 | (size >> (12 + 16));
  	} else {                             //used physical memory
		_seg->limit_low = size;
		_seg->granularity = size >> 16;
  	}

  	_seg->granularity |= 0x40;	/* means BIG for data seg */
  	_seg->access =  type;  //segment type
}

int build_user_ldt_space(proc_t *rp)
{
	int nr = rp->p_asid;
	ASSERT( rp != NIL_PROC);

	set_desc(&(rp->p_ldt[CS_LDT_INDEX]), (unsigned long) USER_SPACE_ADDR,
				(unsigned long) USER_SPACE_SIZE, USER_CODE_SEG);
	set_desc(&(rp->p_ldt[DS_LDT_INDEX]), (unsigned long) USER_SPACE_ADDR,
				(unsigned long) USER_SPACE_SIZE, USER_DATA_SEG);

	return (OK);
}




__local int build_kernel_ldt_space(proc_t *rp)
{
	int nr = proc_number(rp);

	ASSERT( rp != NIL_PROC);
	ASSERT(!IS_USER_TASK(nr));

	set_desc(&(rp->p_ldt[CS_LDT_INDEX]), (unsigned long) 0,
				(unsigned long) 0xffffffff, KRNL_CODE_SEG);
	set_desc(&(rp->p_ldt[DS_LDT_INDEX]), (unsigned long) 0,
				(unsigned long) 0xffffffff, KRNL_DATA_SEG);

	return (OK);
}

//TSS item:27	** {backlink; sp0;        ss0;        sp1;ss1;sp2;ss2;  
//cr3;ip;        flags;ax;cx;dx;bx;sp;
//		bp;si;di;es;cs;         ss;         ds;         fs;gs;ldt;trap;iobase;map} 



int thread_tss_init(thread_t* pthread, unsigned entry, unsigned ustack, proc_t *rp)
{
	int tss_sel_enty = get_gdt_entry();
	int number = proc_number(rp);
	struct tss_struct *tss_regs;

	ASSERT(pthread != NIL_THREAD);
	ASSERT(rp != NIL_PROC);
	ASSERT(ustack != 0);

 	tss_regs = &pthread->tss;
	memset((void*)tss_regs, 0, sizeof(struct tss_struct));

	if (tss_sel_enty == 0)	{
		return -1;
	}

	pthread->k_stack = get_page()+PAGE_SIZE; //for kernel space
	pthread->u_stack = ustack; //for user space

	memset(pthread->debug_regs,0,sizeof(pthread->debug_regs));
       
	//SET PROCESS TSS: first is 9.
	set_desc(&gdt[tss_sel_enty], (unsigned int)(&(pthread->tss)),
		(unsigned long) sizeof pthread->tss, 0x89);

	pthread->th_tss_sel = tss_sel_enty  * DESC_SIZE;

	tss_regs->ldt = rp->ldt_sel;
	tss_regs->iobase = 104; //0xffff;//sizeof pthread->tss;
	tss_regs->eip = (long)entry;
	tss_regs->cr3 = rp->proc_cr3;
	tss_regs->ss0 = KERNEL_DS;

	/*zero regs.*/
	/*tss_regs->eax=0 ; 
	tss_regs->ecx=0 ;
	tss_regs->edx=0 ;
	tss_regs->ebx=0 ;
	tss_regs->esi=0;
	tss_regs->edi=0 ; */
	tss_regs->backlink = 0;
	//tss_regs->eflags	= 0x203246;	/*	ReadFlags() & ~0x200;	*/
	tss_regs->eflags	= 0x200;	/*	ReadFlags() & ~0x200;	*/
	//tss_regs->sp0 = PROC_KSTACK_TOP(number);
	tss_regs->sp0 = pthread->k_stack;
	tss_regs->iobase = 104;
	tss_regs->esp = pthread->u_stack-ARGS_OFFSET;

   if(IS_USER_TASK(number)){
	tss_regs->es=USER_DS & 0xffff;
	tss_regs->cs=USER_CS & 0xffff;
	tss_regs->ss=USER_DS & 0xffff;
	tss_regs->ds =USER_DS& 0xffff;
	tss_regs->fs=USER_DS & 0xffff;
	tss_regs->gs =USER_DS& 0xffff;
	tss_regs->ebp = 0;//ustack;
	//kprintf("user thread at %d -- %d ok\n", rp->p_asid, number);
   } else 
	if(number < 2){
	tss_regs->es=KTHREAD_DS & 0xffff;
	tss_regs->cs=KTHREAD_CS & 0xffff;
	tss_regs->ss=KTHREAD_DS & 0xffff;
	tss_regs->ds =KTHREAD_DS& 0xffff;
	tss_regs->fs=KTHREAD_DS & 0xffff;
	tss_regs->gs =KTHREAD_DS& 0xffff;
    tss_regs->eflags = 0x0000202; 	
	//tss_regs->esp = get_page()+PAGE_SIZE;  //warn :  rewrite it

 //  kprintf("a kernel thread\n");
   }else{
	   panic("%s() : make err task %d\n", number);
   }

   if (tss_regs->esp<= PAGE_SIZE)
   {
	   return -1;
   }

	/*read fpu info*/
	save_fpu_state(pthread->fpu_state_info);
	pthread->current_cr2 = get_cr2();
	pthread->ticks = 0;

	pthread->sticks=0;
	return 0;
}

void arch_thread_free(thread_t* pthread)
{
	ASSERT(pthread != NIL_THREAD);

	free_page(pthread->k_stack - PAGE_SIZE);
	free_gdt_entry(pthread->th_tss_sel / DESC_SIZE);
}

int arch_cr3_init(proc_t *rp)
{
	int i;
	pte_t *pgdir = (pte_t*)KERN_PG_DIR_ADDR;

	if (!IS_USER_TASK(proc_number(rp)) )
	{
		rp->proc_cr3 = pgdir;
		return 0;
	}

	rp->proc_cr3 = get_page();

	if (!rp->proc_cr3)
	{
		return -1;
	}

	for (i=0; i<1024; i++)
	{
		rp->proc_cr3[i] = pgdir[i];
	}

	for (i=512; i<896; i++)
	{
		//用户进程使用2-3G之间的地址,1.5g
		rp->proc_cr3[i] = 0;
	}
	return 0;
}

int arch_proc_init(proc_t *rp)
{
	unsigned ds_base;
	unsigned short  ldt_sel_enty;

	if (arch_cr3_init(rp))
	{
		return -1;
	}

	ldt_sel_enty = get_gdt_entry();

	//no gdt entry
	if (ldt_sel_enty == 0){
		return -1;
	}

   //SET PROCESS LDT: first is 8.
	set_desc(&gdt[ldt_sel_enty], (unsigned int)(rp->p_ldt),
		(unsigned long) sizeof rp->p_ldt, 0x82);  //0x82: ldt

	rp->ldt_sel = ldt_sel_enty*DESC_SIZE;


	if(IS_USER_TASK(proc_number(rp))) {

		trace("built user ldt ..\n");
		
		build_user_ldt_space(rp);

		map_proc(rp);
	}
	else{
		build_kernel_ldt_space(rp);
	}
	return 0;
}



void enable_iop(thread_t *pp)
{
/* Allow a user process to use I/O instructions.  Change the I/O Permission
 * Level bits in the psw. These specify least-privileged Current Permission
 * Level allowed to execute I/O instructions. Users and servers have CPL 3. 
 * You can't have less privilege than that. Kernel has CPL 0, tasks CPL 1.
 */
  pp->tss.eflags |= EFLAGS_IOPL;
}

__local void set_tss_bitmap(u32_t *bitmap, short base, short extent, int val)
{
	int bmask;
	u16_t lidx = base & 0x1f;
	int len = lidx + extent;
	u32_t *bmap_base = bitmap + (base >> 5);

	if (lidx != 0) {
		bmask = (~0 << lidx);
		if (len < 32)
				bmask &= ~(~0 << len);
		if (val)
			*bmap_base++ |= bmask;
		else
			*bmap_base++ &= ~bmask;
		len -= 32;
	}

	bmask = (val ? ~0 : 0);

	while (len >= 32) {
		*bmap_base++ = bmask;
		len -= 32;
	}

	if (len > 0) {
		bmask = ~(~0 << len);
		if (val)
			*bmap_base++ |= bmask;
		else
			*bmap_base++ &= ~bmask;
	}
}


__public int i386_ioperm(u32_t from, u32_t num, int turn_on)
{
	if (from + num <= from)return EINVAL;
	if (from + num > 32*32)return EINVAL;

	set_tss_bitmap((u32_t *)current_thread()->tss.iomap, from, num, !turn_on);
	return 0;
}


	
__local inline void call_tss (unsigned short tss_sel)
{
   unsigned char buf[6];

    buf[0]=0;
    buf[1]=0;
    buf[2]=0;
    buf[3]=0;
    buf[4]=tss_sel & 255;
    buf[5]=tss_sel >> 8;

  __asm ("lcall (%0)"::"g" (&buf));
}

__noreturn void jump_proc_tss (unsigned short tss_sel)
{
   unsigned char buf[6]; /*selector 48bit*/

    buf[0]=0;
    buf[1]=0;
    buf[2]=0;
    buf[3]=0;
    buf[4]=tss_sel & 255;
    buf[5]=tss_sel >> 8;
    
    //__asm ("ljmp (%%eax)": : "a" (&buf));
    __asm ("ljmp (%0)"::"g" (&buf));
}

void free_proc_mem_space(proc_t *rp)
{
	u32_t base = proc_phy_addr(rp);
	int nr=proc_number(rp);


	ASSERT(rp);
	ASSERT(IS_USER_TASK(proc_number(rp)));
	ASSERT(base);

	//kprintf("start free %s at %x\n", rp->p_name, base);

	unmap_proc(rp, true);

}

__noreturn void arch_thread_resume (thread_t *pfrom, thread_t *pto)
{
	int sw=0;
	unsigned short tss_sel;
	unsigned flags;
	ASSERT(pto);

	/*
	**fpu save
	*/
	if (pfrom) {
		clts();
		save_fpu_state(pfrom->fpu_state_info);
		pfrom->current_cr2 = get_cr2();		
	}
	save_eflags(&flags); 

	if ( pto->debug_regs[7] != 0 )
		set_debug_regs( pto->debug_regs );
	else{
		/* With an x86 CPU, clearing the debug control register on every task switch is less expensive than
		   keeping track of the register's contents and clearing it only when neccessary. */
		reset_db7_status();
	}

	boot_parameters.bp_switch++;

	tss_sel = pto->th_tss_sel;
	ASSERT (tss_sel%8 == 0);

	/*
	**fpu load 
	*/
	load_fpu_state(pto->fpu_state_info);

	set_current_thread(pto);

	

	restore_eflags(flags);

	jump_proc_tss(tss_sel); 

}




/*
**
*/
void soft_irq(int irq, regs_t reg)
{
	int sig;
	thread_t *current= current_thread();

	if(irq > 255)
		panic("soft_irq irq error, cs%d, irq=%d\n", reg.cs, irq);

	if (reg.cs!=USER_CS && reg.cs!=KTHREAD_CS)
	{
		panic("soft_irq cs error, cs%d, irq=%d\n", reg.cs, irq);
	}



	if ((current!=idle_task || (current->ticks == CPU_NO_TIMES)) && scheduleable())
	{
		 //switch_enable();
		schedule();
		return;
	 }

	
	
}

void sigflush(regs_t reg)
{
	thread_t *current= current_thread();
	process_signal(&reg,current);
}

