
#include <arch/x86/io.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <jicama/devices.h>
#include <errno.h>
#include <string.h>
#include <arch/x86/traps.h>
#include <assert.h>
#include <jicama/linux.h>
#include <net/inet.h>
//#include <net/socket.h>
#include <net/net.h>

int handler_execve_ptrace(thread_t *pthread, regs_t *reg);

int vfs_select(int nfds, fd_set *readset, fd_set *writeset, 
	fd_set *exceptset, struct timeval *timeout);

//linux二进制兼容模块



__local int UnixCall( setup )()
{
	//kprintf("%s 000\n",__FUNCTION__);
	int n = load_kernel_dlls("/device");

	//kprintf("%s called\n",__FUNCTION__);
	return n;
}

__local int UnixCall( exit)(int code)
{
	thread_t *pthread = current_thread();

	//kprintf("%s called %s\n",__FUNCTION__,pthread->name);

    proc_exit((code), pthread,0);
	//kprintf("%s done\n",__FUNCTION__);
	return 0;
}


__local int UnixCall( close)(int fd)
 {
	int err;

	err=vfs_close(fd);
	return err;
}

int UnixCall( read)(int fd, void * buf, int count)
{
	int chars = 0;
	char*  mybuf = (unsigned char *)(current_proc_vir2phys((long)buf));

	mem_writeable((void*)mybuf, count);

	chars = vfs_read(fd,mybuf,count, 0);

	/*if(chars>0)
	kprintf("%s: fd%d, buf at %s count %d-%d\n",
		__FUNCTION__,fd, (char*)mybuf, count,chars);*/
	return chars;
}

//_syscall6 ( __ptr_t, addr, size_t, len, int, prot,
//	   int, flags, int, fd, __off_t, offset);
//为应用程序分配大内存
u32_t UnixCall( mmap)(unsigned long *pbuf)
{
	unsigned long base, addr=0;
	unsigned long len, limit, off;
	int prot, flags, fd;

	pbuf = current_proc_vir2phys(pbuf);

	if (!pbuf)
	{
		return -1;
	}

	addr = (unsigned long)	peek32((pbuf));	/* user address space*/
	len = (size_t)		peek32(pbuf+1);	/* nbytes of mapping */
	prot = (int)		peek32(pbuf+2);	/* protection */
	flags = (int)		peek32(pbuf+3);	/* mapping type */
	fd = (int) 		peek32(pbuf+4);	/* object to map */
	off = (unsigned long)	peek32(pbuf+5);	/* offset in object */


	kprintf("mmap called(%x,%x, %x,%x,%x,%x)\n",addr,len,prot,flags, fd,off);
	return _sbrk(len);
}

// long sys_munmap(unsigned long addr, size_t len) 
u32_t UnixCall( munmap)(unsigned long addr, size_t len)
{
	kprintf("munmap called(addr 0x%x,len %d)\n",addr,len);
	return -1;
}

__local int UnixCall( write)(int fd, void * pbuf, int count)
{
	int bytes;

	//trace("%s() on fd%d\n",__FUNCTION__, fd);

	if (!pbuf){
		return 0;
	}

	if (count<=0 || count >= 0x7fffffff)
	{
		//kprintf("%s() error on fd%d ,cnt=%d\n",__FUNCTION__, fd,count);
		return -1;
	}

	char *buf = ( char *)(current_proc_vir2phys(pbuf));

	mem_writeable((void*)buf, count);
	bytes = vfs_write(fd,buf,count, count);
	//kprintf("%s() fd%d bytes=%x pbuf=%x bytes=%d\n",__FUNCTION__, fd, count,pbuf, bytes);
	return bytes;
}

__local int UnixCall( lseek)(int fd, off_t off, int whence)
{
	return  lseek(fd,off,whence);
}

__local int UnixCall( fork)(regs_t reg)
{
	int err;
	bool sv;
	thread_t *threadnew;
	thread_t *pthread = current_thread();
	regs_t *regs = &reg;

	if(!IS_USER_TASK(proc_number(THREAD_SPACE(pthread)))){
		return -1;
	}

	lock_schedule(&sv);
	

	//get a pcb
	threadnew = create_new_process( regs->eip, regs->user_esp, (char *)pthread->name);

	if(!threadnew){
		set_schedule(sv);
		return err;
	}
	ASSERT(THREAD_SPACE(threadnew)->p_asid >= INIT_PROC);


	err= do_clone (pthread, threadnew, regs);
	trace("clone is  err = %d\n",err);

	thread_ready(threadnew);

	set_schedule(sv);

	return err;
}


__local   int UnixCall( execve)(u32_t file)
{
	int ret;
	bool sv;
	regs_t *regs = (regs_t *) &file;
	char **__argv=(char **)regs->ecx;
	char **__env=(char **)regs->edx;
	thread_t *pthread = current_thread();

	if (!file)
	{
		return -1;
	}


	/*can not run in kernel mode*/
	 if (!inusermode(regs->cs)){
         kprintf("can not execve from kernel mode.");
		 return EAGAIN;
	 }

	file = (char *)current_proc_vir2phys(regs->ebx);
	//kprintf("unix execve %s",file);
	//kprintf("eip=0x%x cs=0x%x\n", regs->eip,regs->cs);

	lock_schedule(&sv);
	if(__argv)__argv=(char **)(current_proc_vir2phys(__argv));
	if(__env)__env=(char **)(current_proc_vir2phys(__env));

	trace("try execve %s\n", file);
	ret = do_execve(file, __argv, __env);

	if (ret==OK)
	{
		regs->cs = USER_CS; /*setup the cs*/
		regs->eip = pthread->tss.eip;  /*program entry*/
		regs->user_esp = pthread->tss.esp;  /*task's new stack position*/
		trace ("kernel: execve at 0x%x-0x%x\n",regs->eip, regs->user_esp);
	}

	set_schedule(sv);

	handler_execve_ptrace(pthread, regs);

	//unsigned char code = *(unsigned char*)(USER_SPACE_ADDR+regs->eip);

	//kprintf("kexec on entry %p,%p,%x,%x\n", regs->eip,regs->user_esp,code, read_user_byte(regs->eip));	
	return ret;
}

int UnixCall( dup2)(int oldfd, int newfd)
{
	return dup2(oldfd, newfd);
}

int UnixCall( dup)(int fd)
{
	return dup(fd);
}

int UnixCall( fcntl)(int fd, int cmd, u32_t arg)
{	
	//kprintf("%s() %d %d %d\n", __FUNCTION__,fd,arg,cmd);
	return vfs_fcntl(fd,cmd,arg);
}



 int UnixCall( ioctl)(int fd, int cmd, void* arg,int size) 
{
	u32_t type;
	dev_prvi_t*dp= fs_get_prvi_data(fd,&type);
	if (!dp)
	{
		//kprintf("fs_get_prvi_data empty\n");
		return -1;
	}

	if (S_ISSOCK(type))
	{
	//kprintf("%s() fd%d cmd=%x arg=%x\n",__FUNCTION__, fd, cmd,arg);
		arg = current_proc_vir2phys(arg);
		return ioctlsocket(dp,cmd,arg,size);
	}
	else if (S_ISCHR(type) || S_ISBLK(type))
	{
		//kprintf("fs_get_prvi_data ioctl\n");
	return  dev_ioctl(dp, cmd, arg,size, false);
	}

	return -1;
}

int UnixCall( poll)(struct pollfd *fds, unsigned int nfds, int timeout)
{
	int err;
	struct pollfd *pfds=current_proc_vir2phys(fds);

	kprintf("%s() called with %d fds\n", __FUNCTION__,nfds);

	err = poll(pfds,nfds,timeout);
	return err;
}


int UnixCall( select)(int fd, fd_set *readfds, fd_set *writefds)
{
	int err;
	regs_t* r = (regs_t *) &fd;
	fd_set *exceptfds=(fd_set *)r->esi;
	struct timeval *timeout=(struct timeval *)r->edi;

	if(readfds)readfds=current_proc_vir2phys(readfds);
	if(writefds)writefds=current_proc_vir2phys(writefds);
	if(exceptfds)exceptfds=current_proc_vir2phys(exceptfds);
	if(timeout)timeout=current_proc_vir2phys(timeout);

	
	err = vfs_select(fd, readfds, writefds,exceptfds,timeout);
	if (err<0)
	{
	kprintf("%s() [Task: %d] fd=%d err=%s\n",__FUNCTION__,current_thread()->tid,fd,err);
	}

	return err;
}

int UnixCall( writev)(int fd, const struct iovec * vector, int		  count)
{
	int i;
	struct socket * sock=NULL;
	u32_t fd_type=0;
	struct iovec iovec[UIO_MAXIOV];

	if (count > UIO_MAXIOV)
	{
		return E2BIG;
	}

	sock = fs_get_prvi_data(fd,&fd_type);
	if (!sock){	
		return EBADF;
	}

	if (!S_ISSOCK(fd_type)){
		return EBADF;
	}

	vector = current_proc_vir2phys(vector);

	for (i=0; i<count; i++)
	{
		iovec[i].iov_len = vector[i].iov_len;
		iovec[i].iov_base = current_proc_vir2phys(vector[i].iov_base);
	}
	//kprintf("sendv with count=%d\n",count);
	//return sendto(A6_SENDTO_TYPE(int, void*, int, int, 
	//	   struct sockaddr*, int));
	return sendv(sock,iovec,count);
}

int UnixCall( readv)(int fd, const struct iovec * vector, int		  count)
{
	int i;
	struct socket * sock=NULL;
	u32_t fd_type=0;
	struct iovec iovec[UIO_MAXIOV];

	if (count > UIO_MAXIOV)
	{
		return E2BIG;
	}

	sock = fs_get_prvi_data(fd,&fd_type);
	if (!sock){	
		return EBADF;
	}

	if (!S_ISSOCK(fd_type)){
		return EBADF;
	}

	vector = current_proc_vir2phys(vector);
	for (i=0; i<count; i++)
	{
		iovec[i].iov_len = vector[i].iov_len;
		iovec[i].iov_base = current_proc_vir2phys(vector[i].iov_base);
	}
	
	return recvv(sock,iovec,count);
}


int UnixCall( getdents)(int fd, struct dirent_t * de, int nbyte)
{
	int error;
	struct dirent_t  *pde = ( struct dirent_t  *)(current_proc_vir2phys(de));
	vfs_dirent_t vfs_de;	


	kprintf("getdir with %d bytes\n",nbyte);

	if (!de){
		return -1;
	}

	error = vfs_readdir(fd,&vfs_de,nbyte);

	if (error){
		return error;
	}

	mem_writeable((void*)pde, nbyte);
	int reclen = offsetof(struct dirent_t,name) + strlen(vfs_de.l_long_name) + 1;
	pde->ino = vfs_de.d.l_ino; //1表示已经被删除的文件
	pde->off = vfs_de.d.l_off;
	pde->reclen = reclen;
	strncpy(pde->name,vfs_de.l_long_name,nbyte) ;
	kprintf("getdents %s size %d inode %d %d\n",vfs_de.l_long_name,vfs_de.l_item_size, vfs_de.d.l_ino,vfs_de.d.l_off);
	//return sizeof(struct dirent_t);
	return vfs_de.l_item_size;
}

struct itimerval {
	struct timeval it_interval;	/* timer interval */
	struct timeval it_value;	/* current value */
};

#define	ITIMER_REAL		0
#define	ITIMER_VIRTUAL		1
#define	ITIMER_PROF		2



 int UnixCall( setitimer)(int  __which,
		      const struct itimerval * __new,
		      struct itimerval * __old)
 {
	 int seconds;
	 int usec;
	 int msec;

	 if (__which!=ITIMER_REAL)
	 {
		 kprintf("setitimer: err __which\n");
	 }

	 if (__new)
	 {
		 __new = current_proc_vir2phys(__new);
		 seconds = __new->it_value.tv_sec;
		 usec = __new->it_value.tv_usec;
	 }
	 else{
		 kprintf("setitimer: err param\n");
		 return -1;
	 }

	 if (__old)
	 {
		 kprintf("setitimer:need fix __old param\n");
	 }

	 msec = seconds*1000+usec/1000;

	 set_alarm(msec);

	 kprintf("setitimer:sleep %d\n",msec);
	 return 0;
 }

 int UnixCall( alarm)(int seconds)
{
	set_alarm(seconds*1000);
	return 0;
}

int UnixCall( time)(time_t * t)
{
	time_t now=0 ;
	int *punixtime = current_proc_vir2phys(t);


	//kprintf("unix time = %x\n",now);
	now = get_unixtime(punixtime);
	//SYSCALL_NOT_SUPPORT;
	return now;
}

int UnixCall( kill)(pid_t pid, int signo)
{
	kprintf("killpid() pid%d with %d\n", pid,signo);
	return  killpid(pid, signo);
}

 int UnixCall( wait4)(pid_t pid, int * status, int opt,void* *rusage)
{
	pid_t id;
	int *pstat = current_proc_vir2phys(status);

	id = do_waitpid(pid, pstat,opt);

	//kprintf("waitpid%d status: %x  returned %x\n",pid, *pstat,id);
	return id;
}


int UnixCall( uname)(linux_utsname_t * u)
{
	int error;
	linux_utsname_t utsname =		{ 
		UTS_SYSNAME,
		UTS_NODENAME, 
		UTS_RELEASE, 
		UTS_VERSION, 
		UTS_MACHINE, 
		UTS_DOMAINNAME
		};

	linux_utsname_t * _uname_ =  ( linux_utsname_t *)current_proc_vir2phys((long)u);
	
	if (error = mem_writeable(_uname_, sizeof(*u)))
		return error;
	*_uname_ = utsname; 	
	return 0;
}

int UnixCall( open)(const char * pathname, int flags, mode_t mode) 
{
	char *filename;
	int val =0;

	filename = ( char *)current_proc_vir2phys(pathname);

	trace("linux_open %s  flags%x mode %x  with val %x\n", filename,flags, mode,val);
	val = vfs_open(filename,flags,mode);
	return val;
}

int UnixCall(creat)(const char * pathname, int mode)
{
	char *filename;

	filename = ( char *)current_proc_vir2phys(pathname);
	kprintf("%s : %s\n", __FUNCTION__, filename);
	return vfs_creat(filename, mode);
}

int UnixCall(unlink)(const char * pathname)
{
	char *filename;

	filename = ( char *)current_proc_vir2phys(pathname);
	//kprintf("%s : %s\n", __FUNCTION__, filename);
	return vfs_unlink(filename);
}


int UnixCall(symlink)(const char * pathname,const char * pathnew)
{
	char *filename;
	char *filenew;

	filename = ( char *)current_proc_vir2phys(pathname);
	filenew = ( char *)current_proc_vir2phys(pathnew);
	//kprintf("%s : %s,%s\n", __FUNCTION__, filename,filenew);
	return vfs_link(filename,filenew);
}


__local inline int sigaction_from_linux(struct sigaction *to, struct  sigact_t * from)
{
	to->sa_handler = from->func;
	to->sa_mask = from->mask;
	to->sa_flags = from->flags;
	to->restorer = from->restorer;
	return 0;
}

__local inline int sigaction_to_linux(struct  sigact_t*to, struct   sigaction* from)
{
	to->func = from-> sa_handler;
	to->mask = from->sa_mask;
	to->flags = from-> sa_flags;
	to->restorer = from->restorer;
	return 0;
}


int UnixCall( sigaction)(int signo, struct  sigact_t * act, struct sigact_t * oact)
{
	int err;
	struct sigact_t *newact, *oldact;
	struct sigaction _newact, _oldact;

	if (!act)
	{
		return -1;
	}

	//SYSCALL_NOT_SUPPORT;

	newact = (struct sigaction *)(current_proc_vir2phys(act));
	sigaction_from_linux(&_newact,newact);

	err = sigaction_setup(signo,&_newact,&_oldact);

	if (oact)
	{
		oldact = (struct sigaction *)(current_proc_vir2phys(oact));
		sigaction_to_linux(oldact,&_oldact);
	}

	return err;
}

 int UnixCall( signal)(int signo, sigfunc_t fuc, sigfunc_t fuc2)
{
	return signal_setup(signo,fuc,fuc2);
}

 int UnixCall( waitpid)(pid_t pid, int * status,int opt)
{
	int *my_status;
	pid_t cpid;
	regs_t* r = (regs_t *) &pid;

	my_status =(int *) current_proc_vir2phys(status);
	cpid = do_waitpid(pid,my_status,opt);
	enable();
	r->eflags |=EFLAGS_IF;
	//kprintf("waitpid done\n");
	return cpid;
}

 int UnixCall( chdir)(const char* path)
{
	char *dirname = (char*)current_proc_vir2phys(path);
	return chdir(dirname);
}

struct timespec
  {
    time_t tv_sec;		/* Seconds.  */
    long int tv_nsec;		/* Nanoseconds.  */
  };

int UnixCall(nanosleep) (const struct timespec *__requested_time,
		      struct timespec *__remaining)
{
	int err;
	time_t msec;
	struct timespec *req;

	if(__requested_time){
		req = current_proc_vir2phys(__requested_time);
		msec = req->tv_sec*1000 +  req->tv_nsec/1000000;
	}
	else{
		msec = INFINITE;
	}

	//kprintf("%s %d called\n",__FUNCTION__,msec);
	err = thread_wait(current_thread(), msec);
	//schedule();
	return 0;
}

 int UnixCall( getpid)()
{
	pid_t id;
	int sc;
	
	lock_schedule(&sc);
	id = current_thread()->tid;
	set_schedule(sc);
	return id;
}

__local int UnixCall( nosys)(long ebx)
{
	regs_t* r = (regs_t *) &ebx;
	kprintf("%s()  syscall Not Implented(No.%d)\n",__FUNCTION__, r->eax_org);
	return ENOSYS;
}

int UnixCall( getrlimit)(unsigned int resource, struct rlimit *rlim);
int UnixCall( setrlimit)(unsigned int resource, struct rlimit *rlim);


 

//void*	UnixCall_table[]={
 int (*UnixCall_table[NR_LINUX_CALLS]) ()  = {
	UnixCall(setup),		/* 0 */
	UnixCall(exit),
	UnixCall(fork),
	UnixCall(read),
	UnixCall(write),
	UnixCall(open)	,	/* 5 */
	UnixCall(close),
	UnixCall(waitpid),
	UnixCall(creat),
	EmptyCall(link)	,	/* 9 */
	UnixCall(unlink)	,	/* 10 */
	UnixCall(execve),
	UnixCall(chdir),
	UnixCall(time),
	EmptyCall(mknod),
	EmptyCall(chmod)	,	/* 15 */
	EmptyCall(lchown),
	UnixCall(brk),
	EmptyCall(oldstat),
	UnixCall(lseek),
	UnixCall(getpid)	,	/* 20 */
	UnixCall(mount),
	UnixCall(umount),
	UnixCall(setuid),
	UnixCall(getuid),
	UnixCall(stime)		,/* 25 */
	UnixCall(ptrace),
	UnixCall(alarm),
	EmptyCall(oldfstat),
	UnixCall(pause),
	UnixCall(utime)	,	/* 30 */
	EmptyCall(stty),
	EmptyCall(gtty),
	UnixCall(access),
	UnixCall(nice),
	EmptyCall(ftime)	,	/* 35 */
	UnixCall(sync),
	UnixCall(kill),
	UnixCall(rename),
	UnixCall(mkdir),
	UnixCall(rmdir)	,	/* 40 */
	UnixCall(dup),
	UnixCall(pipe),
	UnixCall(times),
	EmptyCall(prof),
	UnixCall(brk),		/* 45 */
	UnixCall(setgid),
	UnixCall(getgid),
	UnixCall(signal),
	UnixCall(geteuid),
	UnixCall(getegid),		/* 50 */
	EmptyCall(acct),
	EmptyCall(phys),
	EmptyCall(lock),
	UnixCall(ioctl),
	UnixCall(fcntl)	,	/* 55 */
	EmptyCall(mpx),
	UnixCall(setpgid),
	EmptyCall(ulimit),
	EmptyCall(oldolduname),
	UnixCall(umask)	,	/* 60 */
	UnixCall(chroot),
	EmptyCall(ustat),
	UnixCall(dup2),
	UnixCall(getppid),
	UnixCall(getpgrp),		/* 65 */
	UnixCall(setsid),
	UnixCall(sigaction),
	UnixCall(sgetmask),
	UnixCall(ssetmask),
	UnixCall(setreuid),		/* 70 */
	UnixCall(setregid),
	UnixCall(sigsuspend),
	UnixCall(sigpending),
	EmptyCall(sethostname),
	UnixCall(setrlimit),		/* 75 */
	UnixCall(getrlimit),
	UnixCall(getrusage),
	UnixCall(gettimeofday),
	UnixCall(settimeofday),
	EmptyCall(getgroups)	,	/* 80 */
	EmptyCall(setgroups),
	EmptyCall(select),
	UnixCall(symlink),
	EmptyCall(oldlstat),
	UnixCall(readlink)	,	/* 85 */
	EmptyCall(uselib),
	EmptyCall(swapon),
	EmptyCall(reboot),
	EmptyCall(readdir)	,	/* obsolete */
	UnixCall(mmap),		/* 90 */
	UnixCall(munmap),
	EmptyCall(truncate),
	EmptyCall(ftruncate),
	EmptyCall(fchmod),
	EmptyCall(fchown),		/* 95 */
	EmptyCall(getpriority),
	EmptyCall(setpriority),
	EmptyCall(profil),
	EmptyCall(statfs),
	EmptyCall(fstatfs)	,	/* 100 */
	UnixCall(ioperm),
	UnixCall(socketcall),
	EmptyCall(linux_log),
	UnixCall(setitimer),
	EmptyCall(getitimer)	,	/* 105 */
	UnixCall(stat),
	UnixCall(lstat),
	UnixCall(fstat),
	EmptyCall(olduname),
	UnixCall(iopl)	,	/* 110 */
	EmptyCall(vhangup),
	EmptyCall(idle),
	EmptyCall(vm86old),
	UnixCall(wait4),
	EmptyCall(swapoff)	,	/* 115 */
	EmptyCall(linux_info),
	EmptyCall(ipc),
	EmptyCall(fsync),
	UnixCall(sigreturn),
	UnixCall(clone)	,	/* 120 */
	EmptyCall(setdomainname),
	UnixCall(uname),
	EmptyCall(modify_ldt),
	EmptyCall(adjtimex),
	EmptyCall(mprotect)	,	/* 125 */
	UnixCall(sigprocmask),
	EmptyCall(create_module),
	EmptyCall(init_module),
	EmptyCall(delete_module),
	EmptyCall(get_kernel_syms),	/* 130 */
	EmptyCall(quotactl),
	EmptyCall(getpgid),
	UnixCall(fchdir),
	EmptyCall(bdflush),
	EmptyCall(linux_fs)	,	/* 135 */	
	UnixCall(personality),
	EmptyCall(afs_linux_call),
	EmptyCall(setfsuid),
	EmptyCall(setfsgid),
	EmptyCall(llseek),		/* 140 */
	UnixCall(getdents),		/* uclibc need this */
	UnixCall(select),
	EmptyCall(143),
	EmptyCall(144),
	UnixCall(readv),
	UnixCall(writev),
	EmptyCall(L___syscall_getsid),
	EmptyCall(148),
	EmptyCall(149),
	EmptyCall(150),
	EmptyCall(151),
	EmptyCall(152),
	EmptyCall(153),
	EmptyCall(154),
	EmptyCall(155),
	EmptyCall(156),
	EmptyCall(157),
	EmptyCall(158),
	EmptyCall(159),
	EmptyCall(160),
	EmptyCall(161),
	UnixCall(nanosleep),
	EmptyCall(163),
	EmptyCall(164),
	EmptyCall(165),
	EmptyCall(vm86),
	EmptyCall(167),
	UnixCall(poll),
	EmptyCall(169),
	EmptyCall(170),
	EmptyCall(171),
	EmptyCall(172),
	EmptyCall(173),
	UnixCall(sigaction)    , /* rt_sigaction */
	UnixCall(sigprocmask)  , /* rt_sigprocmask */
	EmptyCall(176),
	EmptyCall(177),
	EmptyCall(178),
	EmptyCall(179),
	EmptyCall(180),
	EmptyCall(181),
	UnixCall(chown),
	UnixCall(getcwd),
	EmptyCall(184),
	EmptyCall(185),
	EmptyCall(186),
	EmptyCall(187),
	EmptyCall(188),
	EmptyCall(189),
	UnixCall(fork),	/* gcc, linux_tem(3) need vfork */
	UnixCall(getrlimit),
	EmptyCall(192),
	EmptyCall(193),
	EmptyCall(194),
	UnixCall(stat64),
	UnixCall(lstat64),
	EmptyCall(197),
	EmptyCall(198),
	EmptyCall(199),
};

