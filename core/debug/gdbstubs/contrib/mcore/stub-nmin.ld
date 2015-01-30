ENTRY (_stub_start)

MEMORY {

/* Internal flash/RAM memory regions for the New Micros NMIN */

  VECTORS   (R)   : ORIGIN = 16K,          LENGTH = (128*4)
  FLASH     (RX)  : ORIGIN = 16K+(128*4),  LENGTH = 8K-(128*4)
  RAM       (RWX) : ORIGIN = 0x800000,     LENGTH = 1K
}

/* vectors MUST go on a 1 Kbyte boundary */
PROVIDE(vector_LMA = 0x4000);

SECTIONS {

  /* vector table */
  .vectors : AT(vector_LMA)
  {
    vectors.o(.data)
  } > RAM


  /* MMC2114 SGFM (flash) config data, stub code and constants */
  .stub_text :
  {
    sgfm_config.o(.rodata)
    *(.text)
    *(.strings)
    *(.rodata)

  } > FLASH

  _stub_data_ROM = .;

  /* gdb stub initialized data */
  .stub_data : AT(_stub_data_ROM)
  {
    . = ALIGN(0x4);
    _stub_data_start = .;
    *(.data)
    . = ALIGN(0x4);
    _stub_data_end = .;
  } > RAM


  /* stub uninitialized data */
  .stub_bss  :
  {
    . = ALIGN(0x4);
    _stub_bss_start = .;
    *(.bss)
    *(COMMON)
    . = ALIGN(0x4);
    _stub_bss_end = .;
  } > RAM


  /* gdb stub stack */
  .stub_stack :
  {
    *(.stub_stack)
    _stub_stack = .;
  } > RAM

}
