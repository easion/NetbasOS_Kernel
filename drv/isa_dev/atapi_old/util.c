#include <drv/drv.h>
#include <drv/spin.h>
#include <drv/ia.h>
#include "IDEDriver.h"

IDEController _controllers[2];
IDEController *controllers[] = {&_controllers[0], &_controllers[1],NULL};
int irqno;
int controller;

inline int cmos_read (int pos)
  {
    outp8 (0x70, pos);
    asm ("jmp 1f\n1:");
    return inp8 (0x71);
  }

//CREATE_SPINLOCK( atapi_sem );

 int atapi_sys_read(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
{
	int ret;

	if (count<ATAPI_SECTOR_SIZE)
	{
		//kprintf("error sector size\n");
		return -1;
	}

    //spin_lock( &atapi_sem );
	ret= atapi_read(controllers,pos, buf, count) ;
	//spin_unlock( &atapi_sem );

	if(ret == 0)
		return count;

	return -1;
}

int atapi_sys_write(dev_prvi_t* devfp, off_t  pos, char * buf,int count)
{   

	/*read only*/
   return -1;
}

/*
**
*/

int atapi_ioctl(int dev, int cmd, void *args, int len, int fromkernel);


static  driver_ops_t ops;
static void* isr_handler;

int _atapi_irq(void)
{
   /* EOI the PIC */
   outp8(0x20,0x20);
   return 0;
}

static int inited = 0;
 int atapi_open(const char *f, int mode,dev_prvi_t* devfp)
{
    int  deviceNo;	
	IDEController *ctr;
	//unsigned cur_flag;

	if (inited)
	{
		return 0;
	}

	//save_eflags(&cur_flag);	

	atapi_IDEDriver(controllers, IRQ_PRIMARY, IRQ_SECONDARY);
 
    if (!atapi_findDevice(controllers, DEVICE_ATAPI, 0x05, &controller, &deviceNo, &ctr))
    {
		//restore_eflags(cur_flag);
        kprintf("CD-ROM Not Found\n");
        return -1;
    }

	if (controller==PRIMARY)
	{
		irqno= IRQ_PRIMARY ;
	}
	else{
		irqno= IRQ_SECONDARY ;
	}

	//printf("atapi irqno = %d\n",irqno);

	isr_handler = put_irq_handler(irqno,&_atapi_irq,NULL,"atapi");

	//kprintf("atapi_selectDevice controller=%d,deviceNo=%d",controller, deviceNo);

	  /* CD Select Device */
    if (!atapi_selectDevice(controllers,controller, deviceNo))
    {
		//restore_eflags(cur_flag);
        kprintf("open: select device NG error code = %d\n", atapi_getLastError());
        return -1;
    }

	inited = 1;

	//restore_eflags(cur_flag);	
	return 0;
}

 int atapi_close(dev_prvi_t* devfp)
{
	inited = 0;
	free_irq_handler(irqno,isr_handler);
	return 0;
}

static driver_ops_t ops =
{
	d_name:		"atapi",
	d_author:	"HigePon",
	d_version:	" 1.2 $",
	d_index:	ALLOC_MAJOR_NUMBER,
	d_kver:	CURRENT_KERNEL_VERSION,
	open:		atapi_open,
	close:		atapi_close,
	read:		atapi_sys_read,
	write:		atapi_sys_write,
	ioctl:		atapi_ioctl,					
};

int strcmp(const char *s1, const char *s2)
{
  int ret = 0;
  while (!(ret = *(unsigned char *) s1 - *(unsigned char *) s2) && *s2) ++s1, ++s2;

  if (ret < 0)
    ret = -1;
  else if (ret > 0)
    ret = 1 ;

  return ret;
}

int hd_enable = 1;
int cdrom_enable = 1;

int dll_main(char **argv)
{
	int i=0;

	while (argv[i])
	{
		if (strcmp(argv[i], "disable-hd") == 0)
		{
			hd_enable = 0;
		}

		if (strcmp(argv[i], "disable-cdrom") == 0)
		{
			cdrom_enable = 0;
		}

		i++;
	}

	syslog(4, "hd_enable = %d cdrom_enable = %d\n", hd_enable, cdrom_enable);

	if (cdrom_enable)
	{
		if(kernel_driver_register(&ops)!=OK)
			panic("register atapi failed");
	}
	
	if(hd_enable){
		hd_init();
		//panic("hd init ok");
	}

	return 0;
}


int dll_destroy()
{
	if (hd_enable)
		hd_deinit();

	if (cdrom_enable)
		kernel_driver_unregister(&ops);

	return 0;
}

int dll_version()
{
	printk("JICAMA OS atapi Driver!\n");
	return 0;
}
