
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------

#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/ia.h>
#include <drv/errno.h>
#include <drv/spin.h>
 #include "ide.h"
 #include "partition.h"

#define IOCTL_GETBLKSIZE        1
#define IOCTL_GETDEVSIZE        2
#define IOCTL_GETGEOMETRY       3
#define IOCTL_REVALIDATE        4


extern struct disk_partition hd_params[5];

static void (*do_hd_intr)(void) = NULL;
unsigned char NR_HD_DEV;
struct harddisk_struct hd_info[2];
static char disk_name[32];
static int g_readsem,g_wrtsem;

extern int read_partition(void);
static int ide_rw (BOOL is_writemode, unsigned long c, int sectors, void* buffer);
static int drive_busy(void);
static void reset_controller(void);
static int ide_sendcmd(unsigned char cmd);
static int ide_handler(void);
static int	ide_get(void);



 int hd_read(dev_prvi_t* devfp, off_t  pos, char * buf,int count);
 int hd_write(dev_prvi_t* devfp, off_t  pos, char * buf,int count);
static void *isr_handler;

 int hd_close(int  file)
{
	free_irq_handler(14,isr_handler);
	return 0;
}



int hd_ioctl(dev_prvi_t* devfp, int cmd, void *args,int len, int fromkernel)
{
	int minor = devfp->params[0];
	switch (cmd) {
    case IOCTL_GETBLKSIZE:
		//块数
		return 0;
   case IOCTL_GETDEVSIZE:
	   //块大小
		return 512;
	case IOCTL_GETGEOMETRY:
		return 0;	
	case IOCTL_REVALIDATE:
		return 0;	
	default:
		printk("hd_ioctl(): command %d not realization yet", cmd);
		return 0;
	}

	return 0;
}

 int hd_open(const char *f, int mode,dev_prvi_t* devfp)
{
	int minor=0;
	int len;

	len = strlen(f);

	if (len&&isdigit(f[len-1]))
	{		
		printf("hd_open = %s\n",f);
		minor=f[len-1]-'0';
	}	

	devfp->params[0] = minor; //
	return 0;
}

static const driver_ops_t ops =
{
	d_name:		"hdc",
	d_author:	"EASION",
	d_version:	" 1.0 $",
	d_index:	ALLOC_MAJOR_NUMBER,
	d_kver:	CURRENT_KERNEL_VERSION,
	open:		hd_open,
	close:		hd_close,
	read:		hd_read,
	write:		hd_write,
	ioctl:		hd_ioctl,					
};

void hd_init (void)
{
	int err;
	//char buf[512];
		
	if(ide_get()==0){
		kprintf("no ide interface found\n");
		return;
	}

	err=kernel_driver_register(&ops);

	if (err)
	{
		kprintf("kernel_driver_register ide error\n");
		return;
	}

	//printk("%s", __FUNCTION__);
	read_partition();  //Must before fs!!!
	//ide_readinfo();

	isr_handler = put_irq_handler(14, &ide_handler,NULL,"ide_hd");
	g_readsem = create_semaphore("hdread",SEMAPHONE_CLEAR_FLAGS,0);
	g_wrtsem = create_semaphore("hdwrt",SEMAPHONE_CLEAR_FLAGS,0);
}

void hd_deinit()
{
	kernel_driver_unregister(&ops);
	destroy_semaphore(g_readsem);
	destroy_semaphore(g_wrtsem);
}

static int ide_buffer_sz, this_request_errors;
static char *ide_buffer_ptr;

 //static BOOL read_wait_flags;
 //static BOOL write_wait_flags;

 void read_intr(void)
{
	while(inp8(HD_STATUS) & 1<<7);
	while(!(inp8(HD_STATUS) & 1<<3));

	insw(0x1F0, ide_buffer_ptr, ide_buffer_sz);
	//port_read(HD_DATA,(phys_bytes) this_request->buffer+ (phys_bytes) 512*(this_request->nsector&1),512);
	this_request_errors = 0;

	//read_wait_flags = FALSE;

	unlock_semaphore(g_readsem);

}

void write_intr(void)
{
		while(inp8(HD_STATUS) & 1<<7);
        while(!(inp8(HD_STATUS) & 1<<3));
	outsw(0x1F0, ide_buffer_ptr, ide_buffer_sz);

	//write_wait_flags = FALSE;	
	unlock_semaphore(g_wrtsem);
}

/////////////////////////////////////////////读取磁盘信息////////////////////////////////////
static int	ide_get(void)
{
	u16_t  buffer[256];
	 char *text;
	 unsigned char c_disks;
 
     c_disks = cmos_read(0x12);
	
	if (c_disks & 0xf0){
		if (c_disks & 0x0f){
			//printk("system read two disks\n");
			NR_HD_DEV = 2;
		}
		else{
			//printk("system only one disk :%d\n", c_disks);
			NR_HD_DEV = 1;
	   }
    }
	else{
		NR_HD_DEV = 0;
		printk("system no harddisk or system not support!\n");
		return 0;
	}

	outp8 (0x1F6, 0xA0 ); // Get first/second drive 
	outp8 (0x1F7, 0xEC);  // 获取驱动器信息数据 
    
	memset(hd_info, 0, sizeof(struct harddisk_struct));

	while (inp8(HD_STATUS)&0x80);
	insw(0x1f0, buffer, 256);  //将数据读到缓冲区
	hd_info[0].sect = buffer[6];
	hd_info[0].head = buffer[3];
	hd_info[0].cyl = buffer[1];

	text = (u8_t*)&buffer[27];
	swap_char(text);
	syslog (4, "Harddisk Type: %s\n", text );
	strcpy(disk_name, text);

	return NR_HD_DEV;
}

u64_t	ide_readinfo(void)
{
	int i, partsize;
	u64_t sect_count;

	sect_count = hd_info[0].head * hd_info[0].sect * hd_info[0].cyl;

	partsize =   sect_count /2048;
	printk("hdc: Total Space:%i M Sectors:%i  Heads:%i Cylinder:%i \n", partsize, 
		hd_info[0].sect, hd_info[0].head, hd_info[0].cyl);

#if 0
	 printk("Disk Partition Table :\n");
	// printk("Partition     Start  Size (Mb)  \tType\n");
	 for (i = 1; i < 5; i++)
	 {
		 if(hd_params[i].name)
		printk("PARTITION: %c: %d \t%s\n",hd_params[i].name,  hd_params[i].nr_sects / 2048,  hd_params[i].type);
	 }
#endif
	return sect_count;
}

static int drive_busy(void)
{
	unsigned int i;

	for (i = 0; i < 100000; i++)
		if (READY_STAT == (inp8(0x1f7) & (BUSY_STAT | READY_STAT)))
			break;
	i = inp8(0x1f7);  ///////////读取状态位
	i &= (BUSY_STAT | READY_STAT | SEEK_STAT);
	if (i == (READY_STAT | SEEK_STAT))
		return 0;
	printk("ide controller outtime !");
	return 1;
}

static void reset_controller(void)
{
	int	i;

	outp8(HD_CMD, 4);
	for(i = 0; i < 1000; i++)
		nop();
	outp8(HD_CMD,0);
	for(i = 0; i < 10000 && drive_busy(); i++) /* nothing */;
	if (drive_busy())
		printk("ide still busy now!");
	if((i = inp8(ERR_STAT)) != 1)
		printk("reset failed!");
}



static int ide_handler(void)
{
	//printk("ide_handler\n");
#if 1
	if(do_hd_intr==NULL)	{
		reset_controller();
		if (this_request_errors++>5)
		{
		panic("ERROR: Stray Harddisk Interrupt Happend ..");
		}
	}
	else
		do_hd_intr();
	return 0;
#endif
}

static int ide_sendcmd(unsigned char cmd)
{
   unsigned int c;
   unsigned char stat;
   unsigned int timeout;

   timeout = 10000000;
   
   c = timeout;
   while(inp8(HD_STATUS) & BUSY_STAT) {
      if(--c == 0) {
         return(-1);
      }
   }

   printk("not busy\n");

   outp8(HD_COMMAND, cmd);

   c = timeout;
   while(1) {
      stat = inp8(HD_STATUS);
      if((stat & BUSY_STAT) == 0) {
         return (stat);
      }
      if(--c == 0) {
         return(-1);
      }
   }
}

void set_ide_intr(char *buf, int buffersz, BOOL is_write)
{
	ide_buffer_ptr = buf;
	ide_buffer_sz = buffersz / 2;
	this_request_errors = 0;

	if (is_write)
	{
		//write_wait_flags = TRUE;
		do_hd_intr = &write_intr;
	}
	else
	{
		//read_wait_flags = TRUE;
		do_hd_intr = &read_intr;
	}

}


////////////////////////////读写硬盘///////////////////////////////////////////////////////////////////////////
static int ide_rw (BOOL is_writemode, 
unsigned long begin, int sectors, void* buffer)
{
   // unsigned __eflag;
	int sector, head, cylinder;

    // save_eflags(&__eflag);

	sector = begin &0xff;
	cylinder = (begin &0xffff00)>>8;
	head = (begin &0xf000000)>>24;

	while(inp8(HD_STATUS) & 1<<7);
	io_wait();
	outp8(HD_CMD,0);


	outp8(0x1f2, sectors);///////////读写的扇区数.这个端口应该是:0X1F2,下面开始增加一
	outp8(0x1f3,sector); ////////开始扇区
	outp8(0x1f4,cylinder);      ///////开始柱面
	outp8(0x1f5,cylinder>>8); ///////开始柱面高位
	outp8(0x1f6,0xE0|(0 <<4)|head);  /////主磁盘


	if (!is_writemode )
	{
	    outp8(0x1F7, 0x20); ////read命令

		//while(inp8(HD_STATUS) & 1<<7);
        //while(!(inp8(HD_STATUS) & 1<<3));
	   // insw(0x1F0, buffer, 256);
	   //while(read_wait_flags);
	   lock_semaphore(g_readsem);
	}
	else 
	{
		if (inp8(HD_STATUS & 0x01)) {
			printk(" issue write_command command error !\n");
			return -1;
		}
		outp8(0x1F7, 0x30);   ////write命令
		//while((inp8(HD_STATUS) & 0x08) == 0);  //DRQ
		//while(write_wait_flags);
		//outsw(0x1F0, buffer, 256);
		lock_semaphore(g_wrtsem);
     }	

	//restore_eflags(__eflag);
	return OK;
}

static int ide_lbarw (BOOL is_writemode, 
unsigned long begin, int sectors, void* buffer)
{
   // unsigned __eflag;
	int sector, head, cylinder;
#define HD_LBA                  0x40

    // save_eflags(&__eflag);

	sector = begin &0xff;
	cylinder = (begin &0xffff00)>>8;
	head = (begin &0xf000000)>>24  | HD_LBA;

	while(inp8(HD_STATUS) & 1<<7);

	outp8(0x1f2, sectors);///////////读写的扇区数.这个端口应该是:0X1F2,下面开始增加一
	outp8(0x1f3,sector); ////////开始扇区
	outp8(0x1f4,cylinder);      ///////开始柱面
	outp8(0x1f5,cylinder>>8); ///////开始柱面高位
	outp8(0x1f6,0xE0|(0 <<4)|head);  /////主磁盘


	if (!is_writemode )
	{
	    outp8(0x1F7, 0x20); ////read命令
#if 0
		while(inp8(HD_STATUS) & 1<<7);
        while(!(inp8(HD_STATUS) & 1<<3));
	    insw(0x1F0, buffer, 256);
#else
	   //while(read_wait_flags);
		lock_semaphore(g_readsem);
#endif
	}
	else 
	{
		if (inp8(HD_STATUS & 0x01)) {
			printk(" issue write_command command error !\n");
			return -1;
		}
		outp8(0x1F7, 0x30);   ////write命令
		//while((inp8(HD_STATUS) & 0x08) == 0);  //DRQ
		//while(write_wait_flags);
		//outsw(0x1F0, buffer, 256);
		lock_semaphore(g_wrtsem);
     }	

	//restore_eflags(__eflag);
	return OK;
}


CREATE_SPINLOCK( HD_sem );

int hd_read(dev_prvi_t* devfp, off_t  pos, char * pbuf,int count)
{
	u64_t sec,end_sec;
	int i=0;
	int  n=(count+511)/512;
	char * buf = pbuf;
	int minor = devfp->params[0];

	if (count%512 != 0 || count==0)
	{
		kprintf("%s() error read quest(%d) \n",__FUNCTION__, count);
		return -1;
	}

	if (minor>=MAIN_PART)return -1;	

    spin_lock( &HD_sem );


	sec =pos + hd_params[minor].lowsec;
	end_sec = (hd_params[minor].nr_sects+hd_params[minor].lowsec);

	//if (sec>end_sec)
	if ((pos+n)>end_sec)
	{
		kprintf("hd_read():  hd_params%d on current sector 0x%x (0x%x)overflow!\n", minor,sec,end_sec);
		spin_unlock( &HD_sem );
		return -1;
	}	


	while(i<n){	
		set_ide_intr(buf, 512, FALSE);
		if(ide_lbarw (FALSE,sec, 1, buf) != OK){
			kprintf("hd_params read %d sec at %d failed\n", n, sec);
			spin_unlock( &HD_sem );
			return -1;
		}
		//kprintf("read %s sector %d ok\n", buf, sec);
		buf+=512;
		sec++;
		i++;
    }
   
	spin_unlock( &HD_sem );
   return (n*512);
}

int hd_write(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
{
	u64_t sec;
	int i=0, n=(count+511)/512;
	int minor = devfp->params[0];


	if (minor>=MAIN_PART)return -1;	
    spin_lock( &HD_sem );

	sec =pos + hd_params[minor].lowsec;

	if (sec>hd_params[minor].nr_sects+hd_params[minor].lowsec)
	{
		kprintf("hd_read():  hd_params%d on sec %d overflow!\n", minor,sec);
		spin_unlock( &HD_sem );
		return -1;
	}


	while(i<n){
		//kprintf("hd_write %d sec\n", sec);
		set_ide_intr(buf, count, TRUE);
		if(ide_lbarw (TRUE, sec,1, buf) != OK){
			spin_unlock( &HD_sem );
			return -1;
		}
		buf+=512;
		sec++;
		i++;
    }
   
	spin_unlock( &HD_sem );

   return (n*512);
   }

