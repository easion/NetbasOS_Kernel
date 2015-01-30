/*
    bubble.c - Sample project to demonstrate the mcore stub

    Author:  Brian LaPonsey
             Motorola, East Kilbride

    Copyright (c) Motorola, Inc., 2002
    ALL RIGHTS RESERVED

    You are hereby granted a copyright license to use, modify, and
    distribute the SOFTWARE so long as this entire notice is retained
    without alteration in any modified and/or redistributed versions,
    and that such modified versions are clearly identified as such.
    No licenses are granted by implication, estoppel or otherwise under
    any patents or trademarks of Motorola, Inc.

    The SOFTWARE is provided on an "AS IS" basis and without warranty.
    To the maximum extent permitted by applicable law, MOTOROLA DISCLAIMS
    ALL WARRANTIES WHETHER EXPRESS OR IMPLIED, INCLUDING IMPLIED
    WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
    PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH
    REGARD TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS
    THEREOF) AND ANY ACCOMPANYING WRITTEN MATERIALS.

    To the maximum extent permitted by applicable law, IN NO EVENT SHALL
    MOTOROLA BE LIABLE FOR ANY DAMAGES WHATSOEVER
    (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF
    BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS
    INFORMATION, OR OTHER PECUNIARY LOSS) ARISING OF THE USE OR
    INABILITY TO USE THE SOFTWARE.   Motorola assumes no responsibility
    for the maintenance and support of the SOFTWARE.
*/

#include <string.h>
#include "mcore.h"
#include "mmc2107.h"

#define SORTSIZE 32

void _start( void );
void init ( void );
void isr_PIT1 ( void ) __attribute__ ((naked));
void bubble ( unsigned short array[], int n );
int main ( void );

/* shift state of the LEDs */
unsigned char LED_shift; 

/* reserve stack space */
unsigned char stack[128] __attribute__ ((section(".stack")));

const unsigned short random_data[SORTSIZE] =
{
   53, 121,  76,  38,   7, 117,   0,  63,
  102,  35, 107,  48,  88, 173, 128,  85,
   20,  53,  14, 231,   0, 194, 189,  95,
   10, 107, 138, 212,  91, 153, 247, 229
};

/* _start

   Program entry point
   Initializes stack and C runtime environment.
   If main() returns, trap and return to _start.
*/
void
_start( void )
{
  /* initialize stack pointer */
  __asm__("lrw   r1, _stack      ");
  __asm__("mov   r0,  r1         ");

  /* zero the .bss data space */
  __asm__("lrw   r1, _bss_start  ");
  __asm__("lrw   r2, _bss_end    ");
  __asm__("cmphs r1, r2          ");
  __asm__("bt    lab1            ");
  __asm__("movi  r3, 0           ");

  __asm__("lab0:                 ");
  __asm__("st.b  r3, (r1,0)      ");
  __asm__("addi  r1, 1           ");
  __asm__("cmphs r1, r2          ");
  __asm__("bf    lab0            ");

  /* copy data from ROM to RAM */
  __asm__("lab1:                 ");
  __asm__("lrw   r1, _data_start ");
  __asm__("lrw   r2, _data_end   ");
  __asm__("cmphs r1, r2          ");
  __asm__("bt    lab3            ");
  __asm__("lrw   r3, _data_ROM   ");

  __asm__("lab2:                 ");
  __asm__("ld.b  r4, (r3, 0)     ");
  __asm__("st.b  r4, (r1, 0)     ");
  __asm__("addi  r3, 1           ");
  __asm__("addi  r1, 1           ");
  __asm__("cmphs r1, r2          ");
  __asm__("bf    lab2            ");
  __asm__("lab3:                 ");

  main();

  /* set a trap to stop here before returning to _start */
  __asm__("trap 0");
  __asm__("jmpi _start");

  /* asm function returns contents of vector base register */
  __asm__(".export readvbr");
  __asm__("readvbr:");
  __asm__("mfcr r2, vbr");
  __asm__("rts");
}

/* bubble

   Ye olde bubble sort
*/
void
bubble ( unsigned short array[], int n )
{
  int i, j, done;
  unsigned short tmp;

  for (i=0; i<n-1; ++i) {
    done = TRUE;
    for (j=n-1; j>i; --j) {
      if (array[j-1] > array[j]) {
        done = FALSE;
        tmp = array[j-1];
        array[j-1] = array[j];
        array[j] = tmp;
      }
    }
    if (done) {
      break;
    }
  }
}

/* init

   initialize hardware modules used in the main program code
*/
void
init ( void )
{
                   /* Initialize PIT1 module */
  rPCSR1 = 0x0F1F; /* (0b0000111100011111)
                             |||| |||||||__ EN
                             |||| ||||||___ RLD
                             |||| |||||____ PIF
                             |||| ||||_____ PIE
                             |||| |||______ OVW
                             |||| ||_______ PDBG
                             |||| |________ PDOZE
                             ||||__________ PRE  */

  /* Set PIT1 modulus value */
  rPMR1 = 0x0100;  

  /* configure edge port pins 0-3 as outputs */
  rEPDDR = 0x0F;   

  /* install vector table entry for isr_PIT1 (priority-0) */
  vector(isr_PIT1, readvbr() + ((vec_offset + 0) * 4)); 

  /* set up interrupt control module */
  rICR  = 0x0000;  /* use vectored interrupts */
  rPLSR30 = 0;     /* Choose priority 0 for PIT1 (source 30) */
  rNIER |= 1<<0;   /* enable priority-0's as fast interrupts */

  /* enable normal interrupts */
  set_psr_ie();
}


/* isr_PIT1

   Interrupt service routine for PIT1 timer interrupt
   Rotates 4-LED pattern on EPORT[0-3]
*/
void
isr_PIT1 ( void ) /* __attribute__ ((naked)) */
{
  /* ISR entry code */
  stack_quadrant();       /* save r4-r7 on stack */
  stack_shadow_reg();     /* save epc and epsr on stack */
  set_psr_ee();           /* re-enable exception shadowing */

  /* ISR main body code */ 
  rEPDR = ~(1<<LED_shift);     /* set the LED pattern */
  LED_shift = ++LED_shift % 4; /* advance the shift state */
  rPCSR1 = rPCSR1;             /* clear PIT1 flag */

  /* ISR exit code */
  unstack_shadow_reg();   /* restore epc and epsr */
  unstack_quadrant();     /* restore r4-r7 */

  __asm__("rte");         /* return from exception */
}


int 
main ( void )
{
  unsigned short sort_array[SORTSIZE];

  init();

  for (;;)
  {
    memcpy( sort_array, random_data, (SORTSIZE * sizeof(short)));
    bubble( sort_array, SORTSIZE );
  }

  return(0);
}
