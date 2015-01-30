#include "gdbcpu32.h"
#include "mc68332.h"
#include "testapp.h"

#define SCI_COMPLETE_VECTOR 64
#define SCI_INTERRUPT_PRIORITY_LEVEL 6

/* these symbols are created by the linker to help us find the text, bss and
data sections, respectively; they aren't actual allocations */
extern char _data_start;
extern char _data_end;
extern const char _text_start;
extern const char _text_end;
extern char _bss_start;
extern char _bss_end;
extern char _stack_top;
extern char _stack_bottom;

void gcc_start(void);
void cstart(void);
void __main(void);
int main(void);
void FinishHardwareInit(void);
void unhandled_interrupt_handler(void);
void ISR_PREFIX(unhandled_interrupt_handler)(void);

/* the startup function initializes the stack pointer, status register and
vector base register, then calls cstart() */
asm(".align 2");
asm(".global gcc_start");
asm("gcc_start:");

  /* disable interrupts in case of warm restart */
  DISABLE_INTERRUPTS;

  /* set up vector base register */
  asm("move.l #_text_start,%d0");
  asm("movec %d0,%vbr");

  /* initialize on-board 2KB ram to be at address 0x200000 */
  asm("movea.l #0xfffb04,%a0"); /* set TRAMBAR register */
  asm("move.l #_stack_bottom,%d0"); /* specified in .ld file */
  asm("lsr.l #8,%d0");
  asm("move.w %d0,(%a0)");

  /* initialize stack pointer */
  asm("moveal #_stack_top,%sp");

  /* disable watchdog by writing 0 to bit 7 of SYPCR */
  asm("move.b #0x0,0xfffa21");

  /*  jump to C */
  asm("bra cstart");
  asm("nop");

/* sets up the C runtime environment, then calls main() */
void cstart(void)
{
  register const char *src;
  register char *dst;

  /* initialize initialized C global data */
  for (dst = &_data_start, src = &_text_end; dst < &_data_end; dst++, src++)
  {
    *dst = *src;
  }

  /* zero out uninitialized C global data */
  for (dst = &_bss_start; dst < &_bss_end; dst++)
  {
    *dst = 0;
  }

  /* call main */
  main();
}

void __main(void)
{
  /* gotta have this, because gcc wants it (related to gcc's c++ support) */
}

ISR_ASSEMBLY_WRAPPER(unhandled_interrupt_handler);
void unhandled_interrupt_handler(void)
{
  for (;;)
  {
    ;
  }
}
#define UNHANDLED_INTERRUPT_HANDLER ISR_PREFIX(unhandled_interrupt_handler)

typedef void (*PFV)(void);       /* pointer to function returning void */
const PFV vector_table[] __attribute__((section(".vect"))) =
{
  (PFV)(void *)&_stack_top,      /*   0 initial stack pointer */
  gcc_start,                     /*   1 initial program counter */
  UNHANDLED_INTERRUPT_HANDLER,   /*   2 access fault */
  UNHANDLED_INTERRUPT_HANDLER,   /*   3 address error */
  UNHANDLED_INTERRUPT_HANDLER,   /*   4 illegal instruction */
  UNHANDLED_INTERRUPT_HANDLER,   /*   5 integer divide by zero */
  UNHANDLED_INTERRUPT_HANDLER,   /*   6 CHK, CHK2 instructin */
  UNHANDLED_INTERRUPT_HANDLER,   /*   7 FTRAPcc, TRAPcc, TRAPV Instructions */
  UNHANDLED_INTERRUPT_HANDLER,   /*   8 Privelege Violation */
  gdb_interrupt_handler,         /*   9 Trace */
  UNHANDLED_INTERRUPT_HANDLER,   /*  10 Line 1010 emulator */
  UNHANDLED_INTERRUPT_HANDLER,   /*  11 Line 1111 emulator */
  UNHANDLED_INTERRUPT_HANDLER,   /*  12 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  13 coprocessor protocol violation */
  UNHANDLED_INTERRUPT_HANDLER,   /*  14 Format error */
  UNHANDLED_INTERRUPT_HANDLER,   /*  15 uninitialized interrupt */
  UNHANDLED_INTERRUPT_HANDLER,   /*  16 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  17 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  18 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  19 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  20 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  21 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  22 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  23 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  24 spurious interrupt */
  UNHANDLED_INTERRUPT_HANDLER,   /*  25 Level 1 interrupt autovector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  26 Level 2 interrupt autovector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  27 Level 3 interrupt autovector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  28 Level 4 interrupt autovector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  29 Level 5 interrupt autovector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  30 Level 6 interrupt autovector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  31 Level 7 interrupt autovector */
  gdb_interrupt_handler,         /*  32 Trap 0 instruction vector */
  gdb_interrupt_handler,         /*  33 Trap 1 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  34 Trap 2 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  35 Trap 3 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  36 Trap 4 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  37 Trap 5 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  38 Trap 6 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  39 Trap 7 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  40 Trap 8 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  41 Trap 9 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  42 Trap 10 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  43 Trap 11 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  44 Trap 12 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  45 Trap 13 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  46 Trap 14 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  47 Trap 15 instruction vector */
  UNHANDLED_INTERRUPT_HANDLER,   /*  48 FP Branch of Set on Unordered */
  UNHANDLED_INTERRUPT_HANDLER,   /*  49 FP Inexact Result */
  UNHANDLED_INTERRUPT_HANDLER,   /*  50 FP Divide by Zero */
  UNHANDLED_INTERRUPT_HANDLER,   /*  51 FP Underflow */
  UNHANDLED_INTERRUPT_HANDLER,   /*  52 FP Operand Error */
  UNHANDLED_INTERRUPT_HANDLER,   /*  53 FP Overflow */
  UNHANDLED_INTERRUPT_HANDLER,   /*  54 Signaling NaN */
  UNHANDLED_INTERRUPT_HANDLER,   /*  55 FP unimplemented data type */
  UNHANDLED_INTERRUPT_HANDLER,   /*  56 MMU configuration error */
  UNHANDLED_INTERRUPT_HANDLER,   /*  57 MMU ilegal operation error */
  UNHANDLED_INTERRUPT_HANDLER,   /*  58 MMU access level violation error */
  UNHANDLED_INTERRUPT_HANDLER,   /*  59 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  60 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  61 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  62 unassigned, reserved */
  UNHANDLED_INTERRUPT_HANDLER,   /*  63 unassigned, reserved */

  /*user defined vectors */
  gdb_interrupt_handler,         /*  64 SCI_COMPLETE_VECTOR */
  UNHANDLED_INTERRUPT_HANDLER,   /*  65 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  66 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  67 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  68 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  69 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  70 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  71 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  72 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  73 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  74 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  75 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  76 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  77 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  78 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  79 */

  /* TPU interrupt handlers below */
  UNHANDLED_INTERRUPT_HANDLER,   /*  80 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  81 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  82 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  83 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  84 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  85 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  86 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  87 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  88 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  89 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  90 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  91 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  92 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  93 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  94 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  95 */
  /* TPU interrupt handlers above */

  UNHANDLED_INTERRUPT_HANDLER,   /*  96 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  97 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  98 */
  UNHANDLED_INTERRUPT_HANDLER,   /*  99 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 100 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 101 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 102 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 103 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 104 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 105 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 106 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 107 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 108 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 109 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 110 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 111 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 112 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 113 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 114 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 115 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 116 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 117 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 118 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 119 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 120 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 121 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 122 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 123 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 124 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 125 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 126 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 127 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 128 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 129 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 130 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 131 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 132 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 133 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 134 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 135 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 136 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 137 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 138 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 139 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 140 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 141 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 142 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 143 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 144 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 145 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 146 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 147 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 148 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 149 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 150 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 151 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 152 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 153 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 154 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 155 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 156 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 157 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 158 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 159 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 160 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 161 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 162 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 163 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 164 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 165 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 166 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 167 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 168 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 169 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 170 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 171 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 172 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 173 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 174 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 175 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 176 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 177 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 178 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 179 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 180 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 181 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 182 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 183 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 184 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 185 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 186 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 187 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 188 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 189 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 190 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 191 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 192 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 193 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 194 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 195 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 196 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 197 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 198 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 199 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 200 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 201 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 202 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 203 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 204 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 205 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 206 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 207 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 208 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 209 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 210 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 211 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 212 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 213 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 214 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 215 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 216 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 217 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 218 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 219 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 220 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 221 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 222 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 223 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 224 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 225 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 226 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 227 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 228 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 229 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 230 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 231 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 232 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 233 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 234 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 235 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 236 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 237 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 238 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 239 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 240 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 241 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 242 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 243 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 244 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 245 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 246 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 247 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 248 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 249 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 250 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 251 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 252 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 253 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 254 */
  UNHANDLED_INTERRUPT_HANDLER,   /* 255 */
};

void FinishHardwareInit(void)
{
  SYNCR->w = 0;
  SYNCR->x = 1;
  SYNCR->y = 63; /* setup clock to run at 16MHz */

  QILR->ilsci = SCI_INTERRUPT_PRIORITY_LEVEL;
  QILR->qivr = SCI_COMPLETE_VECTOR;

  /* SCCR0->scbr = 55;  for 9600 baud at 16MHz */
  /* SCCR0->scbr = 14;  for 38400 baud at 16MHz */
  SCCR0->scbr = 9; /* for 57600 baud at 16MHz */
  /* SCCR0->scbr = 5;  for 115200 baud at 16MHz */

  SCCR1->re = 1; /* enable receive */
  SCCR1->te = 1; /* enable transmit */
  SCCR1->pe = 0; /* no parity */
  SCCR1->m = 0; /* 10 bit frames */

  /* on test board, rts is on port e bit 0 */
  PEPAR->pepa0 = 0; /* discrete i/o */
  /* set logic level needed to assert rts -- pc gdb needs this */
  PORTE->pe0_dsack0_ = 0;
  DDRE->dde0 = 1; /* set data direction as output */

  SIMCR->iarb = 15;
  QMCR->iarb = 12;

  /* set most port e pins to be i/o */
  PEPAR->pepa7 = 0;
  PEPAR->pepa6 = 0;
  PEPAR->pepa5 = 1; /* select addr strobe */
  PEPAR->pepa4 = 1; /* select data strobe */
  PEPAR->pepa3 = 0;
  PEPAR->pepa2 = 0;
  PEPAR->pepa1 = 0;

  /* set port f to be i/o rather than IRQ/MODCLK */
  PFPAR->pfpa7 = 0;
  PFPAR->pfpa6 = 0;
  PFPAR->pfpa5 = 0;
  PFPAR->pfpa4 = 0;
  PFPAR->pfpa3 = 0;
  PFPAR->pfpa2 = 0;
  PFPAR->pfpa1 = 0;
  PFPAR->pfpa0 = 0;
}

int main(void)
{
  FinishHardwareInit();

#ifdef TESTAPP

  SCCR1->rie = 1; /* enable receiver interrupts */
  ENABLE_ALL_ENABLED_INTERRUPTS; /* allow sci interrupt to happen now */

  gdb_cout("Hello, world!\n");

  testapp();

#else

  while (1)
  {
    /* simple startup code that just forces us
    to give control to the debugging stub */
    COMPILED_IN_BREAKPOINT; /* Causes control to be passed to GDB stub */
  }

#endif

  return 0;
}

