

#include <jicama/process.h>
#include "s3c4510.h"



void arm_icache_enable()
{
	long reg;
	volatile int i;

	/* flush cache before enable */
	u32_t *tagram = (u32_t *)S3C4510_SRAM_ADDR;

	SYSCFG &= ~(1<<1);	/* Disable cache */

	for(i=0; i < 256; i++) 
	{
		*tagram = 0x00000000;
		tagram += 1; 
	}

	/*
		Enable chache
	*/
	reg = SYSCFG;
	reg |= (1 << 1);
	SYSCFG = reg;
}

/**
 * This function will disable I-Cache of CPU
 *
 */
void arm_icache_disable()
{
	long reg;

	reg = SYSCFG;
	reg &= ~(1<<1);
	SYSCFG = reg;
}

/**
 * this function will get the status of I-Cache
 *
 */
int arm_icache_status()
{
	return 0;
}

/**
 * this function will enable D-Cache of CPU
 *
 */
void arm_dcache_enable()
{
	arm_icache_enable();
}

/**
 * this function will disable D-Cache of CPU
 *
 */
void arm_dcache_disable()
{
	arm_icache_disable();
}

/**
 * this function will get the status of D-Cache
 *
 */
int arm_dcache_status()
{
	return arm_icache_status();
}

/**
 * this function will reset CPU
 *
 */
void arm_reset()
{
}

