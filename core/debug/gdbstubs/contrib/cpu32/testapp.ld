OUTPUT_FORMAT(coff-m68k)

GROUP(-ldbug -lc -lm)   /* libdbug.a libc.a libm.a */

ENTRY(gcc_start) /* sole function is to make sure S-record is proper */

MEMORY
{
  sys_stack (W) : ORIGIN = 0x200000, LENGTH = 1K

  /* note:  sys_ram excludes stack! */
  sys_ram (W) : ORIGIN = 0x200400, LENGTH = 1K

  flash (RX) : ORIGIN = 0x402000, LENGTH = 8K
}

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
    _stack_top = .;
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
