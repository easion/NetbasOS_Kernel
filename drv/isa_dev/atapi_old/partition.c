/*
**     (R)Jicama OS
**     IDE Disk Partition
**     Copyright (C) 2003 DengPingPing
*/
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/errno.h>
#include <drv/ia.h>
 #include "ide.h"
 #include "partition.h"
 #include "part.h"


/*
*/
struct disk_partition hd_params[MAIN_PART+1];

extern void ide_rw (int RW, unsigned long sectors, void* buffer);

static char * get_fsname(unsigned char type)
{
       struct partitiontypes *pp;
       for (pp = partition_types; pp->type<0xff; pp++)
            if (pp->type == type)
	          return pp->name;
       return "bad file system or disk.we can't read it!!";
}

static bool ide_find_active (struct partition *p, int i)
{
		if(p->indicator == 0x80){
			//printk ("Active partition : %d \n", i);
			return TRUE;
		}
		return FALSE;
}

void io_wait(){
	inp8(0x80);
}

u64_t	ide_readinfo(void);


int read_partition(void)
{
	int i = 0;
	int sector, head, cylinder;
	u8_t buffer[512];
	struct partition* p;


	//printk("%s", __FUNCTION__);
#if 0
	hd_read(0, 0,  buffer, 512);
#else
	sector = 0 &0xff;
	cylinder = (0 &0xffff00)>>8;
	head = (0 &0xf000000)>>24;

	while(inp8(HD_STATUS) & 0x80);
	io_wait();
	outp8(HD_CMD,0);
	outp8(0x1f2, 1);///////////读写的扇区数.这个端口应该是:0X1F2,下面开始增加一
	outp8(0x1f3,0); ////////开始扇区
	outp8(0x1f4,cylinder);      ///////开始柱面
	outp8(0x1f5,cylinder>>8); ///////开始柱面高位
	outp8(0x1f6,0xE0|(0 <<4)|head);  /////主磁盘
	 outp8(0x1F7, 0x20); ////read命令
      while(inp8(HD_STATUS) & 0x80);
	  insw(0x1F0, buffer, 256);
#endif


	//all disk
	//hd_params[0].start_sect = 0;  	// 从0算起的绝对扇区
	hd_params[0].nr_sects = ide_readinfo();  // 分区扇区总数 
	hd_params[0].flag = 0;
	hd_params[0].lowsec = 0;
	hd_params[0].type = "alldisk";
	hd_params[0].name = 'z';

	if (buffer[510] != (unsigned char) 0x55 || buffer[511] != (unsigned char) 0xAA){
		printk ("read_partition():  Can't Read Harddisk(IDE) partition data...\n");
		return -1;
	}
  
	for (i=1;i<5;i++)
	{
	   p = (struct partition*)&buffer[0x1be + (i-1)*0x10]; //partition table offset after 446 byte.
		//hd_params[i].start_sect = p->start_sec;  	// 从0算起的绝对扇区
		hd_params[i].nr_sects = p->size;  // 分区扇区总数 
		hd_params[i].flag = p->type;
		hd_params[i].lowsec = p->lowsec;
		hd_params[i].type = get_fsname( p->type);
	    hd_params[i].name = 0x42+i;
	    ide_find_active(p, i);
	}
	return 0;
}

/*  
** Misc Function
*/
unsigned char get_pname(unsigned char type)
{
	int i;
	for (i = 1; i < 5; i++)
		if (hd_params[i].flag == type)
			return hd_params[i].name;

	return -1;
}

unsigned char get_ptype(unsigned char name)
{
	int i;
	for (i = 1; i < 5; i++)
		if (name == hd_params[i].name)
			return hd_params[i].flag;

	printk("Partition type Error, get_ptype faild!\n");
	return 0;
}


int get_lowsec(unsigned char type)
{
		int i;
	for (i = 1; i < 5; i++)
	{
		if (hd_params[i].flag == type)
				return hd_params[i].lowsec;
	}
	printk("Partition type Error, get_lowsec Faild!\n");
	return -9999; /*just for make a error!*/
}
