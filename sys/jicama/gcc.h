#ifndef GCC_H
#define GCC_H



/* Initialization and other run-once code goes in ".dtext" section;
similar use-once data goes in ".ddata section." With paging, we can
release the memory used by these items after initialization.

Linux names the discardable code and data sections ".text.init"
and ".data.init". These names are too long (>8 chars) for DJGPP
COFF format.

Note that the extra ".dtext" and ".ddata" sections will appear
only in the object (.o) files. The kernel linker script combines
them into the regular ".text" and ".data" sections (but starts
them on page boundaries, so their memory can be freed). */
#define	INIT_CODE(X)				\
	X __attribute__((section (".dtext")));		\
	X

#define	INIT_DATA(X)				\
	extern X __attribute__((section (".ddata")));	\
	X
//#define __EXPORT_SYMBOL(str,sym)\
//const struct _export_table_entry __ksymbol_##sym __attribute__((section(".kmod"))) =\
//{str,(int)(&sym)};

//#define EXPORT_SYMBOL(sym) __EXPORT_SYMBOL(#sym,sym)




#endif
