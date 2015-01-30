#ifndef FS_H
#define FS_H




       /*-----POSIZ Constants-----*/
#include <fcntl.h>
#include <jicama/ioctl.h>

typedef struct 
{
	int lock;
}rwlock_t;

#define EOF -1
#define NIL_FLP ((struct filp *)0)

#define RAW_FILE_NR 48

/* Values used for whence in lseek(fd, offset, whence).  POSIX Table 2-9. */
#define SEEK_SET           0	/* offset is absolute  */
#define SEEK_CUR           1	/* offset is relative to current position */
#define SEEK_END           2	/* offset is relative to end of file */

#define MODFS_STARTFD 0x80
#define ismodfsfd(n) ((n)->open_kfs!=NULL)
typedef int sfifo_atomic_t;
#define FPATH_LEN 255

typedef struct filp
{
	unsigned char name[FPATH_LEN];
	char *data;
	long fd, attr;//accress;
	size_t size;/* Number of bytes */
	sfifo_atomic_t readpos;		/* Read position */
	sfifo_atomic_t writepos;	/* Write position */
	int ( *open_kfs )(struct filp*);
	int ( *close_kfs )(struct filp*);
	int ( *read_kfs )(struct filp*,  void*, off_t);
	int ( *write_kfs )(struct filp*, const void*, off_t);
	int ( *flush_kfs )(struct filp*);
	int ( *seek_kfs )(struct filp*,off_t, int);
} file_t;

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

typedef struct  { /* kernel stat */
	unsigned short dev;
	unsigned short __pad1;
	unsigned long ino;
	unsigned short mode;
	unsigned short nlink;
	unsigned short uid;
	unsigned short gid;
	unsigned short rdev;
	unsigned short __pad2;
	unsigned long  size;
	unsigned long  blksize;
	unsigned long  blocks;
	unsigned long  atime;
	unsigned long  __unused1;
	unsigned long  mtime;
	unsigned long  __unused2;
	unsigned long  ctime;
	unsigned long  __unused3;
	unsigned long  __unused4;
	unsigned long  __unused5;
}stat_t;

int  fs_stat(const char * pathname, stat_t * stat);

int vfs_open(const char * filename, int flag,  int mode);
int	 vfs_readdir(int fd, const void *buf, size_t s);

struct dirent_t {
	long ino;
	long off;
	short reclen;
	char name[256];
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
	int ( *sync )(void);
	int ( *stat )(const char * , stat_t * );	 
	int ( *stat64 )(const char * , stat64_t * );	 
	int ( *access )(const char *filename, int mode);	 
	int ( *fstat )(unsigned int fd, stat_t *);	 
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

int fs_create_file_filp(void *socketdata,int issock);
void* fs_get_prvi_data(int fd,u32_t *t);




typedef struct
{
  char  l_long_name[260]; /* Null-terminated long file name in Unicode UTF-8 */
  unsigned short l_item_size;
  unsigned long l_attribute;          /* File attributes                                 */
  long long l_ctime;         /* File creation time in Win32 or DOS format       */
  long long l_atime;         /* Last access time in Win32 or DOS format         */
  long long l_mtime;         /* Last modification time in Win32 or DOS format   */
  unsigned long l_size_high;        /* High 32 bits of the file size in bytes          */
  unsigned long l_size_low;        /* Low 32 bits of the file size in bytes           */
  union{
    unsigned char  l_res[8];   /* Reserved bytes, must be ignored                 */
	struct 
	{
		long l_ino;
		long l_off;
	}d;
  };
}__attr_packet vfs_dirent_t;


#define DLL_FILE 1
#define EXEC_FILE 2
#define LOGO_FILE 8
#define TEXT_FILE 16
#define FIFO_FILE 32

size_t fwrite(const void *ptr, size_t size, size_t nmemb,  register struct filp *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, register struct filp *stream);
struct filp* modfs_open(const char *p, char *mode);
int fseek(struct filp *fp, off_t offset, int where);

void fclose(struct filp* p);
struct filp* server_open(const char *name, char *mode);
int flength(struct filp *p);
size_t	 read(int fd, const void *buf, size_t s);
size_t	 write(int fd, const void *buf, size_t s);
int rename(const char *n1, const char *n2);
int getpwd(char *n1, int len);
int dup2(int fd, int fd_new);
int chroot(const char *f);
int mount(const char *devname, char *f2, int flags);
int mkdir(const char *f, mode_t mode);
int rmdir(const char *filename);
int dup(int fd);
int chdir(const char *f);
 off_t sendfsinfo(int _fildes, off_t _fildes2, int arg);


file_t* fifo_init(const char *name, int size);

struct filp* modfs_add(const char *n, char *p, int sz);
struct filp* kfs_slot(void);
int djmz_check(struct filp* _fp);
int unregister_fs(char *name);
int select_fs(char *name);
int register_fs(fsapi_ops_t *ops);

__asmlink int fseek(struct filp *fp, off_t offset, int where);
__asmlink void fclose(struct filp* p);
__asmlink size_t fread(void *ptr, size_t size, size_t nmemb, register struct filp *stream);

bool is_gzip_format_file(u8_t *magic);
int gzip_file(const char *inbuf,const int inlen, char **outbuf, int *len);
size_t gzip_orig_length(u8_t *magic, int len);


int write_dll( char * buf, int size);
int moduledump(char *buf, int size);
int mem_dump(const char * buf, int size);
int read_log(char *buf, int size);
int procdump( char * buf, int size);
int msg_proc(char *buf, int len);
int devices_dump(char *buf, int len);
int irq_proc_dump(char *buf, int len);

int vfs_close(int fd);
size_t	 vfs_read(int fd, const void *buf, size_t s, int arg);
size_t	 vfs_write(int fd, const void *buf, size_t s, int arg);
off_t lseek(int fildes, off_t offset, int whence);
int  vfs_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg);
int vfs_creat(const char *f, u16_t permiss);
int vfs_unlink(const char *f);
int vfs_link(const char *f,const char *fnew);
int vfs_mount(const char *devname, char *f2, const char *fstype, long mode);
int  vfs_fstat(int fd, stat_t * stat);
int  vfs_access(const char * pathname, int mode);
int  vfs_stat64(const char * pathname, stat64_t * stat);
int	 vfs_readlink(const char* path, const void *buf, size_t s);
int vfs_unmount(const char *devname);







int read_proc(char *name, char *buf, int len);
int filp_init(void);

void uart_putchar(int ch);
void mdelay(int msec);


#endif


