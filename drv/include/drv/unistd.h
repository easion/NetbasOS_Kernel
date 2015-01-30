

#ifndef	unistd_h
#define	unistd_h
#ifdef __cplusplus
extern "C" {
#endif

#define NR_CALLS		  256	/* number of system calls allowed */

#define SYSVEC 0x80


//extern int NR_exec(regs_t reg);
#define NR_EXIT		   1 
#define NR_FORK		   2 
#define NR_READ		   3 
#define NR_WRITE		   4 
#define NR_OPEN		   5 
#define NR_CLOSE		   6 
#define NR_WAIT		   7
#define NR_CREAT		   8 
#define NR_LINK		   9 
#define NR_UNLINK		  10 
#define NR_WAITPID		  11
#define NR_CHDIR		  12 
#define NR_TIME		  13
#define NR_MKNOD		  14 
#define NR_CHMOD		  15 
#define NR_CHOWN		  16 
#define NR_BRK		  17
#define NR_STAT		  18 
#define NR_LSEEK		  19
#define NR_GETPID		  20
#define NR_MOUNT		  21 
#define NR_UMOUNT		  22 
#define NR_SETUID		  23
#define NR_GETUID		  24
#define NR_STIME		  25
#define NR_PTRACE		  26
#define NR_ALARM		  27
#define NR_FSTAT		  28 
#define NR_STAT		  106 
#define NR_PAUSE		  29
#define NR_UTIME		  30 
#define NR_ACCESS		  33 
#define NR_NICE		  34
#define NR_SYNC		  36 
#define NR_KILL		  37
#define NR_RENAME		  38
#define NR_MKDIR		  39
#define NR_RMDIR		  40
#define NR_DUP		  41 
#define NR_PIPE		  42 
#define NR_TIMES		  43
#define NR_SETGID		  46
#define NR_GETGID		  47
#define NR_IOCTL		  54
#define NR_FCNTL		  55
#define NR_UNAME	  58
#define NR_EXEC		  59
#define NR_UMASK		  60 
#define NR_CHROOT		  61 
#define NR_SETSID		  62
#define NR_DUP2		  63
#define NR_GETPPID		  64
#define NR_GETTIMEOFDAY		  78
#if 0
#define NR_GETPGRP		  63
#define NR_OPENDIR		  65
#define NR_READDIR		  66

#define NR_GETPWD		  69

#define NR_GETMASK		  68
#define NR_SETMASK		  69
#define NR_SIGNAL		 70

/* Posix signal handling. */
#define SIGACTION	  71
#define SIGSUSPEND	  72
#define SIGPENDING	  73
#define SIGPROCMASK	  74
#define SIGRETURN	  75
#define NR_SIGRETURN 75
#define NR_REBOOT		  76
#endif

static inline unsigned int do_system_call(int function_number,int p1,int p2,
                  int p3,int p4,int p5)
{
     int return_value;
    __asm__ volatile ("int $0x80" \
        : "=a" (return_value) \
        : "0" ((long)(function_number)),"b" ((long)(p1)),"c" ((long)(p2)), \
          "d" ((long)(p3)),"S" ((long)(p4)),"D" ((long)(p5)) ); \
    return return_value;
};

static inline unsigned int netbas_system_call(int function_number,int p1,int p2,
                  int p3,int p4,int p5)
{
     int return_value;
    __asm__ volatile ("int $0x60" \
        : "=a" (return_value) \
        : "0" ((long)(function_number)),"b" ((long)(p1)),"c" ((long)(p2)), \
          "d" ((long)(p3)),"S" ((long)(p4)),"D" ((long)(p5)) ); \
    return return_value;
};


#ifdef __cplusplus
}
#endif

#endif


