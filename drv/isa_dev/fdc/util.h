/*
 * util.h
 * 
 * header for IRQ/DMA utility functions for DJGPP 2.01
 * 
 * Copyright (C) 1998  Fabian Nunez
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * The author can be reached by email at: fabian@cs.uct.ac.za
 * 
 * or by airmail at: Fabian Nunez
 *                   10 Eastbrooke
 *                   Highstead Road
 *                   Rondebosch 7700
 *                   South Africa
 */

#ifndef UTIL_H_
#define UTIL_H_

#include "mytypes.h"

/* used to store hardware definition of DMA channels */
typedef struct DmaChannel {
   BYTE page;     /* page register */
   BYTE offset;   /* offset register */
   BYTE length;   /* length register */
} DmaChannel;

/* function prototypes */
//void set_irq_handler(int irq,void (*handler)(void),IrqSave *irqsave);
//long alloc_dma_buffer(int size);
//void dma_xfer(int channel,long physaddr,int length,BOOL read);
//#include <dpmi.h>

/*typedef struct IrqSave {
   _go32_dpmi_seginfo oldhdlr;
   _go32_dpmi_seginfo newhdlr;
} IrqSave;*/

/* inline funcs */
extern inline BYTE inbxx(UINT16 port)
{
   register BYTE data;
   asm volatile ("inb %1,%0" : "=a" (data) : "d" (port));
   return data;
}

extern inline void outbxx(UINT16 port,BYTE data)
{
   asm volatile ("outb %1,%0" : : "d" (port),"a" (data));
}

extern inline WORD inw(UINT16 port)
{
   register WORD data;
   asm volatile ("inw %1,%0" : "=a" (data) : "d" (port));
   return data;
}

extern inline void outw(UINT16 port,WORD data)
{
   asm volatile ("outw %1,%0" : : "d" (port),"a" (data));
}

extern inline void wfill(WORD *start,UINT32 size,WORD value)
{
   asm volatile ("cld\n"
		 "\trep\n"
		 "\tstosw"
		 : /* no outputs */
		 : "D"(start),"c"(size),"a"(value)
		 : "%edi","%ecx");
}

void dma_xfer(int channel,long physaddr,int length,BOOL read);
unsigned int msleep(unsigned int _useconds);

#endif /* UTIL_H_ */
