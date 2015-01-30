/*
  Copyright (c) 1999 by      William A. Gatliff
  All rights reserved.     bgat@open-widgets.com

  See the file COPYING for details.

  This file is provided "as-is", and without any express
  or implied warranties, including, without limitation,
  the implied warranties of merchantability and fitness
  for a particular purpose.

  The author welcomes feedback regarding this file.

  SH-4 support added by Benoit Miller (fulg@iname.com).

  Support for Sega Dreamcast(tm) added by Benoit Miller.
  "SEGA" and "DREAMCAST" are registered trademarks of Sega
  Enterprises, Ltd.
*/

/* Based on work originally done by the SH-Linux team */

OUTPUT_FORMAT("elf32-shl", "elf32-shl", "elf32-shl")

OUTPUT_ARCH(sh)

MEMORY
{
   RAM (rw):     ORIGIN = 0x8ce0b800, LENGTH = 0x0800
   VECTABLE(rw): ORIGIN = 0x8ce0c000, LENGTH = 0x0700
   STACK (rw):   ORIGIN = 0x8ce0d800, LENGTH = 0x0640
   ROM (rx):     ORIGIN = 0x8ce0e000, LENGTH = 0x1e00
   EXCEPT(rw):   ORIGIN = 0x8ce0fe00, LENGTH = 0x0108
}


SECTIONS
{
  .text :
  {
    /*entry.o(.text)*/
    *(.text)
    *(.rodata)
  } > ROM
  __text_end = .;
  __data_start = .;
  .data :
  {
    *(.data)
  } > RAM
  . = ALIGN(4);
  __data_end = .;
  __bss_start = .;      /* BSS */
  .bss :
  {
    *(.bss)
    *(COMMON)
  } > RAM
  . = ALIGN(4);
  __bss_end = .;
  .stack   :
  {
    *(.stack)
  } > STACK
  __stack_end = .;
  .vect :
  {
    *(.vect)
  } > VECTABLE
  .except :
  {
    *(.except)
  } > EXCEPT
  .stab :
  {
    *(.stab)
  }
  .comment :
  {
    *(.comment)
  }
  .stabstr :
  {
    *(.stabstr)
  }
}
