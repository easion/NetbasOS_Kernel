/*
  Copyright (c) 2003 by      William A. Gatliff
  All rights reserved.      bgat@billgatliff.com

  See the file COPYING for details.

  This file is provided "as-is", and without any express or implied
  warranties, including, without limitation, the implied warranties of
  merchantability and fitness for a particular purpose.

  The author welcomes feedback regarding this file.

  This stub was partially funded by Danfoss A/S.  Thanks, guys!

  **

  $Id: h8300.c,v 1.1 2003/03/26 16:20:23 bgat Exp $

   h8300-elf-gcc -g -ms -mint32 -O2 -nostartfiles -nodefaultlibs \
   -DCRT0 -DH8300_H8S2148EDK main.c gdb.c h8300.c h8300.S \
   -Wl,--script=h8300-h8s2148edk-stub.x; h8300-elf-objdump --syms \
   a.out | grep _start ; h8300-elf-objcopy -O srec a.out a.mot

*/

#include "gdb.h"

#include "h8300.h"



typedef enum
  {
    R0, R1, R2, R3, R4, R5, R6,
    SP, CCR, PC, MACH, MACL
  } register_id_E;

typedef struct
{
  long stack[128];
  long pc;
  long ccr;
  long r6;
  long r5;
  long r4;
  long r3;
  long r2;
  long r1;
  long r0;
  long sp;
} register_file_S;

register_file_S gdb_register_file;

#define H8300_STEPI_OPCODE 0x5730U /* 5720 == trapa #2, 5730 == trapa #3 */

typedef unsigned short h8300_instr_t;
char h8300_stepping;
h8300_instr_t *h8300_stepped_addr;
h8300_instr_t h8300_stepped_instr;




int gdb_peek_register_file (int id, long *val)
{
  int retval = sizeof(long);

  switch (id)
    {
    case R0: *val = gdb_register_file.r0; break;
    case R1: *val = gdb_register_file.r1; break;
    case R2: *val = gdb_register_file.r2; break;
    case R3: *val = gdb_register_file.r3; break;
    case R4: *val = gdb_register_file.r4; break;
    case R5: *val = gdb_register_file.r5; break;
    case R6: *val = gdb_register_file.r6; break;
    case SP: *val = gdb_register_file.sp; break;
    case PC: *val = gdb_register_file.pc; break;
    case CCR: *val = gdb_register_file.ccr & 0x0ffUL; break;
    case MACH: retval = 0; break;
    case MACL: retval = 0; break;
    default: retval = 0;
    }

  return retval;
}


int gdb_poke_register_file (int id, long val)
{
  int retval = sizeof(long);

  switch (id)
    {
    case R0: gdb_register_file.r0 = val; break;
    case R1: gdb_register_file.r1 = val; break;
    case R2: gdb_register_file.r2 = val; break;
    case R3: gdb_register_file.r3 = val; break;
    case R4: gdb_register_file.r4 = val; break;
    case R5: gdb_register_file.r5 = val; break;
    case R6: gdb_register_file.r6 = val; break;
    case SP: gdb_register_file.sp = val; break;
    case PC: gdb_register_file.pc = val; break;
    case CCR: gdb_register_file.ccr = val & 0x0ffUL; break;
    case MACH: break;
    case MACL: break;
    default: retval = 0;
    }

  return retval;
}


void h8300_handle_exception (int sigval)
{
  gdb_monitor_onentry();

  if (h8300_stepping)
    {
      *h8300_stepped_addr = h8300_stepped_instr;
      h8300_stepping = 0;
    }

  gdb_handle_exception(sigval);
}


short h8300_instr_len (long addr)
{
  /* H8S instructions aren't all the same length.  This table is
     instruction length vs. the first byte of its opcode, with a few
     exceptions marked by a -1 */
  static signed char const op2len[256] = {

    /* 0 */
    2, -1, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 

    /* 1 */
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 

    /* 2 */
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 

    /* 3 */
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 

    /* 4 */
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 

    /* 5 */
    2, 2, 2, 2, 2, 2, 2, 2, 
    4, 2, 4, 2, 4, 2, 4, 2, 

    /* 6 */
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, -1, -1, 2, 2, 4, 4,

    /* 7 */
    2, 2, 2, 2, 2, 2, 2, 2, 
    8, 4, 6, 4, 4, 4, 4, 4, 

    /* 8 */
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 

    /* 9 */ 
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 

    /* a */
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 

    /* b */
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 

    /* c */
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 

    /* d */
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 

    /* e */
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 

    /* f */
    2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 
  };

  unsigned char *op = (unsigned char *)addr;
  signed short len = op2len[*op];

  if (-1 == len)
    {
      if (0x6aU == *op)
	{
	  switch (*(op + 1) & 0xf0)
	    {
	    case 0x10 : len = 6; break;
	    case 0x30 : len = 8; break;
	    default: len = 2; break; /* error */
	    }
	}
      else if (0x6bU == *op)
	{
	  switch (*(op + 1) & 0xf0)
	    {
	    case 0x30 : len = 4; break;
	    case 0xa0 : len = 6; break;
	    default: len = 2; break; /* error */
	    }
	}
      else if(0x01U == *op)
	{
	  if (0x6aU == *(op + 2) || 0x6bU == *(op + 2))
	    {
	      if (0x20U & *(op + 3))
		len = 8;
	      else
		len = 6;
	    }
	  else
	    len = op2len[*(op + 2)] + 2;
	}
    }

  return len;
}




int h8300_is_branch (h8300_instr_t op)
{
  if ((0x4000U == (op & 0xf000U))
      || (0x5000U == (op & 0xf000U)))
    {
      /* any 4x or 5x opcode is a branch of some kind, except for
	 mulxu (50 and 51) and divxu (52 and 53) */
      switch (op & 0xff00)
	{
	case 0x5000U:
	case 0x5100U:
	case 0x5200U:
	case 0x5300U:
	  return 0;
	default:
	  return 1;
	}
    }
  return 0;
}

#define H8300OP_RTE(op)   (0x5670 == (op))
#define H8300OP_RTS(op)   (0x5470 == (op))
#define H8300OP_BSR8(op)  (0x5500 == ((op) & 0xff00))
#define H8300OP_BSR16(op) (0x5c00 == (op))
#define H8300OP_JMPER(op) (0x5900 == ((op) & 0xff00))
#define H8300OP_JMP24(op) (0x5a00 == ((op) & 0xff00))
#define H8300OP_JMP8(op)  (0x5b00 == ((op) & 0xff00))
#define H8300OP_JSRER(op) (0x5d00 == ((op) & 0xff00))
#define H8300OP_JSR24(op) (0x5e00 == ((op) & 0xff00))
#define H8300OP_JSR8(op)  (0x5f00 == ((op) & 0xff00))
#define H8300OP_TRAPA(op) (0x5700 == ((op) & 0xff00))

#define H8300OP_BCC16(op) (0x5800 == ((op) & 0xff00))
#define H8300OP_BCC8(op)  (0x4000 == ((op) & 0xf000))

#define H8300OP_BCC_BT 0
#define H8300OP_BCC_BF 1
#define H8300OP_BCC_BHI 2
#define H8300OP_BCC_BLS 3
#define H8300OP_BCC_BHS 4
#define H8300OP_BCC_BLO 5
#define H8300OP_BCC_BNE 6
#define H8300OP_BCC_BEQ 7
#define H8300OP_BCC_BVC 8
#define H8300OP_BCC_BVS 9
#define H8300OP_BCC_BPL 0xa
#define H8300OP_BCC_BMI 0xb
#define H8300OP_BCC_BGE 0xc
#define H8300OP_BCC_BLT 0xd
#define H8300OP_BCC_BGT 0xe
#define H8300OP_BCC_BLE 0xf

#define H8300_CCR_C(ccr)  ((ccr) & 1)
#define H8300_CCR_V(ccr)  ((ccr) & 2)
#define H8300_CCR_Z(ccr)  ((ccr) & 4)
#define H8300_CCR_N(ccr)  ((ccr) & 8)
#define H8300_CCR_U(ccr)  ((ccr) & 0x10)
#define H8300_CCR_H(ccr)  ((ccr) & 0x20)
#define H8300_CCR_UI(ccr) ((ccr) & 0x40)
#define H8300_CCR_I(ccr)  ((ccr) & 0x80)


long h8300_branch_dest (long pc)
{
  h8300_instr_t op = *(h8300_instr_t *)pc;
  signed int disp;
  long dest = pc + h8300_instr_len(pc);
  char ccr = gdb_register_file.ccr;
  short r;


  if (H8300OP_JMPER(op) || H8300OP_JSRER(op))
    {
      r = (op & 0x00f0) >> 8;
      switch (r)
	{
	case 0: dest = gdb_register_file.r0; break;
	case 1: dest = gdb_register_file.r1; break;
	case 2: dest = gdb_register_file.r2; break;
	case 3: dest = gdb_register_file.r3; break;
	case 4: dest = gdb_register_file.r4; break;
	case 5: dest = gdb_register_file.r5; break;
	case 6: dest = gdb_register_file.r6; break;
	case 7: dest = gdb_register_file.sp; break;
	default: dest = 0; break;
	}
    }

  else if (H8300OP_JMP24(op) || H8300OP_JSR24(op))
      dest = *(unsigned long*)pc & 0xffffffUL;

  else if (H8300OP_JMP8(op) || H8300OP_JSR8(op) || H8300OP_BSR8(op))
      dest = op & 0x00ff;

  else if (H8300OP_BSR16(op))
      dest = pc + *(signed short*)(pc + 2) + 2;

  else if (H8300OP_TRAPA(op))
    {
      r = (op & 0x00f0) >> 8;
      dest = ((unsigned long *)(0x20))[r];
    }

 else if (H8300OP_RTE(op) || H8300OP_RTS(op))
      dest = (*(unsigned long*)gdb_register_file.sp) & 0xffffffUL;
  
  else
    {
      if (H8300OP_BCC8(op))
	{
	  disp = (signed char)(op & 0xff);
	  r = (op & 0xf00) >> 8;
	}
      else /* if (H8300OP_BCC16(op)) */
	{
	  disp = *(signed short*)(pc + 2);
	  r = (op & 0xf0) >> 4;
	}

      switch (r)
	{
	case H8300OP_BCC_BT:
	  dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BF:
	  break;

	case H8300OP_BCC_BHI:
	  if (!(H8300_CCR_C(ccr) | H8300_CCR_Z(ccr)))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BLS:
	  if (H8300_CCR_C(ccr) | H8300_CCR_Z(ccr))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BHS:
	  if (!H8300_CCR_C(ccr))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BLO:
	  if (H8300_CCR_C(ccr))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BNE:
	  if (!H8300_CCR_Z(ccr))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BEQ:
	  if (H8300_CCR_Z(ccr))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BVC:
	  if (!H8300_CCR_V(ccr))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BVS:
	  if (H8300_CCR_V(ccr))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BPL:
	  if (!H8300_CCR_N(ccr))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BMI:
	  if (H8300_CCR_N(ccr))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BGE:
	  if (!(H8300_CCR_N(ccr) ^ H8300_CCR_V(ccr)))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BLT:
	  if (H8300_CCR_N(ccr) ^ H8300_CCR_V(ccr))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BGT:
	  if (H8300_CCR_Z(ccr) | (!(H8300_CCR_N(ccr) ^ H8300_CCR_V(ccr))))
	    dest = pc + disp + 2;
	  break;

	case H8300OP_BCC_BLE:
	  if (H8300_CCR_Z(ccr) | (H8300_CCR_N(ccr) ^ H8300_CCR_V(ccr)))
	    dest = pc + disp + 2;
	  break;
	}
    }
  return dest;
}

void gdb_step (long addr)
{
  short len;
  long dest;
  h8300_instr_t op;


  h8300_stepping = 1;

  if (addr)
    gdb_register_file.pc = addr;

  op = *(h8300_instr_t*)gdb_register_file.pc;
  len = h8300_instr_len(gdb_register_file.pc);

  if (h8300_is_branch(op))
    dest = h8300_branch_dest(gdb_register_file.pc);
  else
    dest = gdb_register_file.pc + len;
   
  h8300_stepped_addr = (h8300_instr_t *)dest;
  h8300_stepped_instr = *h8300_stepped_addr;

  *h8300_stepped_addr = H8300_STEPI_OPCODE;
  gdb_return_from_exception();
}


void gdb_continue (long addr)
{
  gdb_return_from_exception();
}


void gdb_kill (void)
{
  extern void start(void);
  start();
}


void gdb_detach (void)
{
  extern void start(void);
  start();
}


void gdb_flush_cache (void *start, int len)
{
  return;
}



#if defined(H8300_H8S2148EDK)

#define SMR SMR0
#define BRR BRR0
#define SCR SCR0
#define TDR TDR0
#define SSR SSR0
#define RDR RDR0
#define SCMR SCMR0

#define BAUD_9600 0x3c

void h8300_h8s2148edk_startup (void)
{
  /* enable, turn off USER led */
  *P8DDR |= 1;
  *P8DR |= 0;

  /* turn on serial ports */
  *MSTPCR &= ~(MSTPCR_MSTP7 | MSTPCR_MSTP6 | MSTPCR_MSTP5);
  
  /* disable transceiver, set bit rate clock source */
  *SCR = 0; /* ~(SCR_TIE | SCR_RIE | SCR_TE | SCR_RE
	       | SCR_MPIE | SCR_TEIE | SCR_CKE0 | SCR_CKE1); */

  /* set data formats */
  *SMR = 0; /* ~(SMR_CA | SMR_CHR | SMR_PE | SMR_STOP | SMR_MP); */
  *SCMR = ~(SCMR_SDIR | SCMR_SINV | SCMR_SMIF);

  /* set bit rate */
  *SMR = *SMR & ~(SMR_CKS0 | SMR_CKS1);
  *BRR = BAUD_9600;
  
  /* wait (at least) one bit interval */
  {
    volatile int loops = 10000;
    while (--loops)
      ;
  }

  /* enable transceiver */
  *SCR |= (SCR_TE | SCR_RE);


#if 1
  extern void start(void);

  if ((unsigned long)start < 0x20000UL)
    {
      /* running under HDI, set up the vector redirect table */
      extern void h8300_trapa_handler();
      ((unsigned long *)0x200000UL)[11] = (0x5a000000UL | (unsigned long)(h8300_trapa_handler));
    }
#endif
}


void gdb_monitor_onentry (void)
{
  /* turn on USER led */
  *P8DR |= 1;
}


void gdb_monitor_onexit (void)
{
  /* turn off USER led */
  *P8DR &= ~1;
}


void gdb_startup (void)
{
  h8300_stepping = 0;
  h8300_h8s2148edk_startup();
}


#endif


int gdb_putc (int c)
{
  while (!(*SSR & SSR_TDRE))
    ;

  *TDR = c;
  *SSR = (unsigned char)~SSR_TDRE;

  return c;
}


int gdb_getc (void)
{
  int c;
  unsigned char ssr;

  do {
    ssr = *SSR;
    if (ssr & (SSR_ORER | SSR_PER | SSR_FER))
	*SSR &= ~(SSR_ORER | SSR_PER | SSR_FER);
  } while (!(ssr & SSR_RDRF));
  
  c = *RDR;
  *SSR = (unsigned char)~SSR_RDRF;
  
  return c;
}
