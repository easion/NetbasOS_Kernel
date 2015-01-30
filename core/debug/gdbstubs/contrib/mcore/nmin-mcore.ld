/* linker script to configure the sample project to run standalone
   in single-chip mode on the New Micros NMIN-21xx */

ENTRY (_stub_start)

MEMORY {

  VECTORS (R)   : ORIGIN = 16K,           LENGTH = (128*4)
  FLASH   (RX)  : ORIGIN = 16K + (128*4), LENGTH = 128K-(128*4)
  RAM     (RWX) : ORIGIN = 0x800000,      LENGTH = 8K
}

/* vectors MUST go on a 1 Kbyte boundary */
PROVIDE(vector_LMA = 0x4000);

SECTIONS {

  /* vector table */
  .vectors : AT(vector_LMA)
  {
    vectors.o(.data)
  } > RAM


  /* MMC2114 flash config data, stub code and constants */
  .text :
  {
    sgfm_config.o(.rodata)
    mcore.o(.text)
    gdb.o(.text)
    mcore.o(.strings)
    gdb.o(.strings)
    mcore.o(.rodata)
    gdb.o(.rodata)
  } > FLASH

  _stub_data_ROM = .;
  _data_ROM = .;

  /* initialized data and text to be relocated by stub */
  .data : AT(_stub_data_ROM)
  {
    _stub_data_start = .;
    _data_start = .;
    *(.text)
    *(.strings)
    *(.rodata)
    *(.data)
    _stub_data_end = .;
    _data_end = .;
  } > RAM


  /* stub uninitialized data */
  .stub_bss  :
  {
    . = ALIGN(0x4);
    _stub_bss_start = .;
    *(.bss)
    *(COMMON)
    _stub_bss_end = .;
  } > RAM


  /* stub stack */
  .stub_stack :
  {
    *(.stub_stack)
    _stub_stack = .;
  } > RAM

  /* application uninitialized data */
  .bss  :
  {
    . = ALIGN(0x4);
    _bss_start = .;
    *(.bss)
    *(COMMON)
    _bss_end = .;
  } > RAM


  /* application stack */
  .stack :
  {
    *(.stack)
    _stack = .;
  } > RAM
}
