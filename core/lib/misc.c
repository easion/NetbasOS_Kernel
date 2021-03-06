
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------

#include <stdarg.h>
#include <jicama/system.h>

 void move_csr(unsigned x, unsigned y)
{
	kprintf("\x1B[%u;%uH", y, x);
}
/*****************************************************************************
*****************************************************************************/
 void set_fore_color(unsigned color)
{
	if(color >= 8)
		kprintf("\x1B[%u;1m", 30 + (color & 7));
	else
		kprintf("\x1B[%u;0m", 30 + (color & 7));
}
/*****************************************************************************
*****************************************************************************/
 void set_back_color(unsigned color)
{
	kprintf("\x1B[%um", 40 + (color & 7));
}
/*****************************************************************************
*****************************************************************************/
 void clear_screen(void)
{
	kprintf("\x1B[2J");
}


unsigned g_seed;
/*****************************************************************************
*****************************************************************************/
unsigned rand(void)
{
	if(g_seed == 0)
		g_seed = 1;
	if((((g_seed << 3) ^ g_seed) & 0x80000000L) != 0)
		g_seed = (g_seed << 1) | 1;
	else
		g_seed <<= 1;
	return g_seed - 1;
}

void srand(unsigned new_seed)
{
	g_seed = new_seed;
}

