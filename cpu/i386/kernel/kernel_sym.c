/*
**     (R)Jicama OS
**     Main Function
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/module.h>
#include <jicama/grub.h>
#include <jicama/coff.h>
#include <jicama/timer.h>
#include <jicama/msgport.h>
#include <jicama/proc_entry.h>
#include <arch/x86/io.h>
#include <string.h>
#include <errno.h>
#include <jicama/devices.h>
#include <jicama/console.h>




size_t fread(void *ptr, size_t size, size_t nmemb, register struct filp *stream);
__asmlink  int strnicmp(const char *s1, const char *s2, size_t count);
 __asmlink void do_gettime(struct tm*  toget);


semaphore_t atomic_swap(volatile semaphore_t * p, semaphore_t x);
__asmlink  u32_t  get_base(u16_t sel);
__asmlink void set_desc_base (u16_t mydesc,   u32_t   base);
__asmlink  int sel_to_ldt_index(u16_t Selector, u16_t *Index);
__asmlink void set_desc_limit (u16_t mydesc,  u32_t   size);
__asmlink  u32_t get_desc_limit (u16_t mydesc);
__asmlink  int key_cook(	int value,	func_t p);
__asmlink  int syscall_setup(int callnr, jicama_syscall_t fun);
__asmlink  int syscall_remove(int callnr);



__public struct _export_table_entry arch_sym_table []=
{
	EXPORT_PC_SYMBOL(swap_char),
	EXPORT_PC_SYMBOL(atomic_swap),
	/*sys call*/
	EXPORT_PC_SYMBOL(get_base),	
	EXPORT_PC_SYMBOL(set_desc_base),	
	EXPORT_PC_SYMBOL(set_desc_limit),	
	EXPORT_PC_SYMBOL(get_desc_limit),	

	{"inportb", inb},
	{"outportb", outb},

	EXPORT_PC_SYMBOL(inb),
	EXPORT_PC_SYMBOL(inw),
	EXPORT_PC_SYMBOL(inl),
	EXPORT_PC_SYMBOL(outb),
	EXPORT_PC_SYMBOL(outw),
	EXPORT_PC_SYMBOL(outl),

	EXPORT_PC_SYMBOL(low_alloc),
	EXPORT_PC_SYMBOL(low_free),

	EXPORT_PC_SYMBOL(write_user_dword),
	EXPORT_PC_SYMBOL(write_user_word),
	EXPORT_PC_SYMBOL(write_user_byte),
	EXPORT_PC_SYMBOL(read_user_byte),
	EXPORT_PC_SYMBOL(read_user_word),
	EXPORT_PC_SYMBOL(read_user_dword),

		/*dma*/
	EXPORT_PC_SYMBOL(enable_dma),
	EXPORT_PC_SYMBOL(disable_dma),
	EXPORT_PC_SYMBOL(isadma_stopdma),
	EXPORT_PC_SYMBOL(isadma_startdma),
	EXPORT_PC_SYMBOL(set_dma_addr),
	EXPORT_PC_SYMBOL(set_dma_count),
	EXPORT_PC_SYMBOL(set_dma_page),
	EXPORT_PC_SYMBOL(set_dma_mode),
	EXPORT_PC_SYMBOL(clear_dma_ff),

	EXPORT_DATA_SYMBOL(sys_boot_info),
	EXPORT_DATA_SYMBOL(gdt),
	EXPORT_DATA_SYMBOL(idt),
		/*vm86&io*/
	EXPORT_PC_SYMBOL(do_int86),
	EXPORT_PC_SYMBOL(get_int86_sp0),
	//EXPORT_PC_SYMBOL(read_real_memory),
	//EXPORT_PC_SYMBOL(fill_real_memory),
	//EXPORT_PC_SYMBOL(real_memory_addr),
	EXPORT_PC_SYMBOL(mdelay),

	EXPORT_PC_SYMBOL(put_syscall),
	EXPORT_PC_SYMBOL(put_interrupt),
	EXPORT_PC_SYMBOL(syscall_setup),
	EXPORT_PC_SYMBOL(syscall_remove),
	//EXPORT_PC_SYMBOL(proc_vis_addr),
	EXPORT_PC_SYMBOL(get_unixtime),
	EXPORT_PC_SYMBOL(sscanf),
	EXPORT_PC_SYMBOL(uart_putchar),

};

void arch_sym_setup(void)
{
	int arch_nr =sizeof(arch_sym_table)/sizeof(struct _export_table_entry);
	install_dll_table("i386.krnl", 1,arch_nr, arch_sym_table);
}


