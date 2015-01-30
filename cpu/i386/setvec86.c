
#include <jicama/system.h>
#include <jicama/process.h>
#include <arch/x86/io.h>
#include <string.h>
#include <arch/x86/keyboard.h>
#include <jicama/paging.h>
#include <arch/x86/traps.h>
#include "dos_call.h"


/* Should this be here or in kernel.h? */
__local void farcall(u16_t cs, u32_t eip);
//#define setvec(n, isr)  (void)(*(VOID (FAR * FAR *)())(MK_FP(0,4 * (n))) = (isr))

u32_t rm_irq_table[256];
struct handler exc_table[32];

/*
__asmlink void asm_int20_handler (void); 
__asmlink void asm_int21_handler (void); 
__asmlink void asm_int22_handler (void); 
__asmlink void asm_int23_handler (void); 
__asmlink void asm_int24_handler (void); 
__asmlink void asm_int25_handler (void); 
__asmlink void asm_int26_handler (void); 
__asmlink void asm_int27_handler (void); 
__asmlink void asm_int28_handler (void); 
__asmlink void asm_int29_handler (void); 
__asmlink void asm_int2f_handler (void); 
__asmlink void asm_int31_handler (void); 
*/

void dos_kernel_init(void)
{
  int i;

 memcpy(rm_irq_table, 0, 1024);

  for (i = 0; i < 32; i++) {
    exc_table[i].cs = 0;
    exc_table[i].eip = 0;
  }

	//put_interrupt(0x20, (unsigned)&asm_int20_handler); /*dos call*/
	//put_interrupt(0x21, (unsigned)&asm_int21_handler); /*dos call*/
	//put_interrupt(0x22, (unsigned)&asm_int22_handler); /*dos call*/
	//put_interrupt(0x23, (unsigned)&asm_int23_handler); /*dos call*/
	//put_interrupt(0x24, (unsigned)&asm_int24_handler); /*dos call*/
	//put_interrupt(0x25, (unsigned)&asm_int25_handler); /*dos call*/
	//put_interrupt(0x26, (unsigned)&asm_int26_handler); /*dos call*/
	//put_interrupt(0x27, (unsigned)&asm_int27_handler); /*dos call*/
	//put_interrupt(0x28, (unsigned)&asm_int28_handler); /*dos call*/
	//put_interrupt(0x29, (unsigned)&asm_int29_handler); /*dos call*/
	//put_interrupt(0x2f, (unsigned)&asm_int2f_handler); /*dos call*/
	//put_interrupt(0x31, (unsigned)&asm_int31_handler); /*dmpi call*/

}



void exception_handler(int n, u32_t code, u32_t eip, u32_t cs, u32_t ef)
{
   u16_t c;// = get_TR();
  __asm __volatile__ ("strw %0" : "=q" (c));

    if (c != VM86_TASK_TSS_SEL) {
		return ;
	}

  if (exc_table[n].cs == 0) {
    //message("Unhandled exception... %d (%x)\n", n, n);
  } else {
   // fd32_log_printf("I have to call 0x%x:0x%lx\n", exc_table[n].cs, exc_table[n].eip);
    farcall(exc_table[n].cs, exc_table[n].eip);
  }
  /* For the moment, let's abort anyway... */
  panic("fd32_abort");
}



union far_call_tss
{
   unsigned char buf8[6];
   unsigned short buf16[3];
};

__local  void farcall (unsigned short tss_sel, u32_t eip)
{
	union far_call_tss xx;

    poke32(&xx.buf16[0],eip);

    xx.buf8[4]=tss_sel & 0xff;
    xx.buf8[5]=tss_sel >> 8;

  __asm ("lcall (%0)"::"g" (&xx));
}


