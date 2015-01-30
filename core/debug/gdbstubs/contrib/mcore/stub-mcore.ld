ENTRY (_stub_start)

MEMORY {

/* Internal flash/RAM memory regions for the MMC2107/MMC2114 */

  VECTORS   (R)   : ORIGIN = 0,        LENGTH = (128*4)
  FLASH     (RX)  : ORIGIN = (128*4),  LENGTH = 8K-(128*4) 
  RAM       (RWX) : ORIGIN = 0x800000, LENGTH = 1K
}

PROVIDE(vector_LMA = 0x0000);

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
