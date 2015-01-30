/*
 **     (R)Jicama OS
**      system call
**     Copyright (C) 2003,2004 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/paging.h>
#include <jicama/linux.h>
#include <jicama/syscall.h>
#include <jicama/msgport.h>
#include <arch/x86/io.h>
#include <jicama/spin.h>
#include <jicama/utsname.h>

#include <arch/x86/traps.h>
#include <arch/x86/clock.h>
#include <jicama/devices.h>
#include <jicama/proc_entry.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <assert.h>


__asmlink int SysCall( thread_get_info)(regs_t *reg);
__asmlink int SysCall( set_thread_priority)(regs_t *reg);
__asmlink int SysCall( set_thread_name)(regs_t *reg);
__asmlink int SysCall( thread_yield)(regs_t *arg);
__asmlink int SysCall( thread_new)(regs_t *arg);
__asmlink int SysCall( thread_join)(regs_t *reg);
__asmlink int SysCall( thread_suspend)(regs_t *reg);
__asmlink int SysCall( thread_resume)(regs_t *reg);

__asmlink int SysCall( thread_create_semaphore)(regs_t *reg);
__asmlink int SysCall( thread_delete_semaphore)(regs_t *reg);
__asmlink int SysCall( thread_lock_semaphore_x)(regs_t *reg);
__asmlink int SysCall( thread_unlock_semaphore_x)(regs_t *reg);

__asmlink int exec_8086(long vm_addr, long code_from, int sz);
__asmlink void exec_8086_test(long vm_addr);
__asmlink unsigned long get_unix_time(void);

void do_reboot(void);
int unmap_large_dir(u32_t form_addr);

__local int SysCall( sys_userlog)(regs_t *reg)
{
	long error = reg->ebx;
	char* buf = current_proc_vir2phys(reg->ecx);
	long len = reg->edx;

	return syslog(LOG_USER+error,"%s", buf);
}

int SysCall( msgport)(regs_t *reg);
int SysCall( msgsend)(regs_t *reg);
int SysCall( msgpend)(regs_t *reg);
int SysCall( post_thread_message)(regs_t *reg);
int SysCall( get_thread_message)(regs_t *reg);
#   define DIO_BYTE	  'b'	/* byte type values */
#   define DIO_WORD	  'w'	/* word type values */
#   define DIO_LONG	  'l'	/* long type values */

int SysCall( get_thread_message)(regs_t *reg)
{
	int ret;
	thread_t *pthread;
	tid_t t = reg->ebx;

	if (t == 0)
	{
		pthread = current_thread();
	}
	else{
		pthread = find_thread_byid(t);
	}
	ret = get_thread_message(pthread, (void *)current_proc_vir2phys(reg->ecx), 
	reg->edx,reg->esi);

	return ret;
}

int SysCall( post_thread_message)(regs_t *reg)
{
	int ret;
	int msg_type = (int)reg->edi;
	thread_t *pthread;

	pthread = find_thread_byid(reg->ebx);
	ret = post_thread_message(pthread, (void *)current_proc_vir2phys(reg->ecx), reg->edx);


	return ret;

}


__local int SysCall( outport)(regs_t *reg)
{
	int err=0;
	long port = reg->ebx;
	long value = reg->ecx;
	long type = reg->edx;

	switch (type)
	{
	case DIO_BYTE:
		outb(port,value);
		break;
	case DIO_WORD:
		outw(port,value);
		break;
	case DIO_LONG:
		outl(port,value);
		break;
	default:
		err=-1;
		break;	
	}
	return err;
}

__local int SysCall( inport)(regs_t *reg)
{
	int err=0;
	long port = reg->ebx;
	long *value = reg->ecx;
	long type = reg->edx;
	int data=0;

	if (value)
	{
		value = current_proc_vir2phys(value);
	}

	switch (type)
	{
	case DIO_BYTE:
		data=inb(port);
		break;
	case DIO_WORD:
		data=inw(port);
		break;
	case DIO_LONG:
		data=inl(port);
		break;
	default:
		err=-1;
		break;	
	}

	if(!err){
		*value = data;
	}

	return err;
}

__local int SysCall( ptrace)(regs_t *reg)
{
	int req=reg->ebx;
	char * buf=(char *)(reg->ecx);
	int size=reg->edx;
	void *regs=(void*)reg->esi;
	char *proc_name = regs;

	if(inusermode(reg->cs)){
		buf = ( char *)(current_proc_vir2phys(buf));
	}

	if (proc_name){
		proc_name = ( char *)(current_proc_vir2phys(proc_name));
	}

	mem_writeable((void*)buf, size);

	switch (req)
	{
	case 0:
		return procdump(buf, size);
		break;
	case 1: /*sym info*/
		return write_dll(buf, size);
		break;
	case 2:/*dll info*/
		return moduledump(buf, size);
		break;
	case 3:
		break;
	case 4:
	break;
	case 5:
		return read_log(buf,size);
	break;
	case 6:
		return ls_proc(buf,size);
	break;
	case 7:
		return read_proc(proc_name, buf,size);
	break;
	case 8:
		return write_proc(proc_name, buf,size);
	break;
	default:
		kprintf("%S():warn: invalid argument %d\n",__FUNCTION__, req);
		break;
	}
	return ENOSYS;
}

__local int SysCall( pause) (regs_t *reg)
{
	//time_t tout=current_proc()->p_timeout;
	thread_wait(current_thread(), INFINITE);
	return 0;
}



__local int	 SysCall( sigresume)  (regs_t *args)
{
	thread_t *thread = current_thread();

	kprintf("%s() called\n",__FUNCTION__);

	//resume = (regs_t*)(current_proc_vir2phys(args->ebx));
	thread->signal_mask = args->ecx;

	sigdelset(thread, SIGKILL);
	sigdelset(thread, SIGSTOP);

	if (!thread->regs.eip)
	{
		panic("error??");
	}

	memcpy(args, &thread->regs, sizeof(regs_t));
	//args->cs = USER_CS;
	//args->user_ss = args->ds = args->es = args->fs = args->gs = USER_DS;	

	load_fpu_state(thread->fpu_state_info);
	return args->eax;/*return value*/
}


__local int	SysCall( signal)  (regs_t *regs)
{
	int no=regs->ebx;
	__sighandler_t fuc=(__sighandler_t )regs->ecx;
	__sighandler_t fuc2=(__sighandler_t )regs->edx;

  if(!inusermode(regs->cs))return -1;
	return signal_setup(no,fuc,fuc2);
}

__local int	SysCall( sigaction)  (regs_t *regs)
{
	int no=regs->ebx;
	struct sigaction *a;
	struct sigaction *b;

  if(!inusermode(regs->cs))return -1;

	  a = (struct sigaction *)(current_proc_vir2phys(regs->ecx));
	  b = (struct sigaction *)(current_proc_vir2phys(regs->edx));

	return sigaction_setup(no,a,b);
}

__local int SysCall( sgetmask) ()
{
  return current_thread()->signal_mask;
}

__local int SysCall( ssetmask) (regs_t *reg)
{
	int newmask=reg->ebx;
	int old = current_thread()->signal_mask;

	current_thread()->signal_mask = newmask;
	sigdelset(current_thread(), SIGKILL);

	return old;
}




__public time_t get_unixtime(time_t *t)
{
	unsigned long n = get_unix_time();

	if((t) != NULL)
	    *t = n;

	return n;
}

 void do_gettime(struct tm*  toget);

__local int	SysCall( clock)  (regs_t *regs)
{
	struct tm *t;

	if(inusermode(regs->cs))
	  t= (struct tm *)(current_proc_vir2phys(regs->edx));
	else 
	 t= (struct tm *)(regs->edx);


	if (regs->edx)	{do_gettime(t);	}
	get_unixtime((time_t *)current_proc_vir2phys(regs->ebx));

	return startup_ticks(); 
}




__local int SysCall( run_v86) (regs_t *regs)
{
	long raddr=current_proc_vir2phys(regs->ebx);
	long size=regs->ecx;
	long daddr=regs->edx;
	extern int exec_8086(long vm_addr, long code_from, int sz);

	//kprintf("raddr=%x\n", raddr);
	exec_8086(daddr, raddr, size);
	return 0;
}


int	 SysCall( nosys)(int nr)
{
	kprintf("%s()  Not Implented (NETBAS No.%d) \n",__FUNCTION__, nr);
	return ENOSYS;
}


__local int	 SysCall( reboot ) (regs_t *regs)
{
	do_reboot();
	return OK;
}



__asmlink void read_real_memory(char *buf, int sz);
__asmlink void fill_real_memory(char *buf, int sz);
__asmlink void real_memory_addr(u32_t *addr, int *sz);



__local int	SysCall( realint )(regs_t *regs)
{
	int x, n=regs->ecx;
	vm86regs_t *_in, *_out; 

  if(inusermode(regs->cs)){
	_out=(vm86regs_t*)(current_proc_vir2phys(regs->edx));
	}else{
	_out=(vm86regs_t *)regs->edx;
	}

	_in=(vm86regs_t *)current_proc_vir2phys(regs->ebx);

   x=do_int86(n, _in,_out);
   return x;
}


__local int	SysCall( real_read)(regs_t *regs)
{
	int s=(int)regs->ecx;
	read_real_memory((char *)current_proc_vir2phys(regs->ebx),s);
	return s;
}


__local int	SysCall( real_fill)(regs_t *regs)
{
	int s=(int)regs->ecx;
	fill_real_memory((char *)current_proc_vir2phys(regs->ebx),s);
	return s;
}

__local int	SysCall( real_address)(regs_t *regs)
{
	u32_t *a=(u32_t*)current_proc_vir2phys(regs->ebx);
	int *b;

	b=(int *)(regs->ecx);

  if(inusermode(regs->cs)){
		b==(int*)(current_proc_vir2phys(regs->ecx));
	}
	 real_memory_addr(a, b);
	 return 0;
}

/*readdir*/

__local int	 SysCall( readdir)(regs_t *regs)
{
	int fd=regs->ebx;
    u8_t *buf = (unsigned char *)(regs->ecx);
    int count = regs->edx;


  if(inusermode(regs->cs))
	  buf = (unsigned char *)(current_proc_vir2phys(regs->ecx));

	mem_writeable((void*)buf, count);

	return vfs_readdir(fd,buf,count);
}



/*
** 
*/
__local int	 SysCall( rename)(regs_t *regs)
{
	int err;

	err=rename((char*)current_proc_vir2phys(regs->ebx), (char*)current_proc_vir2phys(regs->ecx));

	return err;
}


__asmlink int load_dll_file(char *file, char *argv[]);
__asmlink int remove_dll_file(char *file);

/*
** 
*/
__local int	 SysCall( dllin)(regs_t *regs)
{
	int err=-1;

	char *path=(char *)current_proc_vir2phys(regs->ebx);
	char **env=(char **)current_proc_vir2phys(regs->ecx);

	err= load_dll_file(path, env);
	return err;
}

/*
** 
*/
__local int	 SysCall( dllout)(regs_t *regs)
{
	char *path=(char *)current_proc_vir2phys(regs->ebx);
	return 	remove_dll_file(path);
}




/*
** 
*/
int SysCall( get_module)(regs_t *regs)
{
	return -1;
}



int SysCall( vm_get_region_info)(regs_t *regs)
{
	int id =  (regs->ebx); 
	int err=0;
	void *info =  current_proc_vir2phys(regs->ecx);
	err=shm_get_info(id, info);
	return err;
}

//fixme!!!
//获取最低可映射的地址
int SysCall( get_vm_region_param)(regs_t *regs)
{
	unsigned long *addr =  current_proc_vir2phys(regs->ebx); 	
	unsigned int *size =  current_proc_vir2phys(regs->ecx);
	proc_t *cur_task = current_proc();

	unsigned kaddr,ksize;

	kaddr = USER_MMAP_ADDR;
	ksize = USER_MMAP_SIZE;



	if (addr){
		//kprintf("get_vm_region_param begin addr on %p %x bytes\n",kaddr,ksize);
		*addr = kaddr;
		}
	if (size){	*size = ksize;}
	return 0;
}

int SysCall( remap_vm_region)(regs_t *regs)
{
	int err;
	int id =  (regs->ebx); 	
	void* area =  (regs->ecx); 	

	err=shm_remap_area(id,area);
	if (err)
	{
		return err;
	}
	return 0;
}


int SysCall( attach_vm_region)(regs_t *regs)
{
	int err;
	int id=regs->ebx;	
	void **addr =  current_proc_vir2phys(regs->ecx); 	
	char* name=current_proc_vir2phys(regs->edx);	

	err=shm_clone_area(id,addr,name);

	//kprintf("%s(): id = %d addr=%d\n", __FUNCTION__, id, err);
	
	return err;
}

//映射内核空间到用户空间
int SysCall( vm_create_region)(regs_t *regs)
{
	int err;
	void** user_addr=(u32_t)current_proc_vir2phys(regs->ebx); //用户空间地址
	char *name=(char*)current_proc_vir2phys(regs->ecx); //系统的IO地址
	int size= regs->esi; //大小
	long flags= regs->edx;	



	err=shm_create(name,user_addr,size,flags);
	//kprintf("%s(): size = %d addr=%x\n", __FUNCTION__, size, *user_addr);

	return err;
}



int SysCall( vm_delete_region)(regs_t *regs)
{
	int ret;
	u32_t id=(u32_t)regs->ebx;
	ret = shm_release(id);
	return 0;
}

int SysCall( open)(regs_t *regs) 
{
	char *filename;
	int val =0;
	char * pathname=(char*)regs->ebx;
	int flags=regs->edx;
	stat_t buf;
	mode_t mode=regs->ecx;

	filename = ( char *)current_proc_vir2phys(pathname);

	val = fs_stat(filename, &buf);

	if (val == 0)
	{
		if (!S_ISDIR(buf.mode) ){
			//kprintf("linux_stat not SIFDIR %o\n",buf.mode);
			return -1;
		}
	}
	else
		return val;
	//kprintf("linux_open %s  flags%x mode %x  with val %x\n", filename,flags, mode,val);
	val = vfs_open(filename,mode,flags);
	return val;
}




int SysCall( thread_cannel)(regs_t *reg);

int SysCall( dynamic_symbol)(regs_t *regs)
{
	int ret=0;
	u32_t id=(u32_t)regs->ebx;
	proc_t *cur_task = current_proc();
	char *name = ( char *)current_proc_vir2phys(regs->ecx);
	void **fn = ( void**)current_proc_vir2phys(regs->edx);
	void *ptr;

	ptr = dynamic_symbol(cur_task,id,name);

	if (!ptr)
	{
		ret = -1;
	}

	if(fn)*fn = ptr;
	return ret;
}


int SysCall( get_dynamic_module_info)(regs_t *regs)
{
	int ret;
	u32_t id=(u32_t)regs->ebx;
	proc_t *cur_task = current_proc();
	char *info = ( char *)current_proc_vir2phys(regs->ecx);

	ret = get_dynamic_module_info(cur_task,id,info);
	return ret;
}

int SysCall( unload_library)(regs_t *regs)
{
	int ret;
	u32_t id=(u32_t)regs->ebx;
	proc_t *cur_task = current_proc();

	ret = unload_library(cur_task,id);
	return ret;
}

int SysCall( load_library)(regs_t *regs)
{
	int ret;
	char* path=(char*)current_proc_vir2phys(regs->ebx);
	long flags=(long)(regs->ecx);
	proc_t *cur_task = current_proc();

	ret = load_library(cur_task,path,flags);
	return ret;
}

int SysCall( get_dynamic_dependencies)(regs_t *regs)
{
	int ret;
	u32_t id=(u32_t)regs->ebx;
	proc_t *cur_task = current_proc();
	void **info = ( char *)current_proc_vir2phys(regs->ecx);
	int init = regs->edx;

	ret = get_dynamic_dependencies(cur_task,id,info, init);
	return ret;
}

/*
** 
*/
 __local int (*syscall_vector[NR_CALLS]) ()  = {
	SysCall( nosys),		/*  0 = unused	*/
	SysCall( nosys),	/*  1 = exit	*/
	SysCall( nosys),	/*  2 = fork	*/
	SysCall( outport),	/*  3 = read	*/
	SysCall( inport),	/*  4 = write	*/
	SysCall( open),	/*  5 = open	*/
	SysCall( nosys),	/*  6 = close	*/
	SysCall( nosys ),		/*  7 = wait	*/
	SysCall( nosys),	/*  8 = creat	*/
	SysCall( sys_userlog),//_link,	/*  9 = link	*/

	SysCall( nosys), /* 10 = unlink	*/
	SysCall( nosys),		/* 11 = waitpid	*/
	SysCall( nosys),	/* 12 = chdir	*/
	SysCall( clock),	/* 13 = time	*/
	SysCall( nosys),	/* 14 = mknod	*/
	SysCall( nosys),	/* 15 = chmod	*/
	SysCall( nosys),	/* 16 = chown	*/
	SysCall( nosys),		/* 17 = break	*/
	SysCall( nosys),	/* 18 = stat	*/
	SysCall( nosys),	/* 19 = lseek	*/

	SysCall( nosys),		/* 20 = getpid	*/
	SysCall( nosys),	/* 21 = mount	*/
	SysCall( nosys),	/* 22 = umount	*/
	SysCall( nosys),		/* 23 = setuid	*/
	SysCall( nosys),		/* 24 = getuid	*/
	SysCall( nosys),	/* 25 = stime	*/
	SysCall( ptrace),		/* 26 = ptrace	*/
	SysCall( nosys),		/* 27 = alarm	*/
	SysCall( nosys),	/* 28 = fstat	*/
	SysCall( pause),		/* 29 = pause	*/

	SysCall( nosys),	/* 30 = utime	*/
	SysCall( nosys),		/* 31 = (stty)	*/
	SysCall( nosys),		/* 32 = (gtty)	*/
	SysCall( nosys),	/* 33 = access	*/
	SysCall( nosys),		/* 34 = (nice)	*/
	SysCall( nosys),		/* 35 = (ftime)	*/
	SysCall( nosys),	/* 36 = sync	*/
	SysCall( nosys),		/* 37 = kill	*/
	SysCall( nosys),	/* 38 = rename	*/
	SysCall( nosys),	/* 39 = mkdir	*/

	SysCall( nosys),	/* 40 = rmdir	*/
	SysCall( nosys),		/* 41 = dup	*/
	SysCall( nosys),	/* 42 = pipe	*/
	SysCall( nosys),	/* 43 = times	*/
	SysCall( nosys),		/* 44 = (prof)	*/
	SysCall( nosys),		/* 45 = brk(linux)	*/
	SysCall( nosys),		/* 46 = setgid	*/
	SysCall( nosys),		/* 47 = getgid	*/
	SysCall( nosys),		/* 48 = (signal)*/
	SysCall( nosys),		/* 49 = unused	*/

	SysCall( nosys),		/* 50 = unused	*/
	SysCall( nosys),		/* 51 = (acct)	*/
	SysCall( readdir),		/* 52 = (phys)	*/
	SysCall( nosys),		/* 53 = (lock)	*/
	SysCall( nosys),	/* 54 = ioctl	*/
	SysCall( nosys),	/* 55 = fcntl	*/
	SysCall( nosys),		/* 56 = (mpx)	*/
	SysCall( run_v86),		/* 57 = unused	*/
	SysCall( nosys),		/* 58 = SysCall( uname	*/
	SysCall( nosys),	/* 59 = execve	*/

	SysCall( load_library),	/* 60 = umask	*/
	SysCall( unload_library),	/* 61 = chroot	*/
	SysCall( dynamic_symbol),	/* 62 = setsid	*/
	SysCall( get_dynamic_module_info),		/* 63 = getpgrp	*/
	SysCall( get_dynamic_dependencies),		/*64 getppid*/
	SysCall( nosys), //	/* 65 = UNPAUSE	*/
	SysCall( dllin), 	/* 66 = unused  */
	SysCall( dllout),	/* 67 = REVIVE	*/
	SysCall( sgetmask),		/* 68 = 	sgetmas*/
	SysCall( ssetmask),		/* 69 = SET */

	SysCall( signal),		/* 70 = signal */
	SysCall( sigaction),		/* 71 = SIGACTION */
	SysCall( nosys),		/* 72 = SIGSUSPEND */
	SysCall( nosys),		/* 73 = SIGPENDING */
	SysCall( nosys),		/* 74 = SIGPROCMASK */
	SysCall( sigresume),		/* 75 = SIGRETURN */
	SysCall( reboot),		/* 76 = REBOOT */
	SysCall( nosys),		/*77 */
	SysCall( nosys),  /*78*/
	SysCall( nosys), /*79*/

	SysCall( nosys), /*80*/
	SysCall( nosys), /*81*/
	SysCall( realint), /*82*/
	SysCall( nosys), /*83*/
	SysCall( real_address),/*84*/
	SysCall( real_fill), /*85*/
	SysCall( real_read),/*86*/
	SysCall( nosys),		/*87 T */
	SysCall( get_module),		
	SysCall( nosys),		/*89 T */	

	SysCall( nosys),		/*90 T */	
	SysCall( msgsend),		/*91 T */	
	SysCall( msgpend),		/*92 T */	
	SysCall( msgport),		/*93 T */	
	SysCall( get_thread_message),		/*94 T */	
	SysCall( post_thread_message),		/*95 T */	
	SysCall( thread_create_semaphore),		/*96 T */	
	SysCall( thread_delete_semaphore),		/*97 T */	
	SysCall( thread_lock_semaphore_x),		/*98 T */	
	SysCall( thread_unlock_semaphore_x),		/*99 T */	

	SysCall( get_vm_region_param),		/*100 T */	
	SysCall( remap_vm_region),		/*101 T */	
	SysCall( attach_vm_region),		/*102 T */	
	SysCall( vm_delete_region),		/*103 T */	
	SysCall( vm_get_region_info),		/*104 T */	
	SysCall( vm_create_region),		/*105 T */	
	SysCall( thread_get_info),		/*106 T */	
	SysCall( set_thread_name),		/*107 T */	
	SysCall( thread_new),		/*108 T */	
	SysCall( thread_cannel),		/*109 T */	

	SysCall( thread_yield),		/*110 T get current thread id */	
	SysCall( thread_join),		/*111 T */	
	SysCall( thread_resume),		/*112 T */	
	SysCall( thread_suspend),		/*113 T */	
	SysCall( set_thread_priority),		/*114 T */	
	SysCall( nosys),		/*113 T */	
};


/*
** 
*/
void fill_userdef_call(void)
{
	int i;

	for (i=128; i<NR_CALLS; i++)
		syscall_vector[i]=SysCall( nosys);
}

/*
** 
*/
int syscall_setup(int callnr, jicama_syscall_t fun)
{
	if (callnr<128||callnr>NR_CALLS){return FALSE;}
	if (!fun){return FALSE;}
	if (syscall_vector[callnr]!=SysCall(nosys) ){return FALSE;}

	syscall_vector[callnr]=fun;
	return TRUE;
}

/*
** 
*/
int syscall_remove(int callnr)
{
	if (callnr<128||callnr>NR_CALLS){return FALSE;}
	if (syscall_vector[callnr]==SysCall(nosys) ){return FALSE;}

	syscall_vector[callnr]=SysCall( nosys);
	return TRUE;
}


/*
** 
*/
__public int do_system_call(regs_t reg)
{
	int ret;
	int call_nr = reg.eax;
	regs_t *arg_p = (void*)&reg;
	int (*syscall_fn)();


/*if syscall not exist*/
	if(call_nr > NR_CALLS || call_nr < 0){
		return ENOSYS;
	}
	

	syscall_fn = syscall_vector[call_nr];

	if (syscall_fn == SysCall( nosys))
	{
		arg_p = (void*)call_nr;
	}
  
	ret = (*syscall_fn)(arg_p);


	process_signal(&reg, current_thread());
	 return ret; /*system call is ok!*/
}

