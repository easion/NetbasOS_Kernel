#include "netbas.h"

int cold=0;

__inline int
pci_set_powerstate(device_t dev, int state)
{
	pci_softc_t *sc = device_get_ivars(dev);
	return sc->bus->pci_set_power_state(sc->dev,state);
    //return PCI_SET_POWERSTATE(device_get_parent(dev), dev, state);
}

 __inline int
pci_get_powerstate(device_t dev)
{
	TRACE_HERE;
    //return PCI_GET_POWERSTATE(device_get_parent(dev), dev);

	return -1;
}


u_int32_t pci_read_config(device_t dev, int reg, int width)
{
	u_int32_t val=0;
	 pci_pos_t* pci_bus;
	pci_softc_t *sc = device_get_ivars(dev);

	 pci_bus = sc->bus;

	 switch (width)
	 {
	 case 1:
		 pci_bus->read_pci_config8( sc->dev, reg ,&val );
	 break;
	 case 2:
		 pci_bus->read_pci_config16( sc->dev, reg ,&val );
	 break;
	 case 4:
		 pci_bus->read_pci_config32( sc->dev, reg ,&val );
	 break; 
	 }

	 return val;
}

#if 0

unsigned char pci_get_class(device_t dev)
{
	pci_softc_t *sc = device_get_ivars(dev);
	return sc->dev->base_class;
}


int pci_get_irq(device_t dev)
{
	pci_softc_t *sc = device_get_ivars(dev);
	return sc->dev->irq;
}
unsigned char pci_get_subclass(device_t dev)
{
	pci_softc_t *sc = device_get_ivars(dev);
	return sc->dev->sub_class;
}

unsigned char pci_get_progif(device_t dev)
{
	pci_softc_t *sc = device_get_ivars(dev);

	printf("pci_get_progif =%x\n",sc->dev->interface);
	return sc->dev->interface;
}



unsigned short pci_get_vendor(device_t dev)
{
	pci_softc_t *sc = device_get_ivars(dev);
	return sc->dev->vendorid;
}

#endif

unsigned long pci_get_devid(device_t dev)
{
	pci_softc_t *sc = device_get_ivars(dev);

	//printf("vendorid = %x %x\n",sc->dev->vendorid,sc->dev->deviceid);
	return sc->dev->deviceid;
}

u32_t pci_get_base_addr(device_t dev, int n)
{
	int i;
	pci_softc_t *sc = device_get_ivars(dev);

	for (i=0; i<6; i++)
	{
		if (sc->dev->base[i])
		{
			break;
		}
	}

	printf("pci_get_base_addr:i=%d sc->dev->base[i]=%x\n",i,sc->dev->base[i]);
	if (i==6)
	{
		return 0;
	}

	return sc->dev->base[i];
}

u32_t pci_get_base_size(device_t dev, int n)
{
	int i;
	pci_softc_t *sc = device_get_ivars(dev);

	

	return sc->dev->size[0];
}

bus_space_tag_t rman_get_bustag(device_t dev)
{
	void *ioh=pci_get_base_addr(dev,0);
	#define B_PAGE_SIZE 4096


	if (ioh>0x80000000)
	{
		// map the registers
		u32_t offset = (long)ioh & (B_PAGE_SIZE - 1);
		void* physicalAddress = ioh - offset;
		size_t mapSize = pci_get_base_size(dev,0);
		//	(mapSize+= + offset		+ B_PAGE_SIZE - 1) & ~(B_PAGE_SIZE - 1);

		printf("map physicalAddress=%x,mapSize=%d\n",physicalAddress,mapSize);

		map_physical_memory_nocache(physicalAddress,mapSize,"EHCI memory mapped registers");

		return BUS_SPACE_MEM;
	}
	else{
		return BUS_SPACE_IO;
	}
}

#if 1
#include "freebsd_pcireg.h"
__inline void
pci_set_command_bit(device_t dev,  uint16_t bit)
{
	uint16_t        command;
	pci_softc_t *sc = device_get_ivars(dev);
	 pci_pos_t* pci_bus;

	 pci_bus = sc->bus;

	pci_bus->read_pci_config16( sc->dev, PCI_COMMAND, &command );
	//command = PCI_READ_CONFIG(dev, child, PCIR_COMMAND, 2);
	command |= bit;
	//PCI_WRITE_CONFIG(dev, child, PCIR_COMMAND, command, 2);

	pci_bus->write_pci_config16( sc->dev, PCI_COMMAND,command );

}

__inline void
pci_clear_command_bit(device_t dev,  uint16_t bit)
{
        uint16_t        command;
	pci_softc_t *sc = device_get_ivars(dev);
	 pci_pos_t* pci_bus;

	 pci_bus = sc->bus;

 	pci_bus->read_pci_config16( sc->dev, PCI_COMMAND, &command );
    //   command = PCI_READ_CONFIG(dev, child, PCIR_COMMAND, 2);
        command &= ~bit;
    //    PCI_WRITE_CONFIG(dev, child, PCIR_COMMAND, command, 2);
	pci_bus->write_pci_config16( sc->dev, PCI_COMMAND,command );
}

int pci_enable_dev(device_t dev)
{
	unsigned short cmd_stat;
	pci_softc_t *sc = device_get_ivars(dev);
	 pci_pos_t* pci_bus;

	 pci_bus = sc->bus;

	 return pci_bus->pci_enable_device(sc->dev);
}

int
pci_enable_busmaster(device_t dev)
{
	//pci_enable_dev(dev);
        pci_set_command_bit(dev,  PCIM_CMD_BUSMASTEREN|PCIM_CMD_MEMEN);
        return (0);
}

int
pci_disable_busmaster(device_t dev)
{
        pci_clear_command_bit(dev,  PCIM_CMD_BUSMASTEREN|PCIM_CMD_MEMEN);
        return (0);
}


#else

int pci_enable_busmaster(device_t dev)
{
	unsigned short cmd_stat;
	pci_softc_t *sc = device_get_ivars(dev);
	 pci_pos_t* pci_bus;

	 pci_bus = sc->bus;

	 pci_bus->pci_enable_device(sc->dev);
	
	
	pci_bus->read_pci_config16( sc->dev, PCI_COMMAND, &cmd_stat );
	/* Enable busmaster */
	pci_bus->write_pci_config16( sc->dev, PCI_COMMAND, 
		cmd_stat | PCI_COMMAND_IO | PCI_COMMAND_MASTER );
}

int pci_disable_busmaster(device_t dev)
{
	unsigned short cmd_stat;
	 pci_pos_t* pci_bus;
	pci_softc_t *sc = device_get_ivars(dev);

	 pci_bus = sc->bus;
	
	
	pci_bus->read_pci_config16( sc->dev, PCI_COMMAND, &cmd_stat );

	cmd_stat &= ~(PCI_COMMAND_IO | PCI_COMMAND_MASTER);
	/* Enable busmaster */
	pci_bus->write_pci_config16( sc->dev, PCI_COMMAND, 
		cmd_stat  );
}
#endif

void pci_write_config(device_t dev, int reg, u_int32_t val, int width)
 {
	 pci_pos_t* pci_bus;
	pci_softc_t *sc = device_get_ivars(dev);

	 pci_bus = sc->bus;

	 switch (width)
	 {
	 case 1:
		 pci_bus->write_pci_config8( sc->dev, reg, val  );
	 break;
	 case 2:
		 pci_bus->write_pci_config16( sc->dev, reg, val  );
	 break;
	 case 4:
		 pci_bus->write_pci_config32( sc->dev, reg, val  );
	 break; 
	 }
 }




void *
usb_alloc_mem(struct bus_dma_tag *tag, u_int32_t size, u_int8_t align_power)
{
	void *p=NULL;
	//TRACE_HERE;

	p = kmalloc(size,0);
	//p=low_alloc(size);
	//printf("align_power %d %d %d p=%p\n",align_power,1<<align_power,size,p);
	return p;
}

void* page_vtophys(void* page_table);
void* virt2phys(void *vaddr);

bus_size_t
usb_vtophys(void *ptr, u_int32_t size)
{
	//TRACE_HERE;
	//return virt2phys(ptr);
	return (ptr);
}

void
usb_free_mem(void *ptr, u_int32_t size)
{
	//printf("usb_free_mem %p %d\n", ptr,size);
	kfree(ptr);
	//low_free(ptr,size);
	//TRACE_HERE;
}

void	microtime (struct timeval *tv)
{
	if (tv)
	{
		tv->tv_sec = get_unix_time();
		tv->tv_usec = (startup_ticks()%hz)*10000;
	}
	//TRACE_HERE;
}

int usb_kthread_create1(void *func, void *arg, void**ret, char *unused, char *name)
{
	int t;

	t = new_kernel_thread(name,func,arg);
	if (t>0)
	{
		ret[0] = t;
	}
	else
	ret[0] = 0;
	//tid_t new_kernel_thread(char *name, daemon_thread_t* pth, void *arg)

	return 0;
}


void kthread_exit(int ret)
{
	thread_exit(current_thread(),ret);
}

void
bcopy (const void *src, void *dest, size_t len)
{
  if (dest < src)
    {
      const char *firsts = (const char *) src;
      char *firstd = (char *) dest;
      while (len--)
        *firstd++ = *firsts++;
    }
  else
    {
      const char *lasts = (const char *)src + (len-1);
      char *lastd = (char *)dest + (len-1);
      while (len--)
        *lastd-- = *lasts--;
    }
}

void
bzero (void *to, size_t count)
{
  memset (to, 0, count);
}


/*********************************************************/

void    __callout_reset(struct __callout *hld, u_int32_t timeout, void (*func)(void *),void* para )
{
	timeout = TICKS_TO_MS(timeout);
	init_timer(&hld->timer, func, para);
 restart_timer(&hld->timer, timeout);
}

int     __callout_stop(struct __callout *hld){
remove_timer(&hld->timer);
}

int     __callout_init_mtx(struct __callout *hld, void *mtx, long lockflag){
	//mtx_init(mtx);
	hld->mtx = mtx;
 init_timer(&hld->timer, NULL, NULL);
 install_timer(&hld->timer,5000);
}

/*********************************************************/
int tsleep(device_t dev,long flag,char *name,time_t ticks)
{
	ticks=TICKS_TO_MS(ticks);
	thread_wait(current_thread(),ticks);
	return 0;
}

#include <drv/stdarg.h>


driver_ops_t *
make_dev(driver_ops_t *devsw, int minornr, uid_t uid, gid_t gid, int perms, const char *fmt, ...)
{
	char name[256];
	driver_ops_t *ops = kmalloc(sizeof(driver_ops_t),0);

	va_list args;
	int rv;


	va_start(args, fmt);
	rv = vsprintf(name, 256, fmt, args);
	va_end(args);
	
	//return rv;

	memcpy(ops,devsw,sizeof(driver_ops_t));

	ops->d_name = kmalloc(256,0);
	strncpy(ops->d_name,name,256);
	ops->d_index = (uid << 8) | (minornr&0xff);

	if(kernel_driver_register(ops)!=OK){
		kfree(ops);
		return NULL;
	}



	return ops;
}

int uiomove(char *from,int len,char *dest)
{
	memcpy(dest,from,len);
	return 0;
}
int     msleep(void *chan, struct mtx *mtx, int pri, const char *wmesg,
            int timo)
{
	int err;
	thread_wait_t *wait_queue=chan;
	time_t t=0;

	t=TICKS_TO_MS(timo);

	if (!timo)
	//if (0)
	{
		err = thread_sleep_on(wait_queue,NULL);
	}
	else{
		err = thread_sleep_on(wait_queue,&t);
	}

	//err=0;
	//TRACE_HERE;
	return err;
}

void wakeup(void* chan)
{
	thread_wait_t *wait_queue=chan;
	thread_wakeup(wait_queue);
	//TRACE_HERE;
}

void minit(void* chan)
{
	thread_wait_t *wait_queue=chan;
	thread_waitq_init(wait_queue);
	//TRACE_HERE;
}


void selwakeuppri(void *sel, long type)
{
	TRACE_HERE;
}
/*********************************************************/
#if 0

void
mtx_lock(struct mtx *mp)
{
       // KASSERT(mp->state == 0, ("mutex already locked"));
	   if (mp->state != 0)
	   {
		   kprintf("mtx_lock (mutex already locked)\n");
	   }
        mp->state = 1;
		mp->cpu_flag = spinlock_disable(&mp->lock);
}


void
mtx_unlock(struct mtx *mp)
{
       // KASSERT(mp->state == 1, ("mutex not locked"));
	   if (mp->state != 1)
	   {
		   kprintf("mtx_unlock (mutex not locked)\n");
	   }
        mp->state = 0;
		spinunlock_enable(&mp->lock,mp->cpu_flag);
}

void
mtx_assert(struct mtx *mp, int flag)
{
        if (flag == MA_OWNED) {
	   if (mp->state != 1)
		   kprintf("mtx_assert(MA_OWNED) not true\n");
                //KASSERT(mp->state == 1, ("mtx_assert(MA_OWNED) not true"));
        }
}
#endif


void
mtx_init(struct mtx *m, const char *name, const char *type, int opts)
{
	m->state = 0;
	spinlock_init(&m->lock,name);
	m->cpu_flag=0;
}

/*
 * Remove lock `m' from all_mtx queue.  We don't allow MTX_QUIET to be
 * passed in as a flag here because if the corresponding mtx_init() was
 * called with MTX_QUIET set, then it will already be set in the mutex's
 * flags.
 */
void
mtx_destroy(struct mtx *m)
{
}


void *malloc_static(int size, int mode, int flags)
{
	return kmalloc(size,0);
}

void free_static(void *ptr, int type)
{
	kfree(ptr);
}

/*********************************************************/
void *device_get_dma_tag(device_t dev)
{
	//TRACE_HERE;
	return 0;
}

#define readb(addr) (*(volatile unsigned char *) (addr))
#define readw(addr) (*(volatile unsigned short *) (addr))
#define readl(addr) (*(volatile unsigned int *) (addr))

#define writeb(b,addr) (*(volatile unsigned char *) (addr) = (b))
#define writew(b,addr) (*(volatile unsigned short *) (addr) = (b))
#define writel(b,addr) (*(volatile unsigned int *) (addr) = (b))

void bus_space_write_1(bus_space_tag_t iot, bus_space_handle_t ioh,u_int32_t reg, u_int8_t val)
{
	unsigned io_addr = ioh;

	//printf("%s %p reg %x val %x\n",__FUNCTION__,io_addr,reg,val);
	if(iot == BUS_SPACE_MEM){
		writeb(val,io_addr + reg);
	}
	else
		outb (val,io_addr + reg);

	//TRACE_HERE;
}

void bus_space_write_2(bus_space_tag_t iot, bus_space_handle_t ioh,u_int32_t reg, u_int16_t val)
{
	unsigned io_addr = ioh;

	if(iot == BUS_SPACE_MEM){
		writew(val,io_addr + reg);
	}
	else
		outw (val,io_addr + reg);

		//printf("%s %p reg %x val %x\n",__FUNCTION__,io_addr,reg,val);
		//outw (io_addr + reg,val);
	//TRACE_HERE;
}

void bus_space_write_4(bus_space_tag_t iot, bus_space_handle_t ioh,u_int32_t reg, u_int32_t val)
{
	unsigned io_addr = ioh;

	if(iot == BUS_SPACE_MEM){
		writel(val,io_addr + reg);
	}
	else
		outl (val,io_addr + reg);

	//printf("%s %p reg %x val %x\n",__FUNCTION__,io_addr,reg,val);
	//outl (io_addr + reg, val);
	//TRACE_HERE;
}

u_int8_t bus_space_read_1(bus_space_tag_t iot, bus_space_handle_t ioh,u_int32_t reg)
{
	unsigned io_addr = ioh;
	u_int8_t val=0;

	if(iot == BUS_SPACE_MEM){
		val = readb(io_addr + reg);
	}
	else
		val = inb (io_addr + reg);
	return val;
}

u_int16_t bus_space_read_2(bus_space_tag_t iot, bus_space_handle_t ioh,u_int32_t reg)
{
	unsigned io_addr = ioh;
	u_int16_t val=0;

	if(iot == BUS_SPACE_MEM){
		val = readw(io_addr + reg);
		//printf("bus_space_read_4 %p, value  %x\n",io_addr + reg,val);
	}
	else
	 val=inw (io_addr + reg);

	return val;
}

u_int32_t bus_space_read_4(bus_space_tag_t iot, bus_space_handle_t ioh,u_int32_t reg)
{
	unsigned io_addr = ioh;
	u_int32_t val=0;

	if(iot == BUS_SPACE_MEM){
		val= readl(io_addr + reg);

		//printf("bus_space_read_4 %p, value  %x\n",io_addr + reg,val);
	}
	else	
		val= inl (io_addr + reg);
	return val;
}



