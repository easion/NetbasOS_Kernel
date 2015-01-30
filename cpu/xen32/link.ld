/** Xen32 linker script

#define __ASM__
#include <arch/boot/boot.h>
#include <arch/mm/page.h>
 */

ENTRY(_start)

SECTIONS {
	__xen_guest : {
		*(__xen_guest);
	}
	
	.image 0x80000000: { 
		ktext_start = .;
		*(K_TEXT_START);
		*(.text);
		ktext_end = .;
		
		kdata_start = .;
		*(.data);			/* initialized data */
		*(.rodata*);			/* string literals */
		*(COMMON);			/* global variables */
		hardcoded_load_address = .;
		LONG(0x80000000);
		hardcoded_ktext_size = .;
		LONG(ktext_end - ktext_start);
		hardcoded_kdata_size = .;
		LONG(kdata_end - kdata_start);
		symbol_table = .;
		*(symtab.*);            	/* Symbol table, must be LAST symbol! */
		*(.bss);			/* uninitialized static variables */
		kdata_end = .;
	}

	/DISCARD/ : {
		*(.note.GNU-stack);		
		*(.comment);
	}
}
