
#include <jicama/system.h>
#include <jicama/utsname.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <assert.h>
/* always include this file for at91 peripherals */
#include "at91.h"


#include "at91_aic.h"
#include "at91_tc.h"
#include "at91_usart.h"
#include "AT91RM9200.h"

extern void reset_cpu(unsigned long addr);

/* we always count down the max. */
#define TIMER_LOAD_VAL 0xffff

/* macro to read the 16 bit timer */
#define READ_TIMER (tmr->TC_CV)
AT91PS_TC tmr;


void enable_interrupts (void)
{
    return;
}
int disable_interrupts (void)
{
    return 0;
}


void bad_mode(void)
{
    panic("Resetting CPU ...\n");
    reset_cpu(0);
}

struct pt_regs
{
};

void show_regs(struct pt_regs * regs)
{
}

void do_undefined_instruction(struct pt_regs *pt_regs)
{
    kprintf("undefined instruction\n");
    show_regs(pt_regs);
    bad_mode();
}

void do_software_interrupt(struct pt_regs *pt_regs)
{
    kprintf("software interrupt\n");
    show_regs(pt_regs);
    bad_mode();
}

void do_prefetch_abort(struct pt_regs *pt_regs)
{
    kprintf("prefetch abort\n");
    show_regs(pt_regs);
    bad_mode();
}

void do_data_abort(struct pt_regs *pt_regs)
{
    kprintf("data abort\n");
    show_regs(pt_regs);
    bad_mode();
}

void do_not_used(struct pt_regs *pt_regs)
{
    kprintf("not used\n");
    show_regs(pt_regs);
    bad_mode();
}

void do_fiq(struct pt_regs *pt_regs)
{
    kprintf("fast interrupt request\n");
    show_regs(pt_regs);
    bad_mode();
}

void do_irq(struct pt_regs *pt_regs)
{
    kprintf("interrupt request\n");
    show_regs(pt_regs);
    bad_mode();
}

static unsigned long timestamp;
static unsigned long lastinc;

int interrupt_init (void)
{

    tmr = AT91C_BASE_TC0;

    /* enables TC1.0 clock */
    *AT91C_PMC_PCER = 1 << AT91C_ID_TC0;  /* enable clock */

    *AT91C_TCB0_BCR = 0;
    *AT91C_TCB0_BMR = AT91C_TCB_TC0XC0S_NONE | AT91C_TCB_TC1XC1S_NONE | AT91C_TCB_TC2XC2S_NONE;
    tmr->TC_CCR = AT91C_TC_CLKDIS;
    tmr->TC_CMR = AT91C_TC_TIMER_DIV1_CLOCK;  /* set to MCLK/2 */

    tmr->TC_IDR = ~0ul;
    tmr->TC_RC = TIMER_LOAD_VAL;
    lastinc = TIMER_LOAD_VAL;
    tmr->TC_CCR = AT91C_TC_SWTRG | AT91C_TC_CLKEN;
    timestamp = 0;
    return (0);
}

/*
 * timer without interrupts
 */

void reset_timer(void)
{
    reset_timer_masked();
}

unsigned long get_timer (unsigned long base)
{
    return get_timer_masked() - base;
}

void set_timer (unsigned long t)
{
    timestamp = t;
}

void udelay(unsigned long usec)
{
    udelay_masked(usec);
}

void reset_timer_masked(void)
{
    /* reset time */
    lastinc = READ_TIMER;
    timestamp = 0;
}

unsigned long get_timer_masked(void)
{
    unsigned long now = READ_TIMER;
    if (now >= lastinc)
    {
	/* normal mode */
	timestamp += now - lastinc;
    } else {
	/* we have an overflow ... */
	timestamp += now + TIMER_LOAD_VAL - lastinc;
    }
    lastinc = now;

    return timestamp;
}

void udelay_masked(unsigned long usec)
{
    unsigned long tmo;

    tmo = usec / 1000;
    tmo *= CFG_HZ;
    tmo /= 1000;

    reset_timer_masked();

    while(get_timer_masked() < tmo);
      /*NOP*/;
}
