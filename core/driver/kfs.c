
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama OS  
// Copyright (C) 2002-2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------

#include <jicama/system.h>
#include <jicama/fs.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <string.h>

struct filp* kfs_do_open(char *p, char *mode);

__local file_t *cur_fp; 

extern struct filp raw_file[RAW_FILE_NR];

/*
**dump a kfs file
*/
void dump_kfs_file(char *buf, int len)
{
	int i;
	int readsz=0;

	 for (i=0; i<RAW_FILE_NR; i++){
		 if(!raw_file[i].name)continue;
		 sprintf(buf+readsz, "%s\t", raw_file[i].name);
		 readsz=strlen(buf);
	 }
}

/*
**
*/
__local int kfs_read(dev_prvi_t* devfp, off_t  pos, void * buf,int count)
 {
	 kprintf("kfs_read called\n");
	 if (cur_fp==NIL_FLP)
	 {
		 dump_kfs_file(buf, count);
		 return strlen(buf)+1;
	 }

	 if(cur_fp&&cur_fp->read_kfs){
		 kprintf("we read:\n");
		return cur_fp->read_kfs(cur_fp, buf, count);
	 }

	return 0;
}

/*
**write syscall
*/
__local int kfs_write(dev_prvi_t* devfp, off_t  pos, const void * buf,int count)
{
	 if(cur_fp&&cur_fp->write_kfs)
		return cur_fp->write_kfs(cur_fp, buf, count);

	return 0;
}

/*
**ioctl system call
*/

__local int kfs_ioctl(dev_prvi_t* devfp, int cmd, void* arg, int size, int fromkernel)
{
	int ret;
 void  * argp = (void  * )arg; 

  switch (cmd)
    {
    case 0:    
       return 0;    
	case 1:
		ret = 0;
		break;
    default:
      return -1;	
    }
	return ret;
}

/*
**
*/
__local int kfs_open(const char *fdevname, int mode)
{
	char filename[64];

	if (strcmp("/dev/kfs",fdevname)==0 )
	{
		cur_fp=NIL_FLP;
		return 0;
	}
	memset(filename, 0, sizeof(filename));
	strcpy(filename, &fdevname[9]);
	cur_fp=kfs_do_open(filename,mode);
	if (!cur_fp)
	{
		syslog(LOG_INFO,"open %s failed\n", fdevname);
		return -1;
	}
	return 0;
}

__local int kfs_close(dev_prvi_t* devfp)
{
	cur_fp = NULL;
	return 0;
}


/*
**file ops
*/
__local const driver_ops_t ops =
{
	d_name:		"kfs",
	d_author:	"Esaion",
	d_version:	"v2006-01-02",
	d_kver:	CURRENT_KERNEL_VERSION,
	d_index:	ALLOC_MAJOR_NUMBER,
	open:		kfs_open,
	close:		kfs_close,
	read:		kfs_read,
	write:		kfs_write,
	ioctl:		kfs_ioctl,		
};


/*
**kfs init
*/
int kfs_init()
{
	if(kernel_driver_register(&ops) != OK){
		panic("register device rfb failed");
	}

	cur_fp= NIL_FLP;
	return 0;
}

