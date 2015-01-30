
// -----------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2005  DengPingPing      All rights reserved.  
//-------------------------------------------------------------------------------------

#include <errno.h>
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/devices.h>
#include <string.h>


__local int ioport_ioctl(dev_prvi_t* devfp, int cmd, void* args,int size, int bfrom)
{
	switch (cmd)
	{
	//设置开始地址
	case 1:
		break;
	//设置磁盘尺寸
	case 2:
		break;
	//设置块大小
	case 3:
		break;
	case 4:
		break;
	default:
		return 0;
		break;	
	}
	return 0;
}

__local int ioport_open(char *f, int mode,dev_prvi_t* devfp)
{
	return 0;
}

__local int ioport_close(dev_prvi_t* devfp)
{
	return 0;
}

int ioport_init(void)
{
	__local const driver_ops_t ops =
	{		
		d_name:		"ioport",
		d_author:	"easion",
		d_kver:	CURRENT_KERNEL_VERSION,
		d_version:	KMD_VERSION,
		d_index:	ALLOC_MAJOR_NUMBER,
		open:		ioport_open,
		close:		ioport_close,
		read:		NULL,
		write:		NULL,
		ioctl:		ioport_ioctl,		
	};

	if(kernel_driver_register(&ops)!=OK){
		return -1;
	}
	return 0;
}



