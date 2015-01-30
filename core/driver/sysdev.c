
// -----------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2005  DengPingPing      All rights reserved.  
//-------------------------------------------------------------------------------------

#include <errno.h>
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/spin.h>
#include <string.h>
#include <jicama/devices.h>


__local int s_system_read(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
 {
	return 0;
}

__local int s_system_write(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
{
	return 0;
}

__local int s_system_ioctl(dev_prvi_t* devfp, int cmd, void* args,int size, int b_from_kernel)
{
	return 0;
}

__local int s_system_open(char *f, int mode,dev_prvi_t* devfp)
{
	return 0;
}

__local int s_system_close(dev_prvi_t* devfp)
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
		open:		s_system_open,
		close:		s_system_close,
		read:		s_system_read,
		write:		s_system_write,
		ioctl:		s_system_ioctl,		
	};

	if(kernel_driver_register(&ops)!=OK)
		panic("register device system failed");
}

#define GZIP_DEV
#ifdef GZIP_DEV
enum{
	GZIPDEV_FORMAT,
	GZIPDEV_OUT_FILELENGTH,
	GZIPDEV_ZIP,
	GZIPDEV_UNZIP,
	GZIPDEV_MAX_CMD,
};

__local int gzip_read(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
 {
	return 0;
}

__local int gzip_write(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
{
	return 0;
}

__local int gzip_ioctl(dev_prvi_t* devfp, int cmd, void* args,int size, int fromkernel)
{
	int error = 0;
	u32_t *argp = args;
int do_gunzip(uint8_t *dest, const uint8_t *src, int src_len);

	if (!fromkernel)
	{
		argp = current_proc_vir2phys(args);
	}

	switch (cmd)
	{
	case GZIPDEV_FORMAT:
		{ 
		error = is_gzip_format_file(argp);
		break;
		}
	case GZIPDEV_OUT_FILELENGTH:
		{ 
		void *addr = current_proc_vir2phys(argp[0]);
		int len = (argp[1]);

		error = gzip_orig_length(addr, len);
		break;
		}

	case GZIPDEV_UNZIP: 
		{ 
		void *dest_addr = current_proc_vir2phys(argp[0]);
		void *src_addr = current_proc_vir2phys(argp[1]);
		int len = (argp[2]);

		gzip_ran_init();
		error = do_gunzip(dest_addr,src_addr, len);
		break; 
		}
	default : 
		{ 
		error = -1;
		break; 
		}
	
	}
	return error;
}

__local int gzip_open(char *f, int mode,dev_prvi_t* devfp)
{
	return 0;
}

__local int gzip_close(dev_prvi_t* devfp)
{
	return 0;
}



void gzip_dev_init()
{
	__local  driver_ops_t ops =
	{		
		d_name:		"gzip",
		d_author:	"easion",
		d_kver:	CURRENT_KERNEL_VERSION,
		d_version:	KMD_VERSION,
		d_index:	ALLOC_MAJOR_NUMBER,
		open:		gzip_open,
		close:		gzip_close,
		read:		gzip_read,
		write:		gzip_write,
		ioctl:		gzip_ioctl,		
	};


	if(kernel_driver_register(&ops)!=OK)
		panic("register device gzip failed");
}

#endif


