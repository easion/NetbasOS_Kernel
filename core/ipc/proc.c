
#include <jicama/system.h>
#include <jicama/fs.h>
#include <string.h>
#include <jicama/proc_entry.h>
#include <jicama/module.h>
#include <jicama/devices.h>

//static LIST_HEAD(listhead, proc_entry) proc_entry_head;

static struct proc_entry *proc_entry_head = NULL;

struct proc_entry * proc_get_header()
{
	return proc_entry_head;
}



int register_proc_entry(struct proc_entry *proc)
{
	int error = 0;
	struct proc_entry *tmp;

	if (!proc)
	{
		return -1;
	}

	proc->next = NULL;
	proc->time = startup_ticks();

	if (!proc_entry_head)
	{
		proc_entry_head = proc;
	}
	else{
		//foreachproc(proc_entry_head,tmp);
		//tmp->next = proc;
		proc->next = proc_entry_head;
		proc_entry_head = proc;
	}
	return error;
}

int unregister_proc_entry(struct proc_entry *proc)
{
	int error = -1;
	struct proc_entry *tail = proc_entry_head;
	struct proc_entry *tmp;

	foreachlist(proc_entry_head,tmp)
	{
		if (proc == tmp)
		{
			tail->next = tmp->next;
			tmp->next = NULL;
			error = 0;
		}
		else{
			tail = tmp;
		}
	}

	return error;
}

int read_proc(char *name, char *buf, int len)
{
	struct proc_entry *tmp;
	struct proc_entry write_data;

	if (!name)
	{
	//kprintf("read_proc %s here\n", "null");
		return 0;
	}

	foreachlist(proc_entry_head,tmp)
	{
		if (!tmp->write_func)
		{
			kprintf("null wrt\n");
			continue;
		}
	//kprintf("read_proc %s here\n", tmp->name);
		if (strcmp(tmp->name,name) == 0)
		{
			memcpy(&write_data,tmp,sizeof(struct proc_entry));

			write_data.read_size = len;
			write_data.read_pos = 0;
			write_data.read_buffer = buf;

			//kprintf("call wrt\n");
			return tmp->write_func(buf,len,&write_data);
		}
	}

	kprintf("%s not found\n",name);
	return -1;
}

int write_proc(char *name, char *buf, int len)
{
	struct proc_entry *tmp;
	struct proc_entry write_data;

	if (!name)
	{
		return 0;
	}

	foreachlist(proc_entry_head,tmp)
	{
		if (!tmp->read_func)
		{
			continue;
		}
		if (strcmp(tmp->name,name) == 0)
		{
			memcpy(&write_data,tmp,sizeof(struct proc_entry));

			write_data.read_size = len;
			write_data.read_pos = 0;
			write_data.read_buffer = buf;
			return tmp->read_func(buf,len,&write_data);
		}
	}
	return -1;
}


int ls_proc(char *buf, int len)
{
	int count = 0, c;
	struct proc_entry *tmp;

	foreachlist(proc_entry_head,tmp)
	{
		if (count >= len)
		{
			break;
		}
		c = sprintf(buf+count, "%s  ", tmp->name);
		//kprintf("cnt %d name %s\n", c, tmp->name);
		count += c;
	}
	strcat(buf, "\n\r");
	//kprintf("lsproc %d bytes\n", count);
	return count+2;
}


static int kernel_param_proc(char *buf, int len,struct proc_entry *pf)
{
	pprintf(pf, "%s\n\r", get_kernel_command());
  return 0;
}

struct proc_entry cmdlineproc = {
	name: "kernel_param",
	write_func: kernel_param_proc,
	read_func: NULL,
};

static int uptime_proc(char *buf, int len,struct proc_entry *pf)
{
	time_t sec=startup_ticks()/HZ;
	time_t min=(sec/60);
	int hours=(sec/3600);
	int day=(sec/(3600*24));

	pprintf(pf, "Uptime: %d Day %02d Hours %02d Min %02d Sec\n\r",day,hours,min,sec%60 );
  return 0;
}

struct proc_entry uptimeproc = {
	name: "uptime",
	write_func: uptime_proc,
	read_func: NULL,
};

struct proc_entry msgportproc = {
	name: "msgport",
	write_func: msg_proc,
	read_func: NULL,
};

struct proc_entry modproc = {
	name: "module",
	write_func: moduledump,
	read_func: NULL,
};

struct proc_entry klogproc = {
	name: "klog",
	write_func: read_log,
	read_func: NULL,
};

struct proc_entry taskproc = {
	name: "threads",
	write_func: procdump,
	read_func: NULL,
};

struct proc_entry dllproc = {
	name: "symtab",
	write_func: write_dll,
	read_func: NULL,
};


struct proc_entry memproc = {
	name: "memory",
	write_func: mem_dump,
	read_func: NULL,
};

struct proc_entry devproc = {
	name: "devices",
	write_func: devices_dump,
	read_func: NULL,
};


struct proc_entry irqproc = {
	name: "irq",
	write_func: irq_proc_dump,
	read_func: NULL,
};

void proc_entry_init(void)
{
	proc_entry_head = NULL;
	trace ("proc_entry_init ...\n");
	register_proc_entry(&dllproc);
	register_proc_entry(&devproc);
	register_proc_entry(&irqproc);
	register_proc_entry(&memproc);
	register_proc_entry(&modproc);
	register_proc_entry(&klogproc);
	register_proc_entry(&taskproc);
	register_proc_entry(&msgportproc);
	register_proc_entry(&cmdlineproc);
	register_proc_entry(&uptimeproc);
	trace ("proc_entry_init ok...\n");
}



__local int proc_disk_open(char *f, int mode,dev_prvi_t* devfp);
__local int proc_disk_close(dev_prvi_t* devfp);
__local int proc_disk_read(dev_prvi_t* devfp, off_t  offset, char * buf,int count);
__local int proc_disk_write(dev_prvi_t* devfp, off_t  offset, char * buf,int count);

__local int proc_disk_read(dev_prvi_t* devfp, off_t  offset, char * buf,int count)
{	
   return count;
}

__local int proc_disk_write(dev_prvi_t* devfp, off_t  offset, char * buf,int count)
{
   return count;
}

__local int proc_disk_ioctl(dev_prvi_t* devfp, int cmd, void* args,int size, int bfrom)
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


__local const driver_ops_t ops =
{		
	d_name:		"proc",
	d_author:	"easion",
	d_kver:	CURRENT_KERNEL_VERSION,
	d_version:	KMD_VERSION,
	d_index:	ALLOC_MAJOR_NUMBER,
	open:		proc_disk_open,
	close:		proc_disk_close,
	read:		proc_disk_read,
	write:		proc_disk_write,
	ioctl:		proc_disk_ioctl,		
};


__local int proc_disk_open(char *f, int mode,dev_prvi_t* devfp)
{
	return 0;
}

__local int proc_disk_close(dev_prvi_t* devfp)
{
	return 0;
}

int proc_disk_init()
{
	if(kernel_driver_register(&ops)!=OK){
		return -1;
	}
	return 0;
}



/*
**
*/
__local void dll_info_close(file_t *f)
{
}

/*
**
*/
__local void dll_info_flush(file_t *f)
{
}

/*
**
*/
__local int dll_info_write(file_t *f, const void *buf, int len)
{
	return 0;
}

/*
**
*/
__local int dll_info_read(file_t *f, void *buf, int count)
{
	 dump_devicesinfo(buf, count);
	 return strlen(buf)+1;
}

/*
**
*/
__local  void dll_info_open(file_t *f)
{
	return 0;
}


/*
**
*/
file_t* dll_info_init()
{
	file_t *f = kfs_slot();

	if (!f)
	{
		return NIL_FLP;
	}

	f->size = 512;
	
	strncpy( f->name , "dinfo",FPATH_LEN);

	f->attr = TEXT_FILE;
	f->open_kfs = &dll_info_open;
	f->close_kfs = &dll_info_close;
	f->read_kfs = &dll_info_read;
	f->write_kfs = &dll_info_write;
	f->flush_kfs = &dll_info_flush;

	proc_disk_init();
	return f;
}

////////////////////////




