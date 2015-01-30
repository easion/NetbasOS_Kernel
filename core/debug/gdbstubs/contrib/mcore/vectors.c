#include "mcore.h"

void (* vec_tab[64])(void) =
{
  &_stub_start,          /*  0 0x000  Reset vector         */
  &gdb_catch_exception,  /*  1 0x004  Misaligned Access    */
  &gdb_catch_exception,  /*  2 0x008  Access Error         */
  &gdb_catch_exception,  /*  3 0x00C  Divide by Zero       */
  &gdb_catch_exception,  /*  4 0x010  Illegal Instruction  */
  &gdb_catch_exception,  /*  5 0x014  Privilege Violation  */
  &gdb_catch_exception,  /*  6 0x018  Trace Exception      */
  &gdb_catch_exception,  /*  7 0x01C  Breakpoint Exception */
  &gdb_catch_exception,  /*  8 0x020  Unrecoverable Error  */
  &_stub_start,          /*  9 0x024  Soft Reset           */
  &gdb_catch_exception,  /* 10 0x028  Normal Autovector    */
  &gdb_catch_exception,  /* 11 0x02C  Fast Autovector      */
  &gdb_catch_exception,  /* 12 0x030  Hardware Accelerator */
  NULL,                  /* 13 0x034  Unused space         */
  NULL,                  /* 14 0x038  Unused space         */
  NULL,                  /* 15 0x03C  Unused space         */
  &gdb_catch_exception,  /* 16 0x040  trap 0               */
  &gdb_catch_exception,  /* 17 0x044  trap 1               */
  &gdb_catch_exception,  /* 18 0x048  trap 2               */
  &gdb_catch_exception,  /* 19 0x04C  trap 3               */
  NULL,                  /* 20 0x050  Unused space         */
  NULL,                  /* 21 0x054  Unused space         */
  NULL,                  /* 22 0x058  Unused space         */
  NULL,                  /* 23 0x05C  Unused space         */
  NULL,                  /* 24 0x060  Unused space         */
  NULL,                  /* 25 0x064  Unused space         */
  NULL,                  /* 26 0x068  Unused space         */
  NULL,                  /* 27 0x06C  Unused space         */
  NULL,                  /* 28 0x070  Unused space         */
  NULL,                  /* 29 0x074  Unused space         */
  NULL,                  /* 30 0x078  Unused space         */
  NULL,                  /* 31 0x07C  Unused space         */
  &gdb_catch_exception,  /* 32 0x080  Pri 0 vectored int   */
  &gdb_catch_exception,  /* 33 0x084  Pri 1 vectored int   */
  &gdb_catch_exception,  /* 34 0x088  Pri 2 vectored int   */
  &gdb_catch_exception,  /* 35 0x08C  Pri 3 vectored int   */
  &gdb_catch_exception,  /* 36 0x090  Pri 4 vectored int   */
  &gdb_catch_exception,  /* 37 0x094  Pri 5 vectored int   */
  &gdb_catch_exception,  /* 38 0x098  Pri 6 vectored int   */
  &gdb_catch_exception,  /* 39 0x09C  Pri 7 vectored int   */
  &gdb_catch_exception,  /* 40 0x0A0  Pri 8 vectored int   */
  &gdb_catch_exception,  /* 41 0x0A4  Pri 9 vectored int   */
  &gdb_catch_exception,  /* 42 0x0A8  Pri 10 vectored int  */
  &gdb_catch_exception,  /* 43 0x0AC  Pri 11 vectored int  */
  &gdb_catch_exception,  /* 44 0x0B0  Pri 12 vectored int  */
  &gdb_catch_exception,  /* 45 0x0B4  Pri 13 vectored int  */
  &gdb_catch_exception,  /* 46 0x0B8  Pri 14 vectored int  */
  &gdb_catch_exception,  /* 47 0x0BC  Pri 15 vectored int  */
  &gdb_catch_exception,  /* 48 0x0C0  Pri 16 vectored int  */
  &gdb_catch_exception,  /* 49 0x0C4  Pri 17 vectored int  */
  &gdb_catch_exception,  /* 50 0x0C8  Pri 18 vectored int  */
  &gdb_catch_exception,  /* 51 0x0CC  Pri 19 vectored int  */
  &gdb_catch_exception,  /* 52 0x0D0  Pri 20 vectored int  */
  &gdb_catch_exception,  /* 53 0x0D4  Pri 21 vectored int  */
  &gdb_catch_exception,  /* 54 0x0D8  Pri 22 vectored int  */
  &gdb_catch_exception,  /* 55 0x0DC  Pri 23 vectored int  */
  &gdb_catch_exception,  /* 56 0x0E0  Pri 24 vectored int  */
  &gdb_catch_exception,  /* 57 0x0E4  Pri 25 vectored int  */
  &gdb_catch_exception,  /* 58 0x0E8  Pri 26 vectored int  */
  &gdb_catch_exception,  /* 59 0x0EC  Pri 27 vectored int  */
  &gdb_catch_exception,  /* 60 0x0F0  Pri 28 vectored int  */
  &gdb_catch_exception,  /* 61 0x0F4  Pri 29 vectored int  */
  &gdb_catch_exception,  /* 62 0x0F8  Pri 30 vectored int  */
  &isr_RDRF1+1           /* 63 0x0FC  Pri 31 vectored int  */
};
