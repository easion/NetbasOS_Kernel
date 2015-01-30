#include "boot.h"
#include "memstr.h"
#include "hypercall.h"
#include "page.h"
#include "xconsole.h"
start_info_t start_info;
memzone_t meminfo;

extern void xen_callback(void);
extern void xen_failsafe_callback(void);

void arch_pre_main(void)
{
	xen_vm_assist(VMASST_CMD_ENABLE, VMASST_TYPE_WRITABLE_PAGETABLES);
	
	page_specifier_t pte;
	memsetb((uintptr_t) &pte, sizeof(pte), 0);
	
	pte.present = 1;
	pte.writeable = 1;
	pte.frame_address = ADDR2PFN((uintptr_t) start_info.shared_info);
	xen_update_va_mapping(&shared_info, pte, UVMF_INVLPG);
	
	pte.present = 1;
	pte.writeable = 1;
	pte.frame_address = start_info.console_mfn;
	xen_update_va_mapping(&console_page, pte, UVMF_INVLPG);
	
	xen_set_callbacks(XEN_CS, xen_callback, XEN_CS, xen_failsafe_callback);
	
	/* Create identity mapping */
	
	meminfo.start = ADDR2PFN(ALIGN_UP(KA2PA(start_info.ptl0), PAGE_SIZE)) + start_info.pt_frames;
	meminfo.size = start_info.frames - meminfo.start;
	meminfo.reserved = 0;
	
	uintptr_t pa;
	int last_ptl0 = 0;
	for (pa = PFN2ADDR(meminfo.start); pa < PFN2ADDR(meminfo.start + meminfo.size); pa += FRAME_SIZE) {
		uintptr_t va = PA2KA(pa);
		
		if ((PTL0_INDEX(va) != last_ptl0) && (GET_PTL1_FLAGS(start_info.ptl0, PTL0_INDEX(va)) & PAGE_NOT_PRESENT)) {
			/* New page directory entry needed */
			uintptr_t tpa = PFN2ADDR(meminfo.start + meminfo.reserved);
			uintptr_t tva = PA2KA(tpa);
			
			memsetb(tva, PAGE_SIZE, 0);
			
			page_specifier_t *tptl3 = (page_specifier_t *) PA2KA(GET_PTL1_ADDRESS(start_info.ptl0, PTL0_INDEX(tva)));
			SET_FRAME_FLAGS(tptl3, PTL3_INDEX(tva), PAGE_PRESENT);
			SET_PTL1_ADDRESS(start_info.ptl0, PTL0_INDEX(va), tpa);
			
			last_ptl0 = PTL0_INDEX(va);
			meminfo.reserved++;
		}
		
		page_specifier_t *ptl3 = (page_specifier_t *) PA2KA(GET_PTL1_ADDRESS(start_info.ptl0, PTL0_INDEX(va)));
		
		SET_FRAME_ADDRESS(ptl3, PTL3_INDEX(va), pa);
		SET_FRAME_FLAGS(ptl3, PTL3_INDEX(va), PAGE_PRESENT | PAGE_WRITE);
	}
	
	/* Put initial stack safely in the mapped area */
	stack_safe = PA2KA(PFN2ADDR(meminfo.start + meminfo.reserved));
}

void arch_pre_mm_init(void)
{
	pm_init();
	//	exc_register(VECTOR_SYSCALL, "syscall", (iroutine) syscall);
	
}

void arch_post_mm_init(void)
{
		/* video */
		xen_console_init();
}

void arch_pre_smp_init(void)
{
		memory_print_map();
		
		#ifdef CONFIG_SMP
		acpi_init();
		#endif /* CONFIG_SMP */
}


start_kernel()
{
	xen_puts("XEN : Load Jicama OS OK\n");
	while (1)
	{
	}
}


pm_init()
{
	xen_puts("XEN : Load...\n");
}

