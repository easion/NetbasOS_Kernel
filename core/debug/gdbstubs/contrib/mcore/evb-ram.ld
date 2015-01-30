ENTRY (_stub_start)

MEMORY {

  /* CMB system software uses 48K of external RAM. */
  /* Resident stub *might* use 1K of external RAM, depending on */
  /* where it was installed (in internal or external flash) */

  VECTORS  (RW)  : ORIGIN = 0x81000000 + 49K,           LENGTH = (128*4)
  STUB_RAM (RWX) : ORIGIN = 0x81000000 + 49K + (128*4), LENGTH = 8K - (128*4)
  USER_RAM (RWX) : ORIGIN = 0x81000000 + 49K + 8K,      LENGTH = 1M - 49K - 8K
}

/* vectors MUST go on a 1 Kbyte boundary */
PROVIDE(vector_LMA = 0x8100C400); /* 0x81000000 + 49K */

SECTIONS {

  /* vector table */
  .vectors :
  {
    vectors.o(*.data)
  } > VECTORS
  

  /* flash config data block, stub code and constants */
  .stub_text :
  {
    sgfm_config.o(.rodata)
    mcore.o(.text)
    gdb.o(.text)
    mcore.o(.strings)
    gdb.o(.strings)
    mcore.o(.rodata)
    gdb.o(.rodata)
  } > STUB_RAM
    
  _stub_data_ROM = .;

  /* gdb stub initialized data */
  .stub_data : AT(_stub_data_ROM)
  {
    . = ALIGN(0x4);
    _stub_data_start = .;
    mcore.o(.data)
    gdb.o(.data)
    . = ALIGN(0x4);
    _stub_data_end = .;
  } > STUB_RAM


  /* stub uninitialized data */
  .stub_bss  :
  {
    . = ALIGN(0x4);
    _stub_bss_start = .;
    mcore.o(.bss)
    gdb.o(.bss)
    mcore.o(COMMON)
    gdb.o(COMMON)
    . = ALIGN(0x4);
    _stub_bss_end = .;
  } > STUB_RAM


  /* gdb stub stack */
  .stub_stack :
  {
    mcore.o(.stub_stack)
    _stub_stack = .;
  } > STUB_RAM


  /* application code and constants */
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


  /* application initialized data */
  .data : AT(_data_ROM)
  {
    _data_start = .;
    *(.data)
    _data_end = .;

  /* This app resides completely in RAM.
     Setting _data_start = _data_end
     prevents any ROM->RAM data copying */

    _data_start = _data_end;

  } > USER_RAM


  /* application uninitialized data */
  .bss  :
  {
    . = ALIGN(0x4);
    _bss_start = .;
    *(.bss)
    *(COMMON)
    . = ALIGN(0x4);
    _bss_end = .;
  } > USER_RAM


  /* application stack */
  .stack :
  {
    *(.stack)
    _stack = .;
  } > USER_RAM

}
