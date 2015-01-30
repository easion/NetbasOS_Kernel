
// -------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//--------------------------------------------------------------------------------------

#include <jicama/system.h>
#include <jicama/utsname.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <ansi.h>
#include <string.h>

 int mem_read(dev_prvi_t* devfp, off_t  pos, char * buf,int count);
int mem_write(dev_prvi_t* devfp, off_t  pos, char * buf,int count);
int mem_ioctl(dev_prvi_t* devfp, int, void* args,int size, int);


void mem_init()
{
	__local const driver_ops_t ops =
	{
		d_name: "mem",
		d_author: "easion",
		d_version:KMD_VERSION,
		d_index:RAM_DEVNO,
		d_kver:	CURRENT_KERNEL_VERSION,
		open:0,
		close:0,
		read:mem_read,
		write:mem_write,
		ioctl:mem_ioctl,	
		access:0,
	};

	if(kernel_driver_register(&ops)!=OK)
		panic("register mem failed");

}


 int mem_read(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
 {
	return 0;
}

int mem_write(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
{
	return 0;
}

int mem_ioctl(dev_prvi_t* devfp, int cmd, void* args,int len, int fromkernel)
{
	return 0;
}

