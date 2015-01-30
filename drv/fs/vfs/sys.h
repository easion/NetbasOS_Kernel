/*
**     (R)Jicama OS
**     
**     Copyright (C) 2006 DengPingPing
*/

#ifndef __F_SYS_H_
#define __F_SYS_H_
typedef struct  {
        unsigned short  dev;
        unsigned char   __pad0[10];

#define STAT64_HAS_BROKEN_ST_INO        1
        unsigned long   __ino;

        unsigned int    mode;
        unsigned int    nlink;

        unsigned long   uid;
        unsigned long   gid;

        unsigned short  rdev;
        unsigned char   __pad3[10];

        long long       size;
        unsigned long   blksize;

        unsigned long   blocks;      /* Number 512-byte blocks allocated. */
        unsigned long   __pad4;      /* future possible blocks high bits */

        unsigned long   atime;
        unsigned long   __pad5;

        unsigned long   mtime;
        unsigned long   __pad6;

        unsigned long   ctime;
        unsigned long   __pad7;      /* will be high 32 bits of ctime someday */

        unsigned long long      ino;
}stat64_t;


struct dirent_t {
	long ino;
	long off;
	short reclen;
	char name[0];
};


typedef struct 
{
	char *fsname;
	char *author;
	char *copyright;
	int ( *open )(const char *, int, int );
	int ( *create )(const char *, int );
	int ( *link )(const char* file,const char*file2);
	int ( *unlink )(const char * );
	int ( *rmdir )(const char * );
	int ( *rename )(const char *,const char * );
	int ( *mkdir )(const char *,int );
	int ( *close )(int);
	int ( *read )(int, const void*, off_t, int);
	int ( *write )(int, const void*, off_t, int);
	int ( *lseek )(int,  off_t,int);
	int ( *fcntl )(int, int, void*);
	int ( *dup )(int);
	int ( *dup2 )(int, int);
	int ( *sync )();
	int ( *stat )(const char * , struct stat * );	 
	int ( *stat64 )(const char * , stat64_t * );	 
	int ( *access )(const char *filename, int mode);	 
	int ( *fstat )(unsigned int fd, struct stat *);	 
	int ( *mount )(const char *devname, char *f2, const char *fstype,long flag);	 
	int ( *unmount )(char *);	 
	int ( *chdir )(const char *);	 
	int ( *getpwd )(char *, int);	 
	int ( *chroot )(const char *);	 
	off_t (*sendfsinfo)(int _fildes, off_t _fildes2, int arg);
	int (*readdir)(int _fildes, void *buf, int arg);
	off_t (*opendir)(const char *name);
	off_t (*closedir)(int _fildes, off_t _fildes2, int arg);
	off_t (*s4)(int _fildes, off_t _fildes2, int arg);
	int (*pipe)(long *fds);

	int (*create_fd)(void *socketdata, int issock);
	void* (*get_socket_data)(int fd,u32_t *t);
	int (*remove_socket)(int fd);
	int ( *readlink )(const char *, char *, int);	 
}fsapi_ops_t;

#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE-1)

struct pipe
{
  char data[4096];
	int pipe_head;
	int pipe_tail;
};

typedef struct 
{
	dev_prvi_t base;
  //struct ioobject iob;
 // struct pipe_data *pipe_data;

 struct pipe *pipe;

	//struct ioobject *io_wait;
	file_t *peer;
	//int closed;

}pipe_prvi_t;
file_t *get_empty_filp(void);

inode_t * create_pipe_node(int cnt);

int sys_dup(unsigned int fildes);

int sys_mount(const char *devname, char *mntdir, const char *fstype, long flags);
int dev_open(const char *path, unsigned access, dev_prvi_t*);
int sys_chdir(unsigned char* new_path);
int do_get_pwd(char *p, int sz);
int get_currentdisk();
  int sys_create(unsigned char *path, mode_t mode);
 int do_unlink(const char *path);
 int do_link(const char *path_name,const char *pathnew);
int do_get_pwd(char *p, int sz);
int do_rename(unsigned char *path, char *n);
 int do_mkdir(unsigned char *path, int mode);
int sys_chroot(char *c);


int sys_dup2(unsigned int oldfd, unsigned int newfd);
int sys_pipe( long * fildes);

int register_fs(fsapi_ops_t *ops);
off_t clone_fd(int _fildes, off_t _fildes2, int arg);

static void sys_fs_setup();
int sys_unmount(char *dev);
int sys_readdir(unsigned int fd, void *buf, int arg);
int current_taskno();
void fs_update();
inline dev_prvi_t* get_dev_desc(dev_t dev);

#define FD_CLOEXEC	0x0001

#define F_DUPFD		0	/* Duplicate file descriptor.  */
#define F_GETFD		1	/* Get file descriptor flags.  */
#define F_SETFD		2	/* Set file descriptor flags.  */
#define F_GETFL		3	/* Get file status flags.  */
#define F_SETFL		4	/* Set file status flags.  */
# define F_GETLK	5	/* Get record locking info.  */
# define F_SETLK	6	/* Set record locking info (non-blocking).  */
# define F_SETLKW	7	/* Set record locking info (blocking).	*/

#define F_UNLCK		0
#define F_RDLCK		1
#define F_WRLCK		2

#define IS_PROC_FILE(FILENAME) (strnicmp((FILENAME),"/proc/",6)==0)
#define IS_PROC_DIR(FILENAME) (stricmp((FILENAME),"/proc/")==0||stricmp((FILENAME),"/proc")==0)


#define IS_DEV_FILE(FILENAME) (strnicmp((FILENAME),"/dev/",5)==0)
#define IS_DEV_DIR(FILENAME) (stricmp((FILENAME),"/dev/")==0||stricmp((FILENAME),"/dev")==0)


#endif //__F_SYS_H_
