
// -----------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2005  DengPingPing      All rights reserved.  
//-------------------------------------------------------------------------------------

#include <errno.h>
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/rfb.h>
#include <ibm/io.h>
#include <jicama/spin.h>
#include <string.h>
#include <ibm/devices.h>



__local int _systemread(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
 {
	return 0;
}

__local int _systemwrite(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
{
	return 0;
}

__local int _systemioctl(dev_prvi_t* devfp, int cmd, void* args, int fromkernel)
{
	return 0;
}

__local int _systemopen(char *f, int mode,dev_prvi_t* devfp)
{
	return 0;
}

__local int _systemclose(dev_prvi_t* devfp)
{
	return 0;
}



void sysdev_init()
{
	__local const driver_ops_t ops =
	{
		d_name:		"system",
		d_author:	"easion",
		d_kver:	CURRENT_KERNEL_VERSION,
		d_version:	KMD_VERSION,
		d_index:	ALLOC_MAJOR_NUMBER,
		open:		_systemopen,
		close:		_systemclose,
		read:		_systemread,
		write:		_systemwrite,
		ioctl:		_systemioctl,			
	};

	if(kernel_driver_register(&ops)!=OK)
		panic("register device system failed");
}

void* memcpy_from_user( void *dst_ptr, const void *src_ptr, unsigned long count )
 {
	 return 0;
 }

 void* memcpy_to_user( void *dst_ptr, const void *src_ptr, unsigned long count )
 {
	 return 0;
 }

