#include "boot.h"
#include "memstr.h"
#include "hypercall.h"
#include "page.h"
#include "xconsole.h"
#define IDT_ITEMS 64
#define VECTOR_SYSCALL 0x80


static trap_info_t traps[IDT_ITEMS + 1];
static void trap(void)
{
}

void traps_init(void)
{
	int i;
	
	for (i = 0; i < IDT_ITEMS; i++) {
		traps[i].vector = i;
		
		if (i == VECTOR_SYSCALL)
			traps[i].flags = 3;
		else
			traps[i].flags = 0;
		
		traps[i].cs = XEN_CS;
		traps[i].address = trap;
		//exc_register(i, "undef", (iroutine) null_interrupt);
	}
	traps[IDT_ITEMS].vector = 0;
	traps[IDT_ITEMS].flags = 0;
	traps[IDT_ITEMS].cs = 0;
	traps[IDT_ITEMS].address = NULL;
	
	//exc_register(13, "gp_fault", (iroutine) gp_fault);
	//exc_register( 7, "nm_fault", (iroutine) nm_fault);
	//exc_register(12, "ss_fault", (iroutine) ss_fault);
	//exc_register(19, "simd_fp", (iroutine) simd_fp_exception);
	xen_set_trap_table(traps);
}



