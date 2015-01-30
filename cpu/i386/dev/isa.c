
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama OS  
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------
/*
**    Code Origin From Yoctix,
**    Copyright (C) 2000 by Anders Gavare.  All rights reserved.
*/
#include <type.h>
#include <arch/x86/regs.h>
#include <jicama/devices.h>
#include <arch/x86/io.h>

#define	ISADMA_WRITE		44h
#define	ISADMA_READ		48h

int	isadma_maskreg [8]	= { 0x0a, 0x0a, 0x0a, 0x0a, 0xd4, 0xd4, 0xd4, 0xd4 };
int	isadma_modereg [8]	= { 0x0b, 0x0b, 0x0b, 0x0b, 0xd6, 0xd6, 0xd6, 0xd6 };
int	isadma_clearreg [8]	= { 0x0c, 0x0c, 0x0c, 0x0c, 0xd8, 0xd8, 0xd8, 0xd8 };

int	isadma_pagereg [8]	= { 0x87, 0x83, 0x81, 0x82, 0x8f, 0x8b, 0x89, 0x8a };
int	isadma_addrreg [8]	= { 0x00, 0x02, 0x04, 0x06, 0xc0, 0xc4, 0xc8, 0xcc };
int	isadma_countreg [8]	= { 0x01, 0x03, 0x05, 0x07, 0xc2, 0xc6, 0xca, 0xce };

void isadma_stopdma (int channelnr)
  {
     //  Mask the DMA channel:  
    outb (isadma_maskreg[channelnr], 0x04 + (channelnr & 3));

     //  Stop any currently executing transfers:  
    outb (isadma_clearreg[channelnr], 0);

     //  Unmask the DMA channel:  
    outb (isadma_maskreg[channelnr], channelnr & 3);
  }


    /*
     *	Sets up everything needed to do DMA data transfer.
     *
     *	channelnr is the DMA channel number
     *	addr is the physical address of the buffer (it may not cross
     *		a physical 64KB boundary!)
     *	len is the length in bytes to send/receive
     *	mode is either ISADMA_READ or ISADMA_WRITE.
     *
     *	Return 1 on success, 0 on failure.
     */
int isadma_startdma (int channelnr, void *addr, size_t len, int mode)
  {
    if (!addr || channelnr>=8 || len<1 || len>65536)
	return 0;

     //  Mask the DMA channel:  
    outb (isadma_maskreg[channelnr], 0x04 + (channelnr & 3));

     //  Stop any currently executing transfers:  
    outb (isadma_clearreg[channelnr], 0);

     //  Set the mode of the channel:  
    outb (isadma_modereg[channelnr], mode + (channelnr & 3));

     //  Send the address:  
    outb (isadma_addrreg[channelnr], (int)addr & 255);
    outb (isadma_addrreg[channelnr], ((int)addr >> 8) & 255);
    outb (isadma_pagereg[channelnr], ((int)addr >> 16) & 255);

     //  Send the length:  
    outb (isadma_countreg[channelnr], (len-1) & 255);
    outb (isadma_countreg[channelnr], ((len-1) >> 8) & 255);

     //  Unmask the DMA channel:  
    outb (isadma_maskreg[channelnr], channelnr & 3);

    return 1;
  }

