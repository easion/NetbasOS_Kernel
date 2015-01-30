ENTRY (_start)

MEMORY {

  STUB_RAM (RW)  : ORIGIN = 0x800000,      LENGTH = 1K
  USER_RAM (RWX) : ORIGIN = 0x800000 + 1K, LENGTH = 8K-1K
}


SECTIONS {

  /* code and constants */
  .text :
  {
    _text_start = .;
    *(.text)
    *(.strings)
    *(.rodata)
    _text_end = .;
    . = ALIGN(0x8);
    _data_ROM = .;
  } > USER_RAM


  /* uninitialized data */
  .bss  :
  {
    . = ALIGN(0x4);
    _bss_start = .;
    *(.bss)
    *(COMMON)
    . = ALIGN(0x4);
    _bss_end = .;
  } > USER_RAM


  /* initialized data */
  .data : AT(_data_ROM)
  {
    . = ALIGN(0x4);
    _data_start = .;
    *(.data)
    . = ALIGN(0x4);
    _data_end = .;

  /* This app resides completely in RAM.
     Setting _data_start = _data_end
     prevents any ROM->RAM data copying */

    _data_start = _data_end;

  } > USER_RAM


  /* stack */
  .stack :
  {
    *(.stack)
    _stack = .;
  } > USER_RAM

}
