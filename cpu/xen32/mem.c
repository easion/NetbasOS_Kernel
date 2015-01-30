#include "boot.h"
#include "memstr.h"
#include "hypercall.h"
#include "page.h"
#include "xconsole.h"

uintptr_t stack_safe;

size_t get_memory_size(void) 
{
	return start_info.frames * PAGE_SIZE;
}

void memory_print_map(void)
{
	//printf("Xen memory: %p size: %d (reserved %d)\n", PFN2ADDR(meminfo.start), PFN2ADDR(meminfo.size - meminfo.reserved), PFN2ADDR(meminfo.reserved));
}

