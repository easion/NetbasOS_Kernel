#include <drv/defs.h>
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/errno.h>


struct ip_addr {
  u32_t addr[4];
};

int module_init();

//#define LWIP_MAJOR 0X38
static const driver_ops_t lwip_drv_fops;

#ifdef __DLL__

CREATE_SPINLOCK( kbd_lock_sem );

extern int lwip_drv_write(int minor, off_t  pos, const void * buf,int count);
extern int lwip_drv_read(int minor, off_t  pos, void * buf,int count); 
extern int lwip_drv_ioctl(int  file, int cmd, void* arg);
extern int lwip_select();
extern int lwip_drv_close(int  file);


static int lwip_drv_open(const char *f, int mode)
{
  return lwip_drv_fops.d_index;
}


#define ALLOC_MAJOR_NUMBER -1

static const driver_ops_t lwip_drv_fops =
{
	d_name:		"socket",
	d_author:	"Esaion",
	d_version:	"v2006-6-27",
	d_index:	ALLOC_MAJOR_NUMBER,
	d_kver:	CURRENT_KERNEL_VERSION,
	open:		lwip_drv_open,
	close:		lwip_drv_close,
	read:		lwip_drv_read,
	write:		lwip_drv_write,
	ioctl:		lwip_drv_ioctl,		
	select:		lwip_select,
//	timeout:1,
};



extern	struct ip_addr * inetaton(char * c);
static struct ip_addr arg_ip, arg_mask, arg_gw; 
extern int lwip_socket_call_hook(int cmd, void* argp);

int strncmp(const char *str1, const char *str2, int len);

int dll_args(struct ip_addr *a, struct ip_addr *b, struct ip_addr *c)
{
	if (a)
	{
		memcpy(a, &arg_ip, sizeof(struct ip_addr ));
	}

	if (b)
	{
		memcpy(b, &arg_gw, sizeof(struct ip_addr ));
	}

	if (c)
	{
		memcpy(c, &arg_mask, sizeof(struct ip_addr ));
	}

}


int dll_main(char **argv)
{
	int retval;
	u32_t i;

	i=0;

	memset(&arg_ip, 0, sizeof(struct ip_addr));
	memset(&arg_gw, 0, sizeof(struct ip_addr));
	memset(&arg_mask, 0, sizeof(struct ip_addr));


	while (argv[i] != (char *)0)
	{
		//printk("arg:%s\n", argv[i]);

		if (strncmp("ip=", argv[i], 3) == 0)
		{
			memcpy(&arg_ip, inetaton(&argv[i][3]), sizeof(struct ip_addr));
		}

		else if (strncmp("gw=", argv[i], 3) == 0)
		{
			memcpy(&arg_gw, inetaton(&argv[i][3]), sizeof(struct ip_addr));
		}

		else if (strncmp("mask=", argv[i], 5) == 0)
		{
			memcpy(&arg_mask, inetaton(&argv[i][5]), sizeof(struct ip_addr));
		}
		else
		{
			//printk("unknow arg %s\n", argv[i]);
		}

		i++;
	}

	retval=kernel_driver_register(&lwip_drv_fops);
		
	if(retval < 0)
	{
	  kprintf("Could not register lwip device\n");
	  return 1;
	}  

	module_init();
	export_lwip_symtab();
	setup_unix_socketcall(lwip_socket_call_hook);
	return 0;
	
}

int dll_version()
{
	kprintf("JICAMA lwip Driver Version 1.1.1!\n");
	
	return 0;
}

void term_threads()
{
#define SIGTERM           15	/* software termination signal from kill */
#define ETHTHREAD_NAME "EthernetIF"
	void *pthread;	

	pthread = find_thread(ETHTHREAD_NAME);	

	if (pthread)
	{
		printf("kill eth thread ...\n");
		sendsig(pthread, SIGTERM);
	}
	pthread = find_thread("TCPthread");	
	if (pthread)
	{
		printf("kill tcp thread ...\n");
		sendsig(pthread, SIGTERM);
	}		
}



int dll_destroy()
{
	term_threads();
	remove_lwip_symtab();
	return 0;
}

#endif

