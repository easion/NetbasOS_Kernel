OUTPUT_FORMAT(coff-m68k)

GROUP(-lc -lm)   /* libdbug.a libc.a libm.a */

ENTRY(gcc_start) /* sole function is to make sure S-record is proper */

MEMORY
{
  /* note:  sys_ram excludes stack! */
  sys_ram (W) : ORIGIN = 0x30000 + 8K, LENGTH = 8K

  /* make ORIGIN 0x400000 to run from emulation ram */
  /* make ORIGIN 0x000000 to run from flash */
  flash (RX) : ORIGIN = 0x30000, LENGTH = 8K
}

_stack_top = 0xA0000;
__stack    = 0xA0000;

SECTIONS
{
  .stk :
  {
    _stack_bottom = .;
  } > sys_stack

  /* code, constants, strings */
  .text :
  {
    _text_start = .;
    *(.vect)
    *(.text)
    . = ALIGN(0x4);
    _text_end = .;
  } > flash

  /*
  initialized data section; linker will load this at __text_end in the
  output file, but will actually locate it in ram.  by doing a
  memcpy(&__data_start, &__text_end, (&__data_end - &__data_start);
  during startup, we initialize our nonzero globals.
  */
  .data : AT (_text_end)
  {
    _data_start = .;
    *(.data);    /* any variables explicitly initialized to non-zero */
    _data_end = .;
  } > sys_ram

  /* uninitialized data */
  .bss :
  {
    . = ALIGN(0x4);
    _bss_start = .;
    *(.bss)    /* any variables explicitly initialized to zero */
    *(COMMON)    /* any variables not explicitly initialized to a value */
    _bss_end = .;
  } > sys_ram
}

