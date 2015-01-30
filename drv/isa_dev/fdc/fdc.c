/*
 * fdc.c
 * 
 * floppy controller handler functions
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


#include <drv/drv.h>
#include "util.h"
#include "fdc.h"
#include <drv/ia.h>
#include <drv/timer.h>

#define debug_out //printk

/* globals */
//static IrqSave oldirq6;
//static IrqSave oldint1c;
static volatile BOOL done = FALSE;
static BOOL dchange = FALSE;
static BOOL motor = FALSE;
static int mtick = 0;
static volatile int tmout = 0;
static BYTE status[7] = { 0 };
static BYTE statsz = 0;
static BYTE sr0 = 0;
static BYTE fdc_track = 0xff;
static DrvGeom geometry = { DG144_HEADS,DG144_TRACKS,DG144_SPT };

static long tbaddr;    /* physical address of track buffer located below 1M */

/* prototypes */
void sendbyte(int byte);
int getbyte();
void irq6(int);
void int1c(void);
BOOL waitfdc(BOOL sensei);
BOOL fdc_rw(int block,BYTE *blockbuff,BOOL read);
krnl_timer_t fp_timer;
/* helper functions */

/* init driver */
void init(void)
{
   int i;
   char *fd_inc;
    unsigned char cmtype, fd_type;
   
   /* allocate track buffer (must be located below 1M) */
   tbaddr = low_alloc(512); //0x9000L; //alloc_dma_buffer(512);
   
   /* install IRQ6 handler */
   put_irq_handler(6,&irq6,NULL,"FLOPPY");
   debug_out("IRQ6 handler installed\n");

   init_timer(&fp_timer,int1c,NULL);

   install_timer(&fp_timer,5000);
   debug_out("timer handler installed\n");

   reset();

   /* get floppy controller version */
   sendbyte(CMD_VERSION);

   i = getbyte();

    switch ( i )        ////
    {
	case 0x80:	fd_inc =  "NEC765A controller"; break;
	case 0x90: fd_inc =  "NEC765B controller"; break;
	default: fd_inc =  "Enhanced controller" ; break;
    } 

	cmtype = cmos_read(0x10);     //read floppy type from cmos
	 fd_type = ( cmtype >> ( 4 * ( 1 - 0 ) ) ) & 0x07;

	switch( fd_type )
	{
	    case 0x02: // 1.2MB
	    {
			debug_out("1.2MB  ");
			break;
	    }
	    case 0x04: // 1.44MB       ±Í◊º»Ì≈Ã
	    {
			debug_out("1.44MB  ");
		break;
	    }
	    case 0x05: // 2.88MB
	    {		
			debug_out("2.88MB  ");
			break;
	    }
		default:
			debug_out("Find a too old floppy driver! But System not Support it.\n");
		    break;
	}

     debug_out("%s found\n", fd_inc);
	//on
   motoron();
	outb(FDC_DATA, 0x03);
	outb(FDC_DATA, 0xC1);
	outb(FDC_DATA, 0x10);
	//off
   motoroff();


}

/* deinit driver */
void deinit(void)
{
	free_irq_handler(6,&irq6);
   debug_out("uninstalling IRQ6 handler\n");

   remove_timer(&fp_timer);
   debug_out("uninstalling timer handler\n");
   
   /* stop motor forcefully */
   outb(FDC_DOR,0x0c);
}


/* sendbyte() routine from intel manual */
void sendbyte(int byte)
{
   volatile int msr;
   int tmo;
   
   for (tmo = 0;tmo < 128;tmo++) {
      msr = inb(FDC_MSR);
      if ((msr & 0xc0) == 0x80) {
	 outb(FDC_DATA,byte);
	 return;
      }
      inb(0x80);   /* delay */
   }
}

/* getbyte() routine from intel manual */
int getbyte()
{
   volatile int msr;
   int tmo;
   
   for (tmo = 0;tmo < 128;tmo++) {
      msr = inb(FDC_MSR);
      if ((msr & 0xd0) == 0xd0) {
	 return inb(FDC_DATA);
      }
      inb(0x80);   /* delay */
   }

   return -1;   /* read timeout */
}

/* this waits for FDC command to complete */
BOOL waitfdc(BOOL sensei)
{
   tmout = 18;   /* set timeout to 1 second */
     
   /* wait for IRQ6 handler to signal command finished */
   while (!done && tmout)
     ;
   
   /* read in command result bytes */
   statsz = 0;
   while ((statsz < 7) && (inb(FDC_MSR) & (1<<4))) {
      status[statsz++] = getbyte();
   }

   if (sensei) {
      /* send a "sense interrupt status" command */
      sendbyte(CMD_SENSEI);
      sr0 = getbyte();
      fdc_track = getbyte();
   }
   
   done = FALSE;
   
   if (!tmout) {
      /* timed out! */
      if (inb(FDC_DIR) & 0x80)  /* check for diskchange */
	dchange = TRUE;
      
      return FALSE;
   } else
     return TRUE;
}

/* This is the IRQ6 handler */
void irq6(int irq)
{
	//kprintf("flpooy interupt happend\n");
   /* signal operation finished */
   done = TRUE;

   /* EOI the PIC */
   outb(0x20,0x20);
}

/* This is the timer (int 1ch) handler */
void int1c(void)
{
   if (tmout) --tmout;     /* bump timeout */
   
   if (mtick > 0)
     --mtick;
   else if (!mtick && motor) {
      outb(FDC_DOR,0x0c);  /* turn off floppy motor */
      motor = FALSE;
   }
   restart_timer(&fp_timer, 5000);
}

/*
 * converts linear block address to head/track/sector
 * 
 * blocks are numbered 0..heads*tracks*spt-1
 * blocks 0..spt-1 are serviced by head #0
 * blocks spt..spt*2-1 are serviced by head 1
 * 
 * WARNING: garbage in == garbage out
 */
void block2hts(int block,int *head,int *track,int *sector)
{
   *head = (block % (geometry.spt * geometry.heads)) / (geometry.spt);
   *track = block / (geometry.spt * geometry.heads);
   *sector = block % geometry.spt + 1;
}

/**** disk operations ****/

/* this gets the FDC to a known state */
void reset(void)
{
   /* stop the motor and disable IRQ/DMA */
   outb(FDC_DOR,0);
   
   mtick = 0;
   motor = FALSE;

   /* program data rate (500K/s) */
   outb(FDC_DRS,0);

   /* re-enable interrupts */
   outb(FDC_DOR,0x0c);

   /* resetting triggered an interrupt - handle it */
   done = TRUE;
   waitfdc(TRUE);

   /* specify drive timings (got these off the BIOS) */
   sendbyte(CMD_SPECIFY);
   sendbyte(0xdf);  /* SRT = 3ms, HUT = 240ms */
   sendbyte(0x02);  /* HLT = 16ms, ND = 0 */
   
   /* clear "disk change" status */
   seek(1);
   recalibrate();

   dchange = FALSE;
}

/* this returns whether there was a disk change */
BOOL diskchange(void)
{
   return dchange;
}

bool check_disk_change()
{
	/* check for diskchange */
	if (inb(FDC_DIR) & 0x80) {
		dchange = TRUE;
		seek(1);  /* clear "disk change" status */
		recalibrate();
		motoroff();
		debug_out("can not change disk status\n");
		return TRUE;
	}
	else
	{
	}

	  return FALSE;
}

/* this turns the motor on */
void motoron(void)
{
   if (!motor) {
      mtick = -1;     /* stop motor kill countdown */
      outb(FDC_DOR,0x1c);
      msleep(500); /* delay 500ms for motor to spin up */
      motor = TRUE;
   }
}

/* this turns the motor off */
void motoroff(void)
{
   if (motor) {
      mtick = 36;   /* start motor kill countdown: 36 ticks ~ 2s */
   }
}

/* recalibrate the drive */
void recalibrate(void)
{
   /* turn the motor on */
   motoron();
   
   /* send actual command bytes */
   sendbyte(CMD_RECAL);
   sendbyte(0);

   /* wait until seek finished */
   waitfdc(TRUE);
   
   /* turn the motor off */
   motoroff();
}

/* seek to track */
BOOL seek(int track)
{
   if (fdc_track == track)  /* already there? */
     return TRUE;
 
   motoron();
 
   /* send actual command bytes */
   sendbyte(CMD_SEEK);
   sendbyte(0);
   sendbyte(track);

   /* wait until seek finished */
   if (!waitfdc(TRUE))
     return FALSE;     /* timeout! */

   /* now let head settle for 15ms */
   msleep(15);
   
   motoroff();
   
   /* check that seek worked */
   if ((sr0 != 0x20) || (fdc_track != track))
     return FALSE;
   else
     return TRUE;
}

/* checks drive geometry - call this after any disk change */
BOOL log_disk(DrvGeom *g)
{
   /* get drive in a known status before we do anything */
   reset();

   /* assume disk is 1.68M and try and read block #21 on first track */
   geometry.heads = DG168_HEADS;
   geometry.tracks = DG168_TRACKS;
   geometry.spt = DG168_SPT;

   if (read_block(20,NULL)) {
      /* disk is a 1.68M disk */
      if (g) {
	 g->heads = geometry.heads;
	 g->tracks = geometry.tracks;
	 g->spt = geometry.spt;
      }
      return TRUE;             
   }
   
   /* OK, not 1.68M - try again for 1.44M reading block #18 on first track */
   geometry.heads = DG144_HEADS;
   geometry.tracks = DG144_TRACKS;
   geometry.spt = DG144_SPT;

   if (read_block(17,NULL)) {
      /* disk is a 1.44M disk */
      if (g) {
	 g->heads = geometry.heads;
	 g->tracks = geometry.tracks;
	 g->spt = geometry.spt;
      }
      return TRUE;
   }
   
   /* it's not 1.44M or 1.68M - we don't support it */
   return FALSE;
}

/* read block (blockbuff is 512 byte buffer) */
BOOL read_block(int block,BYTE *blockbuff)
{
   return fdc_rw(block,blockbuff,TRUE);
}

/* write block (blockbuff is 512 byte buffer) */
BOOL write_block(int block,BYTE *blockbuff)
{
   return fdc_rw(block,blockbuff,FALSE);
}




/*
 * since reads and writes differ only by a few lines, this handles both.  This
 * function is called by read_block() and write_block()
 */
BOOL fdc_rw(int block,BYTE *blockbuff,BOOL read)
{
   int head,track,sector,tries;

   /* convert logical address into physical address */
   block2hts(block,&head,&track,&sector);
//   debug_out("block %d = %d:%02d:%02d\n",block,head,track,sector);
   
   /* spin up the disk */
   motoron();

   if (!read && blockbuff) {
      /* copy data from data buffer into track buffer */
     // movedata(_my_ds(),(long)blockbuff,_dos_ds,tbaddr,512);
	 memcpy ( (void *) tbaddr, blockbuff, 512);  //Copy data to write address.
   }
   
   for (tries = 0;tries < 3;tries++)
	  {
		if (check_disk_change())	{	return FALSE;}
         
      /* move head to right track */
      if (!seek(track)) {
		 motoroff();
		 debug_out("seek disk failed\n");
		 return FALSE;
      }
      
      /* program data rate (500K/s) */
      outb(FDC_CCR,0);
      
      /* send command */
      if (read) {
		 dma_xfer(2,tbaddr,512,FALSE);
		 sendbyte(CMD_READ);
      } else {
		 dma_xfer(2,tbaddr,512,TRUE);
		sendbyte(CMD_WRITE);
      }
      
      sendbyte(head << 2);
      sendbyte(track);
      sendbyte(head);
      sendbyte(sector);
      sendbyte(2);               /* 512 bytes/sector */
      sendbyte(geometry.spt);

      if (geometry.spt == DG144_SPT)
		sendbyte(DG144_GAP3RW);  /* gap 3 size for 1.44M read/write */
      else
		sendbyte(DG168_GAP3RW);  /* gap 3 size for 1.68M read/write */

      sendbyte(0xff);            /* DTL = unused */
      
      /* wait for command completion */
      /* read/write don't need "sense interrupt status" */
      if (!waitfdc(FALSE)){
		  debug_out("wait time out\n");
		return FALSE;   /* timed out! */
	  }
      
      if ((status[0] & 0xc0) == 0) break;   /* worked! outta here! */
   
      recalibrate();  /* oops, try again... */
   }
   
   /* stop the motor */
   motoroff();

   if (read && blockbuff) {
      /* copy data from track buffer into data buffer */
      //movedata(_dos_ds,tbaddr,_my_ds(),(long)blockbuff,512);
	 memcpy (blockbuff,  (void *) tbaddr, 512);  //Copy data to write address.
   }

#if 0
   debug_out("status bytes: ");
   for (i = 0;i < statsz;i++)  debug_out("%02x ",status[i]);
   debug_out("\n");
#endif
   return (tries != 3);
}

/* this formats a track, given a certain geometry */
BOOL format_track(BYTE track,DrvGeom *g)
{
   int i,h,r,r_id,split;
   BYTE tmpbuff[256];

   /* check geometry */
   if (g->spt != DG144_SPT && g->spt != DG168_SPT)
     return FALSE;
   
   /* spin up the disk */
   motoron();

   /* program data rate (500K/s) */
   outb(FDC_CCR,0);

   seek(track);  /* seek to track */

   /* precalc some constants for interleave calculation */
   split = g->spt / 2;
   if (g->spt & 1) split++;
   
   for (h = 0;h < g->heads;h++) {
      /* for each head... */
      
      /* check for diskchange */
      if (inb(FDC_DIR) & 0x80) {
	 dchange = TRUE;
	 seek(1);  /* clear "disk change" status */
	 recalibrate();
	 motoroff();
	 return FALSE;
      }

      i = 0;   /* reset buffer index */
      for (r = 0;r < g->spt;r++) {
	 /* for each sector... */

	 /* calculate 1:2 interleave (seems optimal in my system) */
	 r_id = r / 2 + 1;
	 if (r & 1) r_id += split;
	 
	 /* add some head skew (2 sectors should be enough) */
	 if (h & 1) {
	    r_id -= 2;
	    if (r_id < 1) r_id += g->spt;
	 }
      
	 /* add some track skew (1/2 a revolution) */
	 if (track & 1) {
	    r_id -= g->spt / 2;
	    if (r_id < 1) r_id += g->spt;
	 }
	 
	 /**** interleave now calculated - sector ID is stored in r_id ****/

	 /* fill in sector ID's */
	 tmpbuff[i++] = track;
	 tmpbuff[i++] = h;
	 tmpbuff[i++] = r_id;
	 tmpbuff[i++] = 2;
      }

      /* copy sector ID's to track buffer */
      //movedata(_my_ds(),(long)tmpbuff,_dos_ds,tbaddr,i);
  	 memcpy ( (void *) tbaddr, tmpbuff, i);  //Copy data to write address.
    
      /* start dma xfer */
      dma_xfer(2,tbaddr,i,TRUE);
      
      /* prepare "format track" command */
      sendbyte(CMD_FORMAT);
      sendbyte(h << 2);
      sendbyte(2);
      sendbyte(g->spt);
      if (g->spt == DG144_SPT)      
	sendbyte(DG144_GAP3FMT);    /* gap3 size for 1.44M format */
      else
	sendbyte(DG168_GAP3FMT);    /* gap3 size for 1.68M format */
      sendbyte(0);     /* filler byte */
	 
      /* wait for command to finish */
      if (!waitfdc(FALSE))
	return FALSE;
      
      if (status[0] & 0xc0) {
	 motoroff();
	 return FALSE;
      }
   }
   
   motoroff();
   
   return TRUE;
}
