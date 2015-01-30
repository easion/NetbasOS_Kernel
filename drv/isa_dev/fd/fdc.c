
// -------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003,2004  DengPingPing      All rights reserved.  
//---------------------------------------------------------------------------------------
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/ia.h>
#include <drv/errno.h>
 #include "floppy.h"

/*
** 2003, 9.15 Floppy Support OK
*/

static int motor = 0;
static int calibration = EGENERIC;
static int int_status=0;/* interrupt occured? 0:no 1:yes */
static void (*do_fp_intr)(void) = NULL;
static long addr = 0x9000L;
static unsigned char res_buffer[7];
int fp_recalibrate( int driver );
extern void * low_alloc(u16_t s);


static int floppy_handler(void)
{
	if(!int_status)
	panic("Stray Floppy Interrupt !\n");
	
	if(do_fp_intr != NULL)
		do_fp_intr();

	//outb(0x20,0x20);

	int_status = 0;
  return 0;
}

 int fd_read(unsigned minor, off_t  pos, char * buf,int count);
 int fd_write(unsigned minor, off_t  pos, char * buf,int count);

int floppy_init(void)
{
	int i;
	static const driver_ops_t ops =
	{
			"floppy",
			"EASION",
			"0.01",
			0x200,
				NULL,
				NULL,
				fd_read,
				fd_write,
				NULL,
		NULL
	};

   	put_irq_handler(6, &floppy_handler,NULL,"floppy");
	floppy_read();
	outb( 0x3f2, 0 );

	if(kernel_driver_register(&ops)!=OK)
		panic("register floppy failed");

	return OK;
}
#ifdef __DLL__
int dll_main(char **argv)
{
	printk("floppy init!\n");
	floppy_init(); /*server init*/
	return 0;
}


int dll_destroy()
{
	return 0;
}

int dll_version()
{
	printk("JICAMA OS fdc VERSION 0.01!\n");
	return 0;
}
#endif
static int is_busy()
{
  unsigned char status;

  status=inb(0x3f4);
  if(status & 128)
      return 1; 
  return 0;
}

void motor_on( void )
{
    if ( !motor )
    {
	outb( 0x3f2, 28 );
	motor = 1;
    }
	return;
}

// /////////////关闭软盘马达/////////////////
void motor_off( void )
{
    if ( motor )
    {
	outb( 0x3f2, 12 );
	motor = 0;
    }
	return;
}


/////////////////重新校准//////////////////////////
int fp_recalibrate( int driver )
{
	int r1, t;

    int_status = 1;
    motor_on();
	wait_on();

    sendbyte(0x07);  //recalibrate command
    sendbyte( driver );      //driver number
	wait_on();

    //int_status = 1;
	sendbyte (0x08);  //sens is ok?
    r1 = getbyte();
	
	 t = 1000;
    while (t>0 && int_status)
      {
	   wait_on ();
	    t--;
      }

    if (int_status){
	printk ("WARNING: no interrupt from fp_recalibrate( %d);\n", driver);
	return EGENERIC;
	}

    if (r1 >= 0x80){
	printk ("WARNING:  fp_recalibrate( int driver ); Execute Faild!\n");
    return EGENERIC;
	}
	motor_off();
	return OK;
}

//////////////////////定位///////////////////////////
int fdc_seek(int driver, int track, int h )
{
    int r1,r2;
		    
	 int_status = 1;
     motor_on();
	 wait_on();
	//printk( "Motor on!\n" );

    int_status = 1;
    sendbyte( 0x0f );  //seek command
    sendbyte( h*4 + driver);
    sendbyte( track );

	sendbyte (0x08);
    r1 = getbyte();
    r2 = getbyte();

    /*  r1 is status register 0  */
    if ((r1 & 0xf8)==0x20)
      {
	/*  Command ok.  */

	/*  Was it the correct cylinder?  */
	if (r2 != track){
		fp_recalibrate(driver );
        printk("r1: %d r2: %d\n", r1, r2);
		printk("Track Error!!!!\n");
	    return 0;
	  }
      return 1;
	  }
   return 0;
}
/////////////////////重置/////////////////////////////////
void reset( void )
{
    outb( 0x3f2, 0 );
    motor = 0;
    outb(0x3f4, 0 ); /////
    outb( 0x3f2, 0x0C );

	sendbyte( 0x03 ); ////specify drive timings 
    sendbyte( 0xdf );
    sendbyte( 0x02 );
   fdc_seek(0, 1, 0 );
    fp_recalibrate(0);
    motor_off();
}

	/*
	**NOTE: Code only for 1.44m. 
	**not support other type floppy
	*/
void block2hts(unsigned long block, int *head, int *track, int *sector )
{
    *head = ( block % ( 18 * 2 ) ) /	18;
    *track = block / ( 18 * 2 );
    *sector = block % 18 + 1;
    //printk("Head: %d, Track: %d, Sector: %d\n", *head, *track, *sector );
}

/*  Specify head load time, 
     step rate time:  */
static void fd_mode(void)
{   
	sendbyte (0x03);
    sendbyte (0xcf);
    sendbyte (0x06);
    outb (0x3f7, 0);
}

 int fp_read(unsigned int blk,  int secs, void* data)
{
	const int driver = 0;
	int try=3;
    int head, track, sector, i;

    block2hts( blk, &head, &track, &sector );

	//printk("fp blk = %d", blk);
    
	for (; try>0; try--)
    {
    isadma_startdma (2, (void *) addr,(size_t)(512*secs), 0x44);
  // fp_dma(0xe6, secs, addr);
	// printk( " fp_read(): dma start ok!\n" );

	fdc_seek(driver, track, head); // seek the track 

	wait_on();

   if ( inb( 0x3f7 ) & 0x80 )      /* Digital Input Register (input) */
	{
	    printk( " fp_read(): Disk change detected.\n" );
	    fdc_seek(driver, 0, 0 );
	    fp_recalibrate(0);
	    return EGENERIC;
	}

    fd_mode();
    int_status = 1;
    sendbyte (0xe6);  //send read command
    sendbyte (head*4 + 0);
    sendbyte (track);		/*  Cylinder  */
    sendbyte (head);		/*  Head  */
    sendbyte (sector);		/*  Sector  */
    sendbyte (2);		/*  0=128, 1=256, 2=512, 3=1024, ...  */
    sendbyte (sector+secs-1);	/*  Last sector in track:here are  sectors count */
    sendbyte (27);
    sendbyte (0xff);

     io_wait();

	  //  printk( " fp_read(): Command good!\n" );
       for(i=0;i<7;i++)
		res_buffer[i]=getbyte();

    if ((res_buffer[0]&0xf8)==0 && res_buffer[1]==0 && res_buffer[2]==0)
      {
	    /*  Copy the resulting data to 'buf':  */
	   //printk(" fp_read() Read OK!!!\n");
	   memcpy (data, (void *) addr, 512*secs);
	   return OK;
      }
	  fp_recalibrate( driver );
	}
   printk (" fp_read(): (r0=%X, r1=%X r2=%X)",
	 res_buffer[0],res_buffer[1],res_buffer[2]);
	motor_off();

	return OK;
}

int fp_write(unsigned int blk,   int secs, void* data)
{
	int try=3;
	const int driver = 0;
    int head, track, sector, i;
	
	 if(secs<1)
		 return 0;

    block2hts( blk, &head, &track, &sector );
    
	for (; try>0; try--)
    {
	motor_on();
	wait_on();

	 isadma_startdma (2, (void *) addr,(size_t)(512*secs), 0x48);
	 memcpy ( (void *) addr, data, 512*secs);  //Copy data to write address.

	fdc_seek(driver, track, head); // seek the track 
	wait_on();

	if ( inb( 0x3f7 ) & 0x80 )      /* Digital Input Register (input) */
	{
	    printk( "fp_write(): Disk change detected.\n" );
	    fdc_seek(driver, 1, 0);
	    fp_recalibrate(0);
	    return EGENERIC;
	}
	 
    fd_mode();
    int_status = 1;
    sendbyte (0xc5);  //send write command
    sendbyte (head*4 + 0);
    sendbyte (track);		/*  Cylinder  */
    sendbyte (head);		/*  Head  */
    sendbyte (sector);		/*  Sector  */
    sendbyte (2);		/*  0=128, 1=256, 2=512, 3=1024, ...  */
    sendbyte (sector+secs-1);	/*  Last sector in track:here are 2 sectors  */
    sendbyte (27);
    sendbyte (0xff);

     io_wait();

       for(i=0;i<7;i++)
		res_buffer[i]=getbyte();

    if ((res_buffer[0]&0xf8)==0 && res_buffer[1]==0 && res_buffer[2]==0)
      {
	   //printk("fp_write(): Write OK!!!\n");
	   return OK;
      }
	}
  
   printk ("fd: (r0=%X, r1=%X r2=%X)",
	 res_buffer[0],res_buffer[1],res_buffer[2]);
	motor_off();

	return EGENERIC;
}


int fd_read(unsigned minor, off_t  pos, char * buf,int count)
{
	int i=0, n=(count+511)/512;

	if(n>4)
		return EIO;

	while(i<n){
    if( fp_read(pos,  n, buf) != OK)
 		return -1;
	(char *)buf+=512;
	i++;
    }
   
   return OK;
}

int fd_write(unsigned minor, off_t  pos, char * buf,int count)
{
	int i=0, n=(count+511)/512;
	n%=4;
	//panic("write %d", count);

	while(i<n){
    if( fp_write(pos,  n, buf) != OK)
		return -1;
	buf+=512;
	i++;
    }
   
   return OK;
   }
