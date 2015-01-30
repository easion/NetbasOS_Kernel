#ifndef __LINUX_ABI_H__
#define __LINUX_ABI_H__

#include <jicama/system.h>
#include <jicama/utsname.h>


typedef struct utimbuf_t {
	time_t atime;
	time_t mtime;
}utimbuf_t;

typedef struct  {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
}linux_utsname_t;

typedef void (*sigfunc_t)(int);
struct sigact_t {
	sigfunc_t func;
	u32_t mask;
	int flags;
	void (*restorer)(void);
};

typedef struct  {
	u32_t retaddr;
	int signo;
	u16_t gs, __gsh;
	u16_t fs, __fsh;
	u16_t es, __esh;
	u16_t ds, __dsh;
	u32_t edi;
	u32_t esi;
	u32_t ebp;
	u32_t esp;
	u32_t ebx;
	u32_t edx;
	u32_t ecx;
	u32_t eax;
	u32_t eip;
	u16_t cs, __csh;
	u32_t eflags;
	u16_t ss, __ssh;
	u32_t oldsigmask;
	char asmcode[8];
}sigcontext_t;


int UnixCall(lstat64)(const char * pathname, stat64_t * stat);
int UnixCall(stat64)(const char * pathname, stat64_t * stat);
int UnixCall(getcwd)(char * buf, size_t size);
int UnixCall(chown)(const char * pathname, uid_t uid, gid_t gid);

int UnixCall(personality)(void);
int UnixCall(readlink)(const char * pathname, char * buf, size_t buflen);
int UnixCall(fchdir)(int fd);
int UnixCall(sigreturn)(u32_t ebx);
int UnixCall(fstat)(int fd, stat_t * stat);
int UnixCall(stat)(const char * pathname, stat_t * stat);
int UnixCall(getrusage) (void);
int UnixCall(mount) (char * dev, char * dir, char * fstype, int flags, void *data);
int UnixCall(umount) (char * name);
int UnixCall(getpgrp) (void);
int UnixCall(umask) (int mask);
int UnixCall(setreuid) (uid_t ruid, uid_t euid);
int UnixCall(setuid) (uid_t uid);
int UnixCall(symlink) (const char * oldname, const char * newname);
int UnixCall(sigprocmask) (int how, sigset_t * set, sigset_t * oset);
int UnixCall(settimeofday) (void);
int UnixCall(lstat) (const char * pathname, stat_t * stat);
int UnixCall(gettimeofday) (struct timeval * tv, struct timezone * tz);
int UnixCall(sigpending) (sigset_t * set);
int UnixCall(sigsuspend) (u32_t bits);
int UnixCall(setregid) (gid_t rgid, gid_t egid);

int UnixCall(sgetmask) (void);
int UnixCall(getuid) (void);
int UnixCall(stime) (time_t * t);
int UnixCall(ssetmask) (int newmask);
int UnixCall(setsid) (void);
int UnixCall(getppid) (void);
int UnixCall( ptrace)(long request, long pid, long addr, long data);
int UnixCall(chroot) (const char * newname);
int UnixCall(brk) (u32_t ebss);
int UnixCall(pipe)(int fd2[2]);
int UnixCall(times)(tms_t * t);
int UnixCall(setpgid)(pid_t pid, pid_t pgid);
int UnixCall(getegid)(void);
int UnixCall(geteuid)(void);
int UnixCall(getgid)(void);
int UnixCall(mkdir)(const char *name);
int UnixCall(rmdir)(const char *name);
int UnixCall(pause)(void);
int UnixCall(sync)(void);
int UnixCall(rename)(const char * oldname, const char * newname);
int UnixCall(utime)(char * pathname, utimbuf_t * t);
int UnixCall(access)(const char *path, int mode);
int UnixCall(setgid)(gid_t gid);
int UnixCall(nice)(int inc);
int UnixCall(socketcall)(int callno, u32_t * argv);
 int UnixCall( ioperm)( unsigned long from, unsigned long num, int turn_on);
int UnixCall( iopl)(unsigned int level );
int UnixCall( sigreturn)(u32_t ebx);
int UnixCall( clone )(unsigned int flag );
int socketcall_handler(int call, int (* callbackfun)(void));

 #define NR_LINUX_CALLS 200
__asmlink int (*UnixCall_table[NR_LINUX_CALLS]) (void);


//int conv_from_linux_regs(regs_t *to, const UnixCall_regs_t *from);
//int conv_to_linux_regs(UnixCall_regs_t *to, const regs_t *from);
int linux_kernel_init(void);

#endif

