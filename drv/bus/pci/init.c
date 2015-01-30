
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------

//  some code from: my.execpc.com/~geezer/os

#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/proc_entry.h>

#define PCI_UNUSED 0
#define PCI_USED 1
#define NR_PCI_DEV 128

struct pci_dev *sys_dev[NR_PCI_DEV]; /*PCI devices struct*/
extern int pci_dev_nr;
static int last_pci_used=0;
void pci_init( void );
void pnp_init(void);

extern int (*pci_read_config_byte)(pci_state_t *state,
		unsigned reg, unsigned char *value);
extern int (*pci_read_config_word)(pci_state_t *state,
		unsigned reg, unsigned short *value);
extern int (*pci_read_config_dword)(pci_state_t *state,
		unsigned reg, unsigned long *value);

extern int (*pci_write_config_byte)(pci_state_t *state,
		unsigned reg, unsigned char value);
extern int (*pci_write_config_word)(pci_state_t *state,
		unsigned reg, unsigned short value);
extern int (*pci_write_config_dword)(pci_state_t *state,
		unsigned reg, unsigned long value);


void pci_dump(struct pci_dev *pcidev);
struct pci_dev *pci_dev_lookup(unsigned long basec, unsigned long subc);
struct pci_dev* pci_lookup(unsigned int start, unsigned short vendorid, unsigned short deviceid );

#define PCI_CONFIG_OP(bwl,act, type) \
static int local_pci_##bwl##_config_##act(pci_state_t *state,unsigned reg, unsigned type value) { \
	return pci_##bwl##_config_##act(state, reg, value);\
} 

PCI_CONFIG_OP(read, byte, char *)
PCI_CONFIG_OP(read, word, short *)
PCI_CONFIG_OP(read, dword, long *)
PCI_CONFIG_OP(write, byte, char )
PCI_CONFIG_OP(write, word, short )
PCI_CONFIG_OP(write, dword, long )

void * pci_get_root_dev();

pci_pos_t pcibus_ops=
{
	get_pci_info: NULL,
	read_pci_config8:local_pci_read_config_byte,
	read_pci_config16:local_pci_read_config_word,
	read_pci_config32:local_pci_read_config_dword,
	write_pci_config8:local_pci_write_config_byte,
	write_pci_config16:local_pci_write_config_word,
	write_pci_config32:local_pci_write_config_dword,

	pci_read_irq:pci_read_irq,
	pci_find_capability:pci_find_capability,
	pci_enable_device:pci_enable_device,
	pci_find_cfg:pci_find_cfg,
	pci_set_power_state:pci_set_power_state,
	pci_set_master:pci_set_master,
	pci_probe:pci_probe,
	pci_read_bases:pci_read_bases,
	pci_dump:pci_dump,
	pci_dev_lookup:pci_dev_lookup,
	pci_lookup:pci_lookup,
		pci_root_dev:pci_get_root_dev,
};

void* pci_bus_get_hooks( int nVersion )
{
	return( &pcibus_ops );
}

static struct bus_man pcibus =
{
	bus_name: "PCI32",
	bus_hooks:pci_bus_get_hooks,
};

int dll_main(char **argv)
{
	pci_init();
	busman_register( &pcibus );
	return 0;
}

int dll_version()
{
	printk("JICAMA PCI Driver Version 0.01!\n");
	return 0;
}

int dll_destroy()
{
	busman_unregister( &pcibus );
	//remove_dll_table("pci32.dll");
	return 0;
}

struct pci_dev* pci_lookup(unsigned int start, unsigned short vendorid, unsigned short deviceid )
{
    int i;

    for ( i = start; i < pci_dev_nr; i++ )
	  if ( sys_dev[ i ]->vendorid == vendorid && sys_dev[ i ]->deviceid == deviceid )	
      return sys_dev[ i ];

	return NULL;
}
extern int pci_scan_dev(void);

struct pci_dev *pcidec_slot()
{
	int i;
	for (i=0; i<NR_PCI_DEV; i++)
	{
		if(sys_dev[i]  == NULL){
			sys_dev[i]  =(struct pci_dev *)kmalloc(sizeof(struct pci_dev ),0);
			if(!sys_dev[i])NULL;
			sys_dev[i]->flags = PCI_USED;
			pci_dev_nr++;
			//printk("item%d ", i);
			return sys_dev[i];
		}
	}
	return (struct pci_dev *)0;
}

void pci_free_slot(struct pci_dev *dev)
{
	memset(dev, 0, sizeof(struct pci_dev));
	dev->flags = PCI_UNUSED;
}

struct pci_dev *pci_dev_lookup(unsigned long basec, unsigned long subc)
{
	int i;
	for (i=0; i<NR_PCI_DEV; i++)
	{
		if(!sys_dev[i] ||sys_dev[i]->flags==PCI_UNUSED)
			continue;
		if(sys_dev[i]->base_class == basec && sys_dev[i]->sub_class == subc){
			//printk("item%d ", i);
			return sys_dev[i];
		}
	}
	return (struct pci_dev *)0;
}

void pci_dump(struct pci_dev *pcidev)
{
	int i;

    if(pcidev->base_class == 12 && pcidev->sub_class == 3) /* USB */
			printk("USB Ctrl:");
		if(pcidev->base_class == 0x02 && pcidev->sub_class == 0x00) /* net work */
			printk("Net card:");
	printk("\n %s %s Base class:%d ",
		pci_showvendor( pcidev->vendorid ),
		pci_showclass(pcidev->base_class, pcidev->sub_class),
		pcidev->base_class);

	//pci_showvendor( pcidev->vendorid );
	//pci_showclass(pcidev->base_class, pcidev->sub_class);
	//printk("\n Base class:%d ",pcidev->base_class);
	printk(" Sub class:%d \n",pcidev->sub_class);
	printk(" Vendorid:%d ",pcidev->vendorid);
	printk(" Deviceid:%d ",pcidev->deviceid);
	printk(" Bus:%d ",pcidev->bus);
	printk(" Dev_fn:%d\n ",pcidev->dev_fn);

	printk("[IRQ:%d]",pcidev->irq);

	for (i=0; i<6; i++)
	{
		if(pcidev->base[i])
		printk(" IOBase%d:%x Size:%x \n ", i, pcidev->base[i], pcidev->size[i]);
	}

	  printk("Rombase-%x, Size-%u,  \n", pcidev->rom_base, pcidev->rom_size );
}

int pci_proc (char *buf, int len, void *pf)
{
	int i;
	struct pci_dev *pcidev;

  pprintf(pf, "\nVendorid deviceid irq base0 size0 calss \n");

	for (i=0; i<NR_PCI_DEV; i++)
	{
		pcidev = sys_dev[i];

		if(!pcidev ||pcidev->flags==PCI_UNUSED)
			continue;
	#if 0
    if(pcidev->base_class == 12 && pcidev->sub_class == 3) /* USB */
			pprintf(pf, "USB Ctrl:");
	if(pcidev->base_class == 0x02 && pcidev->sub_class == 0x00) /* net work */
		pprintf(pf,"Net card:");
	#endif
	/*pprintf(pf,"%4d %4d %4d %4d %4d %8d %8d %4d %10s %15s\n",
		pcidev->sub_class,pcidev->vendorid,
		pcidev->deviceid, pcidev->bus,pcidev->irq,
		pcidev->base[0],pcidev->rom_base, pcidev->rom_size,
		pci_showclass(pcidev->base_class),
		pci_showvendor( pcidev->vendorid )
		);	*/
	pprintf(pf,"%04x %04x %02d %08x %08d %-15s\n",
		pcidev->vendorid ,
		pcidev->deviceid,
		pcidev->irq,
		pcidev->base[0],
		pcidev->size[0],
		pci_showclass(pcidev->base_class)
		);	
	}
	return 0;
}

struct proc_entry e_pciproc = {
	name: "pci32",
	write_func: pci_proc,
	read_func: NULL,
};

static int pci_open(const char *fdevname, int mode,dev_prvi_t* devfp)
{
	//kprintf("fb open ok\n");
	return 0;
}

static int pci_close(dev_prvi_t* devfp)
{
	return 0;
}

int pci_ioctl(dev_prvi_t* devfp, int cmd, void* args,int size, int from_kernel)
{	
	int ret=0;
	unsigned short *addr=(unsigned short*)args;
	
	//kprintf("cmd is %d\n", cmd);

	if (!from_kernel)
	{
		addr = (void*)cur_addr_insystem((size_t)args);
	}

	switch (cmd)
	{
        case 0x1001:
			//pci_scan_dev();   
            //strcpy( (char *) addr, "vbe.accelerant");
            return 0;
			break;
		case 0x1002:
		{
			struct device *dev;

			kprintf("bus_generic_attach ..\n");
			dev = pci_get_root_dev();
			bus_generic_attach(dev);
		}
		break;
		default:
			break;
	}
	return ret;
}

 driver_ops_t pci_bus_dev =
{
	d_name:		"pcibus",
	d_author:	"easion",
	d_kver:	CURRENT_KERNEL_VERSION,
	d_version:	KMD_VERSION,
	d_index:	ALLOC_MAJOR_NUMBER,
	open:		pci_open,
	close:		pci_close,
	ioctl:		pci_ioctl,		
};


void pci_init( void )
{
	int i;

 	for (i=0; i<NR_PCI_DEV; i++){
		sys_dev[ i ] = NULL;
	}

	//kprintf("kernel_driver_register pci ..\n");
 
	if(kernel_driver_register(&pci_bus_dev)!=OK)
		kprintf("register pci device failed");
  
	pci_devtree_init();
	register_proc_entry(&e_pciproc);
    pci_scan_dev();   
}

