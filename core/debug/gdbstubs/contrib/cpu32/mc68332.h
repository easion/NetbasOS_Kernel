#ifndef MC68332_H
#define MC68332_H

/* ------------------------------------------------------------------------- */

#define CPU_REG_BASE_ADDR 0xfff000       /* registers at 0xfff000 - 0xffffff */

/* ------------------------------------------------------------------------- */

#define SIM_REG_BASE_ADDR      (CPU_REG_BASE_ADDR | 0x000a00)

/* Module Configuration Register */
typedef struct
{
  volatile unsigned short exoff : 1;
  volatile unsigned short frzsw : 1;
  volatile unsigned short frzbm : 1;
           unsigned short       : 1;
  volatile unsigned short slven : 1;
           unsigned short       : 1;
  volatile unsigned short shen  : 2;
  volatile unsigned short supv  : 1;
  volatile unsigned short mm    : 1;
           unsigned short       : 2;
  volatile unsigned short iarb  : 4;
} *pSIMCRfields;
#define SIMCR ((pSIMCRfields)(SIM_REG_BASE_ADDR | 0x000000))

/* Synthesizer Control Register */
typedef struct
{
  volatile unsigned short w     : 1;
  volatile unsigned short x     : 1;
  volatile unsigned short y     : 6;
  volatile unsigned short ediv  : 1;
           unsigned short       : 2;
  volatile unsigned short slimp : 1;
  volatile unsigned short slock : 1;
  volatile unsigned short rsten : 1;
  volatile unsigned short stsim : 1;
  volatile unsigned short stext : 1;
} *pSYNCRfields;
#define SYNCR ((pSYNCRfields)(SIM_REG_BASE_ADDR | 0x000004))

/* PORTE pin assignment register */
typedef struct
{
           unsigned short       : 8;
  volatile unsigned short pepa7 : 1;
  volatile unsigned short pepa6 : 1;
  volatile unsigned short pepa5 : 1;
  volatile unsigned short pepa4 : 1;
  volatile unsigned short pepa3 : 1;
  volatile unsigned short pepa2 : 1;
  volatile unsigned short pepa1 : 1;
  volatile unsigned short pepa0 : 1;
} *pPEPARfields;
#define PEPAR ((pPEPARfields)(SIM_REG_BASE_ADDR | 0x000016))

/* PORTF pin assignment register */
typedef struct
{
           unsigned short       : 8;
  volatile unsigned short pfpa7 : 1;
  volatile unsigned short pfpa6 : 1;
  volatile unsigned short pfpa5 : 1;
  volatile unsigned short pfpa4 : 1;
  volatile unsigned short pfpa3 : 1;
  volatile unsigned short pfpa2 : 1;
  volatile unsigned short pfpa1 : 1;
  volatile unsigned short pfpa0 : 1;
} *pPFPARfields;
#define PFPAR ((pPFPARfields)(SIM_REG_BASE_ADDR | 0x00001e))

typedef struct
{
           unsigned short      : 8;
  volatile unsigned short dde7 : 1;
  volatile unsigned short dde6 : 1;
  volatile unsigned short dde5 : 1;
  volatile unsigned short dde4 : 1;
  volatile unsigned short dde3 : 1;
  volatile unsigned short dde2 : 1;
  volatile unsigned short dde1 : 1;
  volatile unsigned short dde0 : 1;
} *pDDREfields;
#define DDRE ((pDDREfields)(SIM_REG_BASE_ADDR | 0x000014))

typedef struct
{
           unsigned short             : 8;
  volatile unsigned short pe7_siz1    : 1;
  volatile unsigned short pe6_siz0    : 1;
  volatile unsigned short pe5_as_     : 1; /* note: databooks have PE4 & PE5 */
  volatile unsigned short pe4_ds_     : 1; /*  nomenclature swapped in places! */
  volatile unsigned short pe3_rmc_    : 1;
  volatile unsigned short pe2_avec_   : 1;
  volatile unsigned short pe1_dsack1_ : 1;
  volatile unsigned short pe0_dsack0_ : 1;
} *pPORTEfields;
#define PORTE ((pPORTEfields)(SIM_REG_BASE_ADDR | 0x000010))

/* ------------------------------------------------------------------------- */

#define QSM_REG_BASE_ADDR      (CPU_REG_BASE_ADDR | 0x000c00)

/* QSM Configuration Register */
typedef struct
{
   volatile unsigned short stop : 1;
   volatile unsigned short frz1 : 1;
   volatile unsigned short frz0 : 1;
            unsigned short      : 5;
   volatile unsigned short supv : 1;
            unsigned short      : 3;
   volatile unsigned short iarb : 4;
} *pQMCRfields;
#define QMCR ((pQMCRfields)(QSM_REG_BASE_ADDR | 0x000000))

/* SCI Control Register 0 */
typedef struct
{
            unsigned short      :  3;
   volatile unsigned short scbr : 13;
} *pSCCR0fields;
#define SCCR0 ((pSCCR0fields)(QSM_REG_BASE_ADDR | 0x000008))

#define SCDR_RDR (*(volatile unsigned short *)(QSM_REG_BASE_ADDR | 0x00000e))
#define SCDR_TDR (*(volatile unsigned short *)(QSM_REG_BASE_ADDR | 0x00000e))

/* SCI Status Register */
typedef struct
{
           unsigned short      : 7;
  volatile unsigned short tdre : 1;
  volatile unsigned short tc   : 1;
  volatile unsigned short rdrf : 1;
  volatile unsigned short raf  : 1;
  volatile unsigned short idle : 1;
  volatile unsigned short or   : 1;
  volatile unsigned short nf   : 1;
  volatile unsigned short fe   : 1;
  volatile unsigned short pf   : 1;
} *pSCSRfields;
#define SCSR ((pSCSRfields)(QSM_REG_BASE_ADDR | 0x00000c))

/* SCI Control Register 1 */
typedef struct
{
            unsigned short       : 1;
   volatile unsigned short loops : 1;
   volatile unsigned short woms  : 1;
   volatile unsigned short ilt   : 1;
   volatile unsigned short pt    : 1;
   volatile unsigned short pe    : 1;
   volatile unsigned short m     : 1;
   volatile unsigned short wake  : 1;
   volatile unsigned short tie   : 1;
   volatile unsigned short tcie  : 1;
   volatile unsigned short rie   : 1;
   volatile unsigned short ilie  : 1;
   volatile unsigned short te    : 1;
   volatile unsigned short re    : 1;
   volatile unsigned short rwu   : 1;
   volatile unsigned short sbk   : 1;
} *pSCCR1fields;
#define SCCR1 ((pSCCR1fields)(QSM_REG_BASE_ADDR | 0x00000a))

/* QSM Interrupt Level Register */
typedef struct
{
            unsigned short        : 2;
   volatile unsigned short ilqspi : 3;
   volatile unsigned short ilsci  : 3;
   volatile unsigned short qivr   : 8;
} *pQILRfields;
#define QILR ((pQILRfields)(QSM_REG_BASE_ADDR | 0x000004))

/* ------------------------------------------------------------------------- */

/*
Since gcc doesn't directly support interrupt service routines (it won't push/
 pop registers and return with an rte instruction), we have to give it some
 help by providing an assembly language wrapper function, defined with the
 ISR_ASSEMBLY_WRAPPER macro.  This macro, which takes the C function name for
 the isr as an argument, creates an assembly language function of the same
 name as the C function except for the addition of a leading underscore.
 The assembly language version gets put in the vector table and runs when
 interrupt processing is needed.  It does the needed register pushes, calls
 the C function for the usual isr functionality, then does the register pops
 and returns with the required rte instruction.

 Here's an example usage:
  ISR_ASSEMBLY_WRAPPER(sci_interrupt_handler);

 And here's an example of the macro expansion (the resulting assembly code):
   .global sci_interrupt_handler        ; so jsr can "see" the C function call
   .global _sci_interrupt_handler       ; assembly language "function prototype"
 _sci_interrupt_handler:                ; label that gets put in vector table
   movem.l %d0-%d7/%a0-%a6,-(%a7)       ; push registers on stack
   jsr sci_interrupt_handler            ; call the C function (the real isr)
   movem.l (%sp)+,%d0-%d7/%a0-%a6       ; pop registers from stack
   rte                                  ; return from exception instruction
*/

/* helper macros: */
#define CONCATX(a,b) a ## b
#define CONCAT(a,b) CONCATX(a,b)
#define QUOTE_LITERAL(q) #q
#define QUOTE_DEREFERENCE(q) QUOTE_LITERAL(q)

#define REAL_ISR_PREFIX _
#define ISR_PREFIX(isr) CONCAT(REAL_ISR_PREFIX, isr)
#define QUOTED_REAL_ISR_PREFIX QUOTE_DEREFERENCE(REAL_ISR_PREFIX)

#define ISR_ASSEMBLY_WRAPPER(isr) \
  asm( \
  " .global " QUOTE_LITERAL(isr) "\n" \
  " .global _" QUOTE_LITERAL(isr) "\n" \
  QUOTED_REAL_ISR_PREFIX QUOTE_LITERAL(isr) ":\n" \
  " movem.l %d0-%d7/%a0-%a6,-(%a7)\n" \
  " jsr " QUOTE_LITERAL(isr) "\n" \
  " movem.l (%sp)+,%d0-%d7/%a0-%a6\n" \
  " rte" \
  );

/* ------------------------------------------------------------------------- */

#define ENABLE_ALL_ENABLED_INTERRUPTS asm(" andi.w #0xf8ff,%sr")
#define DISABLE_INTERRUPTS asm(" ori.w #0x0700,%sr")

/* ------------------------------------------------------------------------- */

#endif /* MC68332_H */

