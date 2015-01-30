#include "boot.h"
#include "memstr.h"
#include "hypercall.h"
#include "page.h"
#include "xconsole.h"


#define MASK_INDEX(index, ring) ((index) & (sizeof(ring) - 1))

static bool asynchronous = false;
static void xen_putchar( const char ch);

void xen_console_init(void)
{
	if (!(start_info.flags & SIF_INITDOMAIN))
		asynchronous = true;
}

void xen_putchar(const char ch)
{
	if (asynchronous) {
		uint32_t cons = console_page.out_cons;
		uint32_t prod = console_page.out_prod;
		
		//memory_barrier();
		
		if ((prod - cons) > sizeof(console_page.out))
			return;
		
		if (ch == '\n')
			console_page.out[MASK_INDEX(prod++, console_page.out)] = '\r';
		console_page.out[MASK_INDEX(prod++, console_page.out)] = ch;
		
		//write_barrier();
		
		console_page.out_prod = prod;
		
		xen_notify_remote(start_info.console_evtchn);
	} else
		xen_console_io(CONSOLE_IO_WRITE, 1, &ch);
}

/** @}
 */

void xen_puts(const char *str)
{
	while (*str)
	{
		xen_putchar(*str++);
	}
}
