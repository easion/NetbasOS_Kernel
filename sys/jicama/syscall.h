
// ------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//-------------------------------------------------------------------------------
#ifndef SYS_CALL_H
#define SYS_CALL_H



#define NR_CALLS		  256	/* number of system calls allowed */


#define SYSVEC 0x60
#define UNIXVEC 0x80


 extern int get_clock(void);
 extern void mk_ujfs(void);
 extern struct inode* dir_namei(unsigned char* path_name);
 extern void fs_dir(void);
 extern int putchar(const unsigned char c);
 extern unsigned char getchar(void);


//extern int sys_exec(regs_t reg);
#define SYS_EXIT		   1 
#define SYS_FORK		   2 
#define SYS_READ		   3 
#define SYS_WRITE		   4 
#define SYS_OPEN		   5 
#define SYS_CLOSE		   6 
#define SYS_WAIT		   7
#define SYS_CREAT		   8 
#define SYS_LINK		   9 
#define SYS_UNLINK		  10 
#define SYS_WAITPID		  11
#define SYS_CHDIR		  12 
#define SYS_TIME		  13
#define SYS_MKNOD		  14 
#define SYS_CHMOD		  15 
#define SYS_CHOWN		  16 
#define SYS_BRK		  17
#define SYS_STAT		  18 
#define SYS_LSEEK		  19
#define SYS_GETPID		  20
#define SYS_MOUNT		  21 
#define SYS_UMOUNT		  22 
#define SYS_SETUID		  23
#define SYS_GETUID		  24
#define SYS_STIME		  25
#define SYS_PTRACE		  26
#define SYS_ALARM		  27
#define SYS_FSTAT		  28 
#define SYS_PAUSE		  29
#define SYS_UTIME		  30 
#define SYS_ACCESS		  33 
#define SYS_SYNC		  36 
#define SYS_KILL		  37
#define SYS_RENAME		  38
#define SYS_MKDIR		  39
#define SYS_RMDIR		  40
#define SYS_DUP		  41 
#define SYS_PIPE		  42 
#define SYS_TIMES		  43
#define SYS_SETGID		  46
#define SYS_GETGID		  47
#define SYS_SIGNAL		  48
#define SYS_IOCTL		  54
#define SYS_FCNTL		  55
#define SYS_EXEC		  59
#define SYS_UMASK		  60 
#define SYS_CHROOT		  61 
#define SYS_SETSID		  62
#define SYS_GETPGRP		  63

/* Posix signal handling. */
#define SIGACTION	  71
#define SIGSUSPEND	  72
#define SIGPENDING	  73
#define SIGPROCMASK	  74
#define SIGRETURN	  75

typedef int *syscall_fuc (void);



//extern int (*syscall_vector[NR_CALLS]) (regs_t *r);




#endif
