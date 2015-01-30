

#ifndef	unistd_h
#define	unistd_h



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
#define NR_PAUSE		  29
#define NR_UTIME		  30 
#define NR_ACCESS		  33 
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
#define NR_SIGNAL		  48
#define NR_IOCTL		  54
#define NR_FCNTL		  55
#define NR_UNAME	  58
#define NR_EXEC		  59
#define NR_UMASK		  60 
#define NR_CHROOT		  61 
#define NR_SETSID		  62
#define NR_GETPGRP		  63
#define NR_OPENDIR		  65
#define NR_READDIR		  66

#define NR_GETPWD		  69
#define NR_GETTIMEOFDAY		  78
#define NR_GETARG		  87

/* Posix signal handling. */
#define SIGACTION	  71
#define SIGSUSPEND	  72
#define SIGPENDING	  73
#define SIGPROCMASK	  74
#define NR_SIGRETURN 75
#define NR_REBOOT		  76

#if 0

/* Values used by access().  POSIX Table 2-8. */
#define F_OK               0	/* test if file exists */
#define X_OK               1	/* test if file is executable */
#define W_OK               2	/* test if file is writable */
#define R_OK               4	/* test if file is readable */



/* This value is required by POSIX Table 2-10. */
#define _POSIX_VERSION 199009L	/* which standard is being conformed to */

/* These three definitions are required by POSIX Sec. 8.2.1.2. */
#define STDIN_FILENO       0	/* file descriptor for stdin */
#define STDOUT_FILENO      1	/* file descriptor for stdout */
#define STDERR_FILENO      2	/* file descriptor for stderr */
#endif

#ifdef __KERNEL__
/* How to exit the system. */
#define RBT_HALT	   0
#define RBT_REBOOT	   1
#define RBT_PANIC	   2	/* for servers */
#define RBT_MONITOR	   3	/* let the monitor do this */
#define RBT_RESET	   4	/* hard reset the system */
#endif



#endif


