/*
**     (R)Jicama OS
**     Direct Memory Access controller interface
**     Copyright (C) 2003 DengPingPing
*/
#include <type.h>
#include <arch/x86/regs.h>
#include <jicama/devices.h>
#include <arch/x86/io.h>

/* enable/disable a specific DMA channel */
void enable_dma(unsigned int dmanr)
{
	if (dmanr<=3)
		outb(0xda, dmanr);
	else
		outb(0xd4, dmanr & 3);
}

void disable_dma(unsigned int dmanr)
{
	if (dmanr<=3)
		outb(0xda, dmanr | 4);
	else
		outb(0xd4, (dmanr & 3) | 4);
}


 void clear_dma_ff(unsigned int dmanr)
{
		outb( (dmanr<=3) ? 0x0c : 0xd8, 0);
}

/* set mode (above) for a specific DMA channel */
 void set_dma_mode(unsigned int dmanr, char mode)
{
	if (dmanr<=3)
		outb(0x0b, mode | dmanr);
	else
		outb(0xd6, mode | (dmanr&3)); //mode register (w)
}

/* Set only the page register bits of the transfer address.
 * This is used for successive transfers when we know the contents of
 * the lower 16 bits of the DMA current address register, but a 64k boundary
 * may have been crossed.
 */
void set_dma_page(unsigned int dmanr, char pagenr)
{
	switch(dmanr) {
		case 0:
			outb(0x87, pagenr);
			break;
		case 1:
			outb(0x83, pagenr);
			break;
		case 2:
			outb(0x81, pagenr);
			break;
		case 3:
			outb(0x82, pagenr);
			break;
		case 5:
			outb(0x8b, pagenr & 0xfe);
			break;
		case 6:
			outb(0x89, pagenr & 0xfe);
			break;
		case 7:
			outb(0x8a, pagenr & 0xfe);
			break;
	}
}


/* Set transfer address & page bits for specific DMA channel.
 * Assumes dma flip flop is clear.
 */
 void set_dma_addr(unsigned int dmanr, unsigned int a)
{
	set_dma_page(dmanr, a>>16);
	if (dmanr <= 3)  {
	    outb( a & 0xff, ((dmanr&3)<<1) + 0x00 );  //8 bit slave DMA, channels 0..3 
            outb( (a>>8) & 0xff, ((dmanr&3)<<1) + 0x00 );
	}  else  {
	    outb( (a>>1) & 0xff, ((dmanr&3)<<2) + 0xc0 ); //16 bit master DMA, ch 4(=slave input)..7
	    outb( (a>>9) & 0xff, ((dmanr&3)<<2) + 0xc0 );
	}
}


/* Set transfer size (max 64k for DMA1..3, 128k for DMA5..7) for
 * a specific DMA channel.
 * You must ensure the parameters are valid.
 * NOTE: from a manual: "the number of transfers is one more
 * than the initial word count"! This is taken into account.
 * Assumes dma flip-flop is clear.
 * NOTE 2: "count" represents _bytes_ and must be even for channels 5-7.
 */
 void set_dma_count(unsigned int dmanr, unsigned int count)
{
        count--;
	if (dmanr <= 3)  {
	    outb( ((dmanr&3)<<1) + 1 + 0x00, count & 0xff );
	    outb( ((dmanr&3)<<1) + 1 + 0x00, (count>>8) & 0xff );
        } else {
	    outb( ((dmanr&3)<<2) + 2 + 0xc0 , (count>>1) & 0xff);  ///16 bit master DMA, ch 4(=slave input)..7
	    outb( ((dmanr&3)<<2) + 2 + 0xc0, (count>>9) & 0xff );
        }
}

void fp_dma(int command, int secs, unsigned int dma_addr)
{  
	//NOTE:floppy dma :2
	disable();
	disable_dma(2);
	clear_dma_ff(2);                                        //fd.read //read - write
	set_dma_mode(2, (command == 0xe6)? 0x44 : 0x48);
	set_dma_addr(2, dma_addr);
	set_dma_count(2,512*secs);  //size count
	enable_dma(2);
	enable();
}
