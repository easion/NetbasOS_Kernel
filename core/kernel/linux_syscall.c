
#include <arch/x86/io.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <jicama/linux.h>
#include <errno.h>
#include <string.h>
#include <arch/x86/traps.h>
#include <assert.h>

int UnixCall( ptrace)(long request, long pid, long addr, long data);

int UnixCall( lstat64)(const char * pathname, stat64_t * stat)
{
	char *file = ( char *)(current_proc_vir2phys((long)pathname));
	char *buf = ( char *)(current_proc_vir2phys((long)stat));
	return vfs_stat64(file, buf);
}

int UnixCall( stat64)(const char * pathname, stat64_t * stat)
{
	char *file = ( char *)(current_proc_vir2phys((long)pathname));
	char *buf = ( char *)(current_proc_vir2phys((long)stat));
	return vfs_stat64(file, buf);
}

int UnixCall( getcwd)(char * pbuf, size_t size)
{
	int err;

	char *buf = ( char *)(current_proc_vir2phys((long)pbuf));
	mem_writeable((void*)buf, size);

	err=	 getpwd(buf, size);
	return err;
}

int UnixCall( chown)(const char * pathname, uid_t uid, gid_t gid)
{
	SYSCALL_NOT_SUPPORT; 
	return ENOSYS;
}



int UnixCall( personality)()
{
	//SYSCALL_NOT_SUPPORT;
	return 0;
}

int UnixCall( readlink)(const char * pathname, char * buf, size_t buflen)
{
	int error;	

	pathname = ( char *)(current_proc_vir2phys((long)pathname));
	buf = ( char *)(current_proc_vir2phys((long)buf));

	mem_writeable((void*)buf, buflen);
	error = vfs_readlink(pathname,buf,buflen);
		
	return error;
}

int UnixCall( fchdir)(int fd)
{
	SYSCALL_NOT_SUPPORT;
	return ENOSYS;
}

int UnixCall( fstat)(int fd, stat_t * pstat)
{
	int err;
	//SYSCALL_NOT_SUPPORT;
	stat_t *buf = ( char *)(current_proc_vir2phys((long)pstat));

	err = vfs_fstat(fd, buf);
	return err;
}

int UnixCall( stat)(const char * pathname, stat_t * pstat)
{
	int err;
	char *file = ( char *)(current_proc_vir2phys((long)pathname));
	stat_t *buf = ( char *)(current_proc_vir2phys((long)pstat));

	if (!pathname)
	{
		return -1;
	}

	err = fs_stat(file, buf);

	return err;
}

int UnixCall( getrusage)()
{
	SYSCALL_NOT_SUPPORT;
	return ENOSYS;
}

int UnixCall( mount)(char * dev, char * dir, char * fstype, int flags, void *data)
{
	char *kdev = ( char *)(current_proc_vir2phys((long)dev));
	char *kdir = ( char *)(current_proc_vir2phys((long)dir));
	char *ktype = ( char *)(current_proc_vir2phys((long)fstype));

	//kprintf("vfs_mount dev-%s dir-%s at ecx 0x%x %s\n", kdev,kdir,fstype,ktype);
	int err = vfs_mount(kdev,kdir,ktype,flags);
	return err;
}

int UnixCall( umount)(char * name)
{
	char *kdev = ( char *)(current_proc_vir2phys((long)name));
	kprintf("vfs_umount dev-%s \n", kdev);
	int err = vfs_unmount(kdev);
	//SYSCALL_NOT_SUPPORT; 
	return err;
}

int UnixCall( getpgrp)()
{
	SYSCALL_NOT_SUPPORT; 
	return ENOSYS;
}

int UnixCall( umask)(int mask)
{
	int old = 0777;//curr->fs->umask;

	//curr->fs->umask = mask & 0777; 
	return old;
}

int UnixCall( setreuid)(uid_t ruid, uid_t euid)
{
	uid_t id = current_proc()->uid;
	SYSCALL_NOT_SUPPORT; 
	return id;
}

int UnixCall( setuid)(uid_t uid)
{
	current_proc()->uid = uid;
	return 0;
}



int UnixCall( sigprocmask)(int how, sigset_t * set, sigset_t * oset)
{
	thread_t *pthread = current_thread();
	sigset_t new_set, old_set = pthread->signal_mask;
	regs_t *regs = (regs_t*) &how;

#define _S(nr) (1<<((nr)-1))
#define _BLOCKABLE (~(_S(SIGKILL) | _S(SIGSTOP)))


  if(inusermode(regs->cs)){
	  if(set)set = (sigset_t *)(current_proc_vir2phys(set));
	  if(oset)oset = (sigset_t *)(current_proc_vir2phys(oset));
  }

	//kprintf("how=%d set=%s\n", how, signame(signo(*set)) );

	if (set) {
		mem_writeable((void*)set, sizeof(sigset_t));
		new_set = *set & _BLOCKABLE;
		switch (how) {
		case SIG_BLOCK:
			pthread->signal_mask |= new_set;
			break;
		case SIG_UNBLOCK:
			pthread->signal_mask &= ~new_set;
			break;
		case SIG_SETMASK:
			pthread->signal_mask = new_set;
			break;
		default:
			return -EINVAL;
		}
	}
	if (oset) {
		mem_writeable((void*)oset, sizeof(sigset_t));	
		*oset=old_set;
	}
	return 0;
}

int UnixCall( settimeofday)()
{
	SYSCALL_NOT_SUPPORT;
	return ENOSYS;
}

int UnixCall( setrlimit)(unsigned int resource, struct rlimit *rlim)
{
	struct rlimit _new, *old, *ptr;
	thread_t *pthread = current_thread();

	if (resource >= RLIM_NLIMITS)
		return -EINVAL;
	ptr = current_proc_vir2phys(rlim);

	old = pthread->rlimit + resource;
	_new.rlim_cur = ptr->rlim_cur;
	_new.rlim_max = ptr->rlim_max;

	if (((_new.rlim_cur > old->rlim_max) ||
	     (_new.rlim_max > old->rlim_max))  )
		return -EPERM;
	*old = _new;
	return 0;
}

int UnixCall( getrlimit)(unsigned int resource, struct rlimit *rlim)
{
	struct rlimit  *ptr;
	thread_t *pthread = current_thread();

	if (resource >= RLIM_NLIMITS)
		return -EINVAL;

	mem_writeable(rlim,sizeof *rlim);
	ptr = current_proc_vir2phys(rlim);

	ptr->rlim_cur = pthread->rlimit[resource].rlim_cur;
	ptr->rlim_max = pthread->rlimit[resource].rlim_max;
		
	return 0;	
}

//符号连接
int UnixCall( lstat)(const char * pathname, stat_t * stat)
{
	int err;
	char *file = ( char *)(current_proc_vir2phys((long)pathname));
	stat_t *buf = ( char *)(current_proc_vir2phys((long)stat));

	if (!pathname)
	{
		return -1;
	}

	err = fs_stat(file, buf);

	return err;
}

int UnixCall( gettimeofday)(struct timeval * tv, struct timezone * tz)
{
	time_t now = startup_ticks();

	if (tv) {
		tv=(struct timeval *)(current_proc_vir2phys(tv));
		tv->tv_sec = upsince + now/HZ;
		tv->tv_usec = (now%HZ)*(1000000/HZ);
	}

	if (tz && 0) {/*fixme*/
		tz=(struct timezone *)(current_proc_vir2phys(tz));
		tz->tz_minuteswest=-480; /* CST */
		tz->tz_dsttime=0;
	}
	return 0;
}

//尽快实现
int UnixCall( sigpending)(sigset_t * set)
{
	sigset_t *  bits= (sigset_t *)current_proc_vir2phys((long)set) ;
	thread_t *pthread = current_thread();
	int error;

	if (error = mem_writeable(set, sizeof(sigset_t)))
		return error;
	//curr->maskedsig(set);
	bits = pthread->signal_bits & pthread->signal_mask;
	return 0;
}

int UnixCall( sigsuspend)(u32_t bits)
{
	sigset_t newmask;
	thread_t *pthread = current_thread();

	newmask = bits;
	if (sig_invalid(newmask))
		return EINVAL;

	sigset_t oldmask = pthread->signal_mask;
	pthread->signal_mask = newmask;
	thread_wait(pthread, INFINITE);
	pthread->signal_mask = oldmask;
	return EINTR;
}

int UnixCall( setregid)(gid_t rgid, gid_t egid)
{
	SYSCALL_NOT_SUPPORT; 
	return ENOSYS;
}

int UnixCall( sgetmask)()
{
	return current_thread()->signal_mask;
}

int UnixCall( getuid)()
{
	uid_t id;
	int sc;
	
	lock_schedule(&sc);
	id = current_proc()->uid;
	set_schedule(sc);
	return id;
}

int UnixCall( stime)(time_t * t)
{
	long *tptr=(long *) current_proc_vir2phys((long)t) ;
	struct timeval tx;

	tx.tv_sec=*tptr;
	tx.tv_sec=0;

	set_time(&tx);
    return 0;
}

int UnixCall( ssetmask)(int newmask)
{
	thread_t *pthread =current_thread() ;
	int old = pthread->signal_mask;

	pthread->signal_mask = newmask;
	sigdelset(pthread, SIGKILL);
	return old;
}

int UnixCall( setsid)(){SYSCALL_NOT_SUPPORT; return 0;}

int UnixCall( getppid)()
{
	pid_t id;
	int sc;
	lock_schedule(&sc);
	id= current_thread()->ptid;
	set_schedule(sc);
	return id;
}



int UnixCall( chroot)(const char * newname)
{
	return chroot((char*)current_proc_vir2phys(newname));
}

int UnixCall( brk)(u32_t ebss)
{
	u32_t brk_addr = 0;

	//kprintf("%s to addr 0x%x \n",__FUNCTION__, ebss);
	brk_addr =  _brk(ebss);
	//kprintf("linux_brk returned %x \n",brk_addr);
	return brk_addr;
}

int UnixCall( pipe)(int *fd2)
{
	long* fds = (long *)current_proc_vir2phys((long)fd2);
	int pipe(long *fds);

	return pipe(fds);
}

int UnixCall( times)(tms_t * buf)
{
	thread_t *pthread = current_thread();
	tms_t* tbuf = (tms_t *)current_proc_vir2phys(buf);

	if (buf)
    {
      tbuf->tms_utime=pthread->sticks; //self
      tbuf->tms_stime=pthread->sticks;
      tbuf->tms_cutime=pthread->sticks; //clild
      tbuf->tms_cstime=pthread->sticks;
    }
	return startup_ticks();
}

int UnixCall( setpgid)(pid_t pid, pid_t pgid)
{
	//SYSCALL_NOT_SUPPORT; 
	return ENOSYS;
}

int UnixCall( getegid)()
{
	//SYSCALL_NOT_SUPPORT;
	return 0;
}

int UnixCall( geteuid)()
{
	uid_t id = current_proc()->uid;
	//SYSCALL_NOT_SUPPORT; 
	return id;
}

int UnixCall( getgid)()
{
	//SYSCALL_NOT_SUPPORT; 
	return 0;
}

int UnixCall( mkdir)(const char *name)
{
	char *dirname = (char*)current_proc_vir2phys(name);
	return mkdir(dirname, 0);
}

int UnixCall( rmdir)(const char *name)
{
	char *dirname = (char*)current_proc_vir2phys(name);
	return rmdir(dirname);
}

int UnixCall( pause)()
{
	thread_wait(current_thread(), INFINITE);
	
	return 0;
}

int UnixCall( sync)()
{
	return 0;
}

int UnixCall( rename)(const char * oldname, const char * newname)
{
	int err;

	err=rename((char*)current_proc_vir2phys(oldname), (char*)current_proc_vir2phys(newname));
	return err;
}

int UnixCall( utime)(char * pathname, utimbuf_t * t){SYSCALL_NOT_SUPPORT; return 0;}

int UnixCall( access)(const char *path, int mode)
{
	int error;
	char *_path = current_proc_vir2phys(path);

	error = vfs_access(_path, mode);
	//SYSCALL_NOT_SUPPORT; 
	return error;
}

int UnixCall( setgid)(gid_t gid)
{
	SYSCALL_NOT_SUPPORT;
	return 0;
}

int UnixCall( nice)(int increment)
{
	thread_t *pthread = current_thread();

   if( pthread->prio < increment)
	   pthread->prio=1;
   else
	   pthread->prio-=increment;

  return 0;
}

#define NR_SOCKET_CALL	17		/* sysrecvmsg(2)		*/
static int (*unix_socket_call_hook)(int cmd, void* argp)=NULL ;
void setup_unix_socketcall(int (*fun)(int cmd, void* argp))
{
	unix_socket_call_hook = fun;
}

int UnixCall(socketcall)(int callno, u32_t * argv)
{
	int error;
	const static u8_t socket_argments[NR_SOCKET_CALL+1]={0,3,3,3,2,3,3,3,4,4,4,6,6,2,5,5,3,3};
	static int socketfd = -1;
	int fromkernel = 0;
	u32_t * pargv=current_proc_vir2phys(argv);

	if (callno < 1 || callno > NR_SOCKET_CALL){
		kprintf("error socket called %d\n",callno);
		return EINVAL;
	}

	if (error = mem_writeable(pargv, sizeof(u32_t) * socket_argments[callno]))
		return error;

	if (!unix_socket_call_hook)
	{
		return ENOSYS;
	}

	/*kprintf("socketcall callno %d\n",callno);

	{
		int j;
		for (j=0; j<socket_argments[callno];j++ )
		{
			kprintf("socketcall arg%d %d\n",j, pargv[j]);
		}
	}*/


	//error = socketcall_hook(callno, argv);
	error = unix_socket_call_hook(callno, pargv);
	//kprintf("socketcall returned %d\n",error);
	return error;
}
