
// -------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//----------------------------------------------------------------------------------
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/ia.h>
#include <drv/errno.h>
 #include "floppy.h"

static unsigned char FD_DEV = 0;

int sendbyte( int value )
{
    unsigned char stat, i;
    
    for ( i = 0; i < 128; i++ ) {
	stat = inb( 0x3f4 ) & (0x80+0x40);    /////////读取状态寄存器
	if  ( stat  == 0x80 )
	{
	    outb( 0x3f5, value );    ///////将参数写入数据寄存器
	    return 1;
	}
	wait_on(); // 作一些延迟
    }

    printk("Floppy- sendbyte(): FAILED..FUCK!  ..\n");
	return 0;
}

int getbyte( void )
{
    unsigned char stat, i;
    
    for ( i = 0; i < 128; i++ ) {
	stat = inb( 0x3f4 ) & (0x80+0x40+0x10); /////////读取状态寄存器
	
	if (stat == 0x80)
	  return -1;

	if ( stat  == 0xD0 )
	    return inb(0x3f5);  ///////获取数据

	wait_on(); // 延迟
    }
    printk("Floppy- getbyte(): FAILED..FUCK!  ..\n");
        return 0;
}


/////////////////////////////////软盘初始函数/////////////////////////////////////////
int floppy_read( void )
{
    unsigned int i, c;
    unsigned char cmtype, fd_type;
    unsigned char  fdd_name[2];
    unsigned char *fd_inc=0, *fdd=0;

    //printk("floppy_read(): send fp type command..  ..\n");
    sendbyte( 0x10 );  //fdc version
    //printk("floppy_read(): get fp type..  ..\n");
    i = getbyte();     ///

    switch ( i )        ////
    {
	case 0x80:	fd_inc =  "NEC765A controller"; break;
	case 0x90: fd_inc =  "NEC765B controller"; break;
	default: fd_inc =  "Enhanced controller" ; break;
    }

    for ( c = 0; c < 2; ++c ) // check for 2 floppy disks
    {
		cmtype = cmos_read(0x10);     //read floppy type from cmos
	     fd_type = ( cmtype >> ( 4 * ( 1 - c ) ) ) & 0x07;

       	if ( fd_type == 0 )
	               continue;

	FD_DEV++;
	switch( fd_type )
	{
	    case 0x02: // 1.2MB
	    {
		fd[ FD_DEV ].sectors = 15;
		fd[ FD_DEV ].tracks = 80;
		fd[ FD_DEV ].heads = 2;
	    fdd_name[0]=FD_DEV+0x41;
	    fdd_name[1]='\0';
	     break;
	    }
	    case 0x04: // 1.44MB       标准软盘
	    {
		fd[ FD_DEV ].sectors = 18;   ////////扇区
		fd[ FD_DEV ].tracks = 80;       //////////
		fd[ FD_DEV ].heads = 2;       /////磁头
	    fdd_name[0]=FD_DEV+0x41;
	    fdd_name[1]='\0';
	   break;
	    }
	    case 0x05: // 2.88MB
	    {
		fd[ FD_DEV ].sectors = 36;
		fd[ FD_DEV ].tracks = 80;
		fd[ FD_DEV ].heads = 2;
		fdd_name[0]=FD_DEV+0x41;
	    fdd_name[1]='\0';
		break;
	    }
		default:
			printk("Find a too old floppy driver! But System not Support it.\n");
		    break;
	}
	}

    printk("floppy_read(): FD COUNT- %d\n", FD_DEV);
	return 1;
}

#define DMA_ADDR       0x004	/* port for low 16 bits of DMA address */
#define DMA_TOP        0x081	/* port for top 4 bits of 20-bit DMA addr */
#define DMA_COUNT      0x005	/* port for DMA count (count =  bytes - 1) */
#define DMA_FLIPFLOP   0x00C	/* DMA byte pointer flip-flop */
#define DMA_MODE       0x00B	/* DMA mode port */
#define DMA_INIT       0x00A	/* DMA init port */
#define DMA_RESET_VAL   0x06

/* DMA channel commands. */
#define DMA_READ        0x46	/* DMA read command */
#define DMA_WRITE       0x4A	/* DMA write command */

/* Gather transfer data for each sector. */
static struct trans {		/* precomputed transfer params */
  unsigned tr_count;		/* byte count */
  //struct iorequest_s *tr_iop;	/* belongs to this I/O request */
  unsigned long tr_phys;		/* user physical address */
  unsigned long tr_dma;		/* DMA physical address */
} ftrans[18]; //MAX_SECTORS

#define DEV_WRITE 0
static void dma_setup(struct trans* tp, int command)
{
	/* Set up the DMA registers.  (The comment on the reset is a bit strong,
   * it probably only resets the floppy channel.)
   */
  outb(DMA_INIT, DMA_RESET_VAL);    /* reset the dma controller */
  outb(DMA_FLIPFLOP, 0);		/* write anything to reset it */
  outb(DMA_MODE, command == DEV_WRITE ? DMA_WRITE : DMA_READ);
  outb(DMA_ADDR, (int) tp->tr_dma >>  0);
  outb(DMA_ADDR, (int) tp->tr_dma >>  8);
  outb(DMA_TOP, (int) (tp->tr_dma >> 16));
  outb(DMA_COUNT, (tp->tr_count - 1) >> 0);
  outb(DMA_COUNT, (tp->tr_count - 1) >> 8);
  outb(DMA_INIT, 2);	/* some sort of enable */

/* The IBM PC can perform DMA operations by using the DMA chip.  To use it,
 * the DMA (Direct Memory Access) chip is loaded with the 20-bit memory address
 * to be read from or written to, the byte count minus 1, and a read or write
 * opcode.  This routine sets up the DMA chip.  Note that the chip is not
 * capable of doing a DMA across a 64K boundary (e.g., you can't read a
 * 512-byte block starting at physical address 65520).
 */
}

int fd_ioctl(void *dev, int cmd, void *args)
{	
	switch (cmd) {
    case 0:
		return 0;
   case 1:
		return 0;
	case 2:
		return 0;	
	
	default:
		printk("fd_ioctl(): command %d not realization yet", cmd);
		return 0;
	}
	return 0;
}
