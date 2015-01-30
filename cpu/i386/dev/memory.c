/* Jicama OS  
 * Copyright (C) 2001-2003  DengPingPing      All rights reserved.    
 */
#include <jicama/system.h>
#include <arch/x86/io.h>
#include <arch/x86/regs.h>
#include <jicama/devices.h>

u32_t get_bios_mem_size(unsigned long* start, unsigned long* end)
{
	unsigned int  total=0;

	// low -> high read
	unsigned int basemm = (cmos_read(0x15) | (cmos_read(0x16) << 8));  ////base memory read.
	unsigned int extmm = (cmos_read(0x17) | (cmos_read(0x18) << 8));  ////////extend memory read.

	total = (basemm + extmm)*1024L; //////////count the total memory.

	if(start)*start = basemm;
	if(end)*end = total;

	kprintf("base: %d\n", basemm);
	kprintf("ext: %d\n", extmm);
	kprintf("total: %d\n", total);
	return total;
}

