
#include <jicama/system.h>
#include <jicama/process.h>
#include <arch/x86/io.h>
#include <string.h>
#include <arch/x86/keyboard.h>
#include <jicama/paging.h>
#include <arch/x86/traps.h>


int int22_handler(regs_t reg)
{
	panic("%s called!", __FUNCTION__);
	return 0;
}

int int23_handler(regs_t reg)
{
	panic("%s called!", __FUNCTION__);
	return 0;
}

int int24_handler(regs_t reg)
{
	panic("%s called!", __FUNCTION__);
	return 0;
}

int int25_handler(regs_t reg)
{
	panic("%s called!", __FUNCTION__);
	return 0;
}

int int26_handler(regs_t reg)
{
	panic("%s called!", __FUNCTION__);
	return 0;
}

int int27_handler(regs_t reg)
{
	panic("%s called!", __FUNCTION__);
	return 0;
}

int int28_handler(regs_t reg)
{
	panic("%s called!", __FUNCTION__);
	return 0;
}

int int29_handler(regs_t reg)
{
	panic("%s called!", __FUNCTION__);
	return 0;
}

int int2f_handler(regs_t reg)
{
	panic("%s called!", __FUNCTION__);
	return 0;
}

int int20_handler(regs_t reg)
{
}

int int21_handler(regs_t reg)
{
}

void dmpi_handler()
{
}

