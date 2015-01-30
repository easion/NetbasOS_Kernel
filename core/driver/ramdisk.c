
// -----------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2005  DengPingPing      All rights reserved.  
//-------------------------------------------------------------------------------------

#include <errno.h>
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/devices.h>
#include <string.h>
#include <assert.h>

__local u32_t ramdisk_addr=0;
__local u32_t ramdisk_end;
__local size_t ramdisk_size=0;
__local int disk_block_size;
__local int ramdisk_open(char *f, int mode,dev_prvi_t* devfp);
__local int ramdisk_close(dev_prvi_t* devfp);
__local int ramdisk_read(dev_prvi_t* devfp, off_t  offset, char * buf,int count);
__local int ramdisk_write(dev_prvi_t* devfp, off_t  offset, char * buf,int count);

__local int ramdisk_read(dev_prvi_t* devfp, off_t  offset, char * buf,int count)
{
	size_t pos;
	u32_t addr;

	if (ramdisk_size<=0 || !disk_block_size){
		return -1;
	}

	pos = disk_block_size*offset;
	addr =ramdisk_addr+pos;

	if(addr > ramdisk_end)
		return -1;
	else if(addr+count > ramdisk_end ){
		count = ramdisk_end - addr;
	}

	//kprintf("ramdisk_read called at %x %x!\n", ramdisk_addr, pos);
	memcpy(buf, (void *)addr, count);
   return count;
}

__local int ramdisk_write(dev_prvi_t* devfp, off_t  offset, char * buf,int count)
{
	size_t pos;
	u32_t addr;

	if (ramdisk_size<=0 || !disk_block_size){
		return -1;
	}

	pos = disk_block_size*offset;
	addr =ramdisk_addr+pos;

	if(addr > ramdisk_end)
		return -1;
	else if(addr+count > ramdisk_end ){
		count = ramdisk_end - addr;
	}

	kprintf("ramdisk_write called at %x %x!\n", ramdisk_addr, pos);
	memcpy((void *)addr, buf, count);
   return count;
}

__local int ramdisk_ioctl(dev_prvi_t* devfp, int cmd, void* args,int size, int bfrom)
{
	switch (cmd)
	{
	//设置开始地址
	case 1:
		ramdisk_addr = args;
		break;
	//设置磁盘尺寸
	case 2:
		ramdisk_size = args;
		ramdisk_end = ramdisk_addr + ramdisk_size;
		break;
	//设置块大小
	case 3:
		disk_block_size = (int)args;
		break;
	case 4:
		break;
	default:
		return 0;
		break;	
	}
	return 0;
}

int ramdisk_set_params(void *addr, int size, int blk_size)
{
	ASSERT(addr);
	ASSERT(size);
	ASSERT(blk_size);
	ramdisk_addr = addr;
	ramdisk_size = size;
	ramdisk_end = ramdisk_addr + ramdisk_size;
	disk_block_size = (int)blk_size;
	return 0;
}


__local const driver_ops_t ops =
{		
	d_name:		"ramdisk",
	d_author:	"easion",
	d_kver:	CURRENT_KERNEL_VERSION,
	d_version:	KMD_VERSION,
	d_index:	ALLOC_MAJOR_NUMBER,
	open:		ramdisk_open,
	close:		ramdisk_close,
	read:		ramdisk_read,
	write:		ramdisk_write,
	ioctl:		ramdisk_ioctl,		
};


__local int ramdisk_open(char *f, int mode,dev_prvi_t* devfp)
{
	return 0;
}

__local int ramdisk_close(dev_prvi_t* devfp)
{
	return 0;
}

int ramdisk_init(void)
{
	if(kernel_driver_register(&ops)!=OK){
		return -1;
	}
	return 0;
}



