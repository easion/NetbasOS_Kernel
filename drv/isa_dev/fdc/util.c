#include <drv/drv.h>
#include <drv/spin.h>
#include "util.h"
#include "fdc.h"

 int fd_read(unsigned minor, off_t  pos, char * buf,int count);
 int fd_write(unsigned minor, off_t  pos, char * buf,int count);
void deinit(void);
int cmos_read (int pos)
  {
    outb (0x70, pos);
    asm ("jmp 1f\n1:");
    return inb (0x71);
  }
CREATE_SPINLOCK( floppy_sem );

 int fd_read(unsigned minor, off_t  pos, char * buf,int count)
{
	bool res;
	int i;

    spin_lock( &floppy_sem );
	for (i=0; i<10; i++)
	{
		res = fdc_rw(pos,buf,TRUE);
		if (res)
		{
			break;
		}
	}
	spin_unlock( &floppy_sem );
	if (res)
	   return count;

	return -1;
}

int fd_write(unsigned minor, off_t  pos, char * buf,int count)
{
	bool res;
	int i;

    spin_lock( &floppy_sem ); 
	//printk("fd_write %d\n",pos);
	for (i=0; i<10; i++)
	{
		res = fdc_rw(pos,buf,FALSE);   
		if (res)
		{
			break;
		}
	}
	spin_unlock( &floppy_sem );
	if (res)
	{
	   return count;
	}
	return -1;
}

int fdc_open(const char *f, int mode)
{
	return 0x200;
}

int fdc_close(dev_prvi_t* devfp)
{
}

#ifdef __DLL__


int dll_main(char **argv)
{
	static const driver_ops_t ops =
	{
		d_name:		"fda",
		d_author:	"Fabian Nunez",
		d_kver:	CURRENT_KERNEL_VERSION,
		d_version:	"0.02",
		d_index:	0x200,
		d_index:	0x200,
		open:		fdc_open,
		close:		fdc_close,
		read:		fd_read,
		write:		fd_write,
		ioctl:		NULL,						
	};

	init();
	if(kernel_driver_register(&ops)!=OK)
		panic("register floppy failed");
	return 0;
}


int dll_destroy()
{
	deinit();
	return 0;
}

int dll_version()
{
	printk("JICAMA OS fdc VERSION 0.01!\n");
	return 0;
}
#endif

/* definition of DMA channels */
const static DmaChannel dmainfo[] = {
   { 0x87, 0x00, 0x01 },
   { 0x83, 0x02, 0x03 },
   { 0x81, 0x04, 0x05 },
   { 0x82, 0x06, 0x07 }
};

#define assert(test) ((void)0) 

/*
 * this sets up a DMA trasfer between a device and memory.  Pass the DMA
 * channel number (0..3), the physical address of the buffer and transfer
 * length.  If 'read' is TRUE, then transfer will be from memory to device,
 * else from the device to memory.
 */
void dma_xfer(int channel,long physaddr,int length,BOOL read)
{
   long page,offset;
   
   assert(channel < 4);
   
   /* calculate dma page and offset */
   page = physaddr >> 16;
   offset = physaddr & 0xffff;
   length -= 1;  /* with dma, if you want k bytes, you ask for k - 1 */

   disable();  /* disable irq's */
   
   /* set the mask bit for the channel */
   outb(0x0a,channel | 4);
   
   /* clear flipflop */
   outb(0x0c,0);
   
   /* set DMA mode (write+single+r/w) */
   outb(0x0b,(read ? 0x48 : 0x44) + channel);
   
   /* set DMA page */
   outb(dmainfo[channel].page,page);
   
   /* set DMA offset */
   outb(dmainfo[channel].offset,offset & 0xff);  /* low byte */
   outb(dmainfo[channel].offset,offset >> 8);    /* high byte */
   
   /* set DMA length */
   outb(dmainfo[channel].length,length & 0xff);  /* low byte */
   outb(dmainfo[channel].length,length >> 8);    /* high byte */
   
   /* clear DMA mask bit */
   outb(0x0a,channel);
   
   enable();  /* enable irq's */
}

#define CLOCKS_PER_SEC	91


unsigned int msleep(unsigned int _useconds)
{
	int i;
	u32_t cl_time;

	/* 977 * 1024 is about 1e6.  The funny logic keeps the math from
	 overflowing for large _useconds */
	//_useconds >>= 10;
	//cl_time = _useconds * CLOCKS_PER_SEC / 977;

	for (i=0; i<_useconds; i++)
	{
	}

	//mdelay(_useconds/10000);
	//mdelay(_useconds);
	return 0;
}





#define FORMAT


/* test functions */
int format(void)
{
   int block,i,c,track;
   BYTE trackbuff[512];
   DrvGeom geometry;

   for (i = 0;i < 512;i++) trackbuff[i] = 0;   
   
   puts("insert a HD stiffy that has nothing of value in it and press enter");
   //getchar();
   
#ifdef FORMAT

   log_disk(NULL);
   
   geometry.heads = DG168_HEADS;
   geometry.tracks = DG168_TRACKS;
   geometry.spt = DG168_SPT;
   
   /* format disk */
   for (i = 0;i < geometry.tracks;i++) {
      if (!format_track(i,&geometry)) {
	 if (diskchange())
	   printk("diskchange - abort!\n");
	 else
	   printk("\nerror!\n");
	 return 1;
      }
      printk("formatted track %d\r",i);
   }
   
#endif
   
   if (!log_disk(&geometry)) {
      printk("cannot read geometry!\n");
      return (1);
   }

   if (geometry.spt == DG144_SPT)
     printk("1.44M format\n");
   else
     printk("1.68M format\n");
   
   /* write block */
   for (block = 0;block < geometry.spt;block++) {
      sprintf(trackbuff,"block number %d",block);

      if (!write_block(block,trackbuff)) {
	 if (diskchange())
	   printk("diskchange - abort!\n");
	 else
	   printk("error writing!\n");
	 return 1;
      }
   }
   
   /* read block */
   for (block = 0;block < geometry.spt;block++) {
      strcpy(trackbuff,"************");
      if (!read_block(block,trackbuff)) {
	 if (diskchange())
	   printk("diskchange - abort!\n");
	 else
	   printk("error reading!\n");
	 return 1;
      }

      /* display block (1st 16 bytes) */
      for (i = 0;i < 16;i++)
	printk("%02x ",trackbuff[i]);
      
      printk(": ");
      
      for (i = 0;i < 16;i++) {
	 c = trackbuff[i];
	 printk("%c",isprint(c) ? c : '.');
      }
      
      printk("\n");
   }

   srand(0x99e32);
   
   /* seek a few times */
   for (i = 0;i < 10;i++) {
      track = rand() % 80;
      printk("seeking to %d: ",track);
      if (seek(track))
	printk("OK\n");
      else
	printk("error!\n");
   }

   printk("All done - press enter to finish\n");
   
   return 0;
}


