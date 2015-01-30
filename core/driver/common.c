
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
#include <jicama/proc_entry.h>

int kernel_driver_register( driver_ops_t *devop);
__asmlink void mem_init();
__asmlink void con_init();
__asmlink void sysdev_init();
__asmlink int busman_init();

//__local int last_device_id = 1;
int kfs_init();
static const char *dll_dev_name="devinfo";

file_t* dll_info_init();
static fd_set g_driver_mask;

static TAILQ_HEAD(,device_struct) driver_list;

driver_ops_t *dev_driver_header()
{
	return TAILQ_FIRST(&driver_list);
}


/*
**
*/
 __local int null_read(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
 {
	 kprintf("null_read null\n");
	return 0;
}

/*
**
*/
__local int null_write(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
{
	return 0;
}

/*
**
*/
__local int null_ioctl(dev_prvi_t* devfp, int buf, void* args, int from)
{
	return 0;
}

__local int null_open(const char *f, int mode,dev_prvi_t* devfp)
{
	//kprintf("null_open called\n");
	return 0;
}

__local int null_close(dev_prvi_t* devfp)
{
	return 0;
}


/*
**
*/
__public void device_init(void){
	int handle;
	__local const driver_ops_t ops =
	{
		d_name:		"null",
		d_author:	"easion",
		d_kver:	CURRENT_KERNEL_VERSION,
		d_version:	KMD_VERSION,
		d_index:	ALLOC_MAJOR_NUMBER,
		open:		null_open,
		close:		null_close,
		read:		null_read,
		write:		null_write,
		ioctl:		null_ioctl,		
	};


	FD_ZERO(&g_driver_mask);
	TAILQ_INIT(&driver_list);

	con_init();
	mem_init(); //register memory device
	ramdisk_init();

	if(kernel_driver_register(&ops)!=OK)
		panic("register device null failed");


	ioport_init();
	sysdev_init();
	kfs_init();	
	busman_init();
	trace ("devices init ok\n");

	return;
}




/*
**
*/
__local driver_ops_t *dev_get(int fd)
{
	driver_ops_t *item;
	void *nxt;

	TAILQ_FOREACH_SAFE(item,&driver_list,next_dev,nxt){
		if (MAJOR(item->d_index)==MAJOR(fd))
		{
			return item;
		}
	}


	return NULL;
}

__local driver_ops_t *dev_find(char *name)
{
	driver_ops_t *item;
	void *nxt;

	TAILQ_FOREACH_SAFE(item,&driver_list,next_dev,nxt){
		if (strcmp(item->d_name,name)==0)
		{
			return item;
		}

		//kprintf("found %s\n",item->d_name);
	}


	return NULL;
}





int dev_clear_bit(int devno)
{
	FD_SET(MAJOR(devno),&g_driver_mask);
	return 0;
}

int dev_alloc_bit(int devno)
{
	int i;
	driver_ops_t *item;
	int maj=MAJOR(devno);

	if (devno!=ALLOC_MAJOR_NUMBER)
	{
		if (! FD_ISSET(maj, &g_driver_mask) )
		{
			FD_SET(maj,&g_driver_mask);
			return maj;
		}
		else{
			return -2;
		}
	}

	for (i=0; i<FD_SETSIZE; i++){
		if (!FD_ISSET(i,&g_driver_mask))
		{
			FD_SET(i,&g_driver_mask);
			return i;
		}
	}
	return -1;
}




/*
**
*/

__public int kernel_driver_register(driver_ops_t *devop)
{
	int i;
	int freeslot=-1;
	/*new register a new devices driver*/
	dev_t handle=MAJOR(devop->d_index);
	driver_ops_t *item;

	if (!devop){
		kprintf("%s() error got line %d\n",__FUNCTION__,__LINE__);
		return -3;
	}

	/*check it*/
	if((handle >= FD_SETSIZE && devop->d_index!=ALLOC_MAJOR_NUMBER)){
		kprintf("kernel_driver_register: %x %x error\n",handle,devop->d_index);
		return ENXIO;
	}

	if (devop->d_kver != CURRENT_KERNEL_VERSION){
		//系统版本不匹配
		kprintf("%s() error got line %d\n",__FUNCTION__,__LINE__);
		return -2;
	}

	if (dev_find(devop->d_name))
	{
		kprintf("%s() %s found(line %d)\n",__FUNCTION__,__LINE__,devop->d_name);
		return -1;
	}

	freeslot = dev_alloc_bit(devop->d_index);
	if (freeslot<0)
	{
		kprintf("%s() dev_alloc_bit error for %s (line %d)\n",__FUNCTION__,__LINE__,devop->d_name);
		return freeslot;
	}

	if (devop->d_index == ALLOC_MAJOR_NUMBER)
	{
		devop->d_index = freeslot<<8;
	}

	TAILQ_INSERT_TAIL(&driver_list, devop,next_dev);	

	//kprintf("%s() succ got line %d %s OK\n",__FUNCTION__,__LINE__,devop->d_name);

	//g_driver_mask[freeslot]=(driver_ops_t*)devop;
	return 0;
}

/*
**
*/
__public int kernel_driver_unregister(driver_ops_t *old)
{
	int handle=MAJOR(old->d_index);
	int error = -1;
	//driver_ops_t *tail = g_driver_head;
	//driver_ops_t *item;

	if(handle >= FD_SETSIZE)
		return ENXIO;

	if (old==NULL)
	{
		return EBUSY;
	}

	dev_clear_bit(old->d_index);


	TAILQ_REMOVE(&driver_list, old,next_dev);

	return error;
	}

/*
**
*/
__public int dev_ioctl(dev_prvi_t* devfp,int cmd, void * arg, int size, int b_from_kernel)
{
	driver_ops_t *dev_op;	

	dev_op = dev_get(devfp->devno);

	if(dev_op == (driver_ops_t*)0)	{
		kprintf("dev_ioctl(%p,%d) not exist\n",devfp, devfp->devno);
		return EAGAIN;
	}
	if(dev_op->ioctl == NULL){
		kprintf("dev_ioctl(%d) have n't ioctl function\n", devfp->devno);
		return -1;
	}

	//kprintf("process fd %d\n", devfp->devno);

	return dev_op->ioctl(devfp, cmd, arg,size, b_from_kernel);
}

/*
**
*/
__public int dev_read(dev_prvi_t* devfp,off_t blkpos, void *buf, unsigned len)
{
	driver_ops_t *dev_op;

	//kprintf("dev_read() called 0x%x\n", dev_no);
	
	dev_op = dev_get(devfp->devno);

	if(dev_op == NULL){
		trace("UNREGISTER Device(0x%x)!\n", devfp->devno);
		return EAGAIN;
	}
	if(dev_op->read == NULL){
		trace("UNREGISTER read ops(0x%x)!\n", devfp->devno);
		return -1;
	}
	
	//kprintf("/dev/%s (0x%x) pos%d len%d\n", dev_op->d_name, dev_no, blkpos, len);
	return dev_op->read(devfp,blkpos,(u8_t*) buf, len);
}

/*
**
*/
__public int dev_write(dev_prvi_t* devfp, off_t blkpos, void *buf, unsigned len)
{
	driver_ops_t *dev_op;

	//kprintf("dev_write %x\n",devfp->devno);

	dev_op = dev_get(devfp->devno);

	if(dev_op == NULL){
		trace("UNREGISTER Device(0x%x)!\n", devfp->devno);
		return EAGAIN;
	}

	if(dev_op->write == NULL){
		kprintf("UNREGISTER write ops(0x%x)!\n", devfp->devno);
		return -1;
	}
	
	return dev_op->write(devfp, blkpos, (u8_t*)buf, len);
}


__public int dev_attach(dev_prvi_t* devfp, void *netif,void*hwaddr, int (*receive)(void *netif, void *p))
{
	driver_ops_t *dev_op;
	
	dev_op = dev_get(devfp->devno);
	if(dev_op == NULL){
		trace("UNREGISTER Device(0x%x)!\n", devfp->devno);
		return EAGAIN;
	}
	dev_op->netif = netif;
  dev_op->receive = receive;
  return dev_op->attach(devfp, hwaddr);
}

int dev_detach(dev_prvi_t* devfp)
{
	int rc;
	driver_ops_t *dev_op;
	
	dev_op = dev_get(devfp->devno);
	if(dev_op == NULL){
		trace("UNREGISTER Device(0x%x)!\n", devfp->devno);
		return EAGAIN;
	}

  if (dev_op->detach) 
    rc = dev_op->detach(devfp);
  else
    rc = 0;

  dev_op->netif = NULL;
  dev_op->receive = NULL;

  return rc;
}


__public int dev_transmit(dev_prvi_t* devfp, void *p)
{
	driver_ops_t *dev_op;
	
	dev_op = dev_get(devfp->devno);
	if(dev_op == NULL){
		trace("UNREGISTER Device(0x%x)!\n", devfp->devno);
		return EAGAIN;
	}
  if (!dev_op->transmit) return -ENOSYS;

  return dev_op->transmit(devfp, p);
}

__public int dev_receive(dev_prvi_t* devfp, void *p)
{
	driver_ops_t *dev_op;


	dev_op = dev_get(devfp->devno);
	if(dev_op == NULL){
		trace("UNREGISTER Device(0x%x)!\n", devfp->devno);
		return EAGAIN;
	}

  if (!dev_op->receive){
	  kprintf("dev_receive no device here\n");
	  return -ENOSYS;
  }

  return dev_op->receive(dev_op->netif, p);
}



/*
**
*/
__public int dev_open(const char *path, unsigned access,dev_prvi_t*ret)
{
	int i=0, fd=-1;
	int error = -1;
	char dev_name[64];
	driver_ops_t *fops;


	if(path == NULL)
		return -1;



	if (strnicmp(path, "/dev/",5)!=0){
		return -1;
	}

	if (!ret)
	{
	kprintf("%s()  got line %d %s error\n",__FUNCTION__,__LINE__,path);
		return -1;
	}	
	
	memset(dev_name, 0, sizeof(dev_name));
	
	//get node name:
	while (path[5+i]!='\0' && path[5+i]!='/')
	{
		dev_name[i]=path[5+i];
		i++;
	}

	//dev_name=(char *)path+5;

	fops = dev_find(dev_name);

	if(!fops){
		kprintf("%s()  got line %d %s, %s error\n",__FUNCTION__,__LINE__,path,dev_name);
		return -1;	
	}

	
	//fops->access = access;
	if(fops->open!=0)
		error = fops->open((char *)path, access,ret);
	else{
		error = -1;
		kprintf("dev_open(): dev %s at 0x%x ,but not register!\n", path, fops->d_index);
	}

	if (error < 0)
	{
	kprintf("%s()  got line %d %s error\n",__FUNCTION__,__LINE__,path);
		return error;
	}

	ret->devno = fops->d_index;

	//kprintf("dev_open ret->devno=%x\n",ret->devno);
	init_ioobject(&ret->iob,"DEV");
	return (fops->d_index);
}

/*
**
*/
__public int dev_close(dev_prvi_t* devfp)
{
	driver_ops_t *dev_op;
	int rv;

	dev_op = dev_get(devfp->devno);

	if(dev_op == NULL)
		return EAGAIN;

	if(dev_op->close == NULL)
		return -1;	

	rv = dev_op->close(devfp);	

	detach_ioobject(&devfp->iob);
	return rv;
}

/*
**
*/
__public int devices_dump(char *buf, int count)
{
	int len = 0;
	int i;
	void *nxt;

	len += sprintf(buf+len,"%-15s\t%s\t%s\t\t%s\t\t%s\n", 
		"Name","Number","Author","Version");

	driver_ops_t *item;

	TAILQ_FOREACH_SAFE(item,&driver_list,next_dev,nxt)
	{			
		len+=sprintf(buf+len,"%-15s\t0x%x\t%s\t\t%s\t\n", 
			item->d_name,
			item->d_index,
			(item->d_author==NULL)?"(NULL)":item->d_author ,
			(item->d_version==NULL)?"(NULL)":item->d_version
			//item->d_kver
			//item->d_license
			);
	}
	
	return len;
}

void dump_devicesinfo(char *buf, int len)
{
	int readsz=0;
	driver_ops_t *item;
	void *nxt;

	TAILQ_FOREACH_SAFE(item,&driver_list,next_dev,nxt){
		 sprintf(buf+readsz, "%s:\t%x\t %s\n", 
			 item->d_name, item->d_index, 
			 item->d_author);
		 readsz=strlen(buf);
	 }
}

