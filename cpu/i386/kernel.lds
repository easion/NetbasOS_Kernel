/*OUTPUT_FORMAT("coff-go32")*/
ENTRY(_start)

/*for Jicama Kernel Code*/
SECTIONS
{
    .text 0x100000 :	/* 1 meg */
    {
	code = .; _code = .; __code = .;
	*(.text)
	. = ALIGN(4096);
    }
    
     .data :
    {
	data = .; _data = .; __data = .;
	*(.data)
	. = ALIGN(4096);
    }

    .bss :
    {
	bss = .; _bss = .; __bss = .;
	*(.bss)
	*(COMMON) /* GNU C "common" variables */
	. = ALIGN(4096);
    }
    end = .; _end = .; __end = .;
}