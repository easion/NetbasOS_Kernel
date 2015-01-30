

#ifndef __fsserver_H__
#define __fsserver_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <drv/fnctl.h>

#define NR_FILES		   128 
#define NR_FPOOLS		   4096
#define NR_SUPER	8
#define START_OPEN  (3)

//#define istermfd(n) ((n)>=0&&(n)<3)
//#define isdevfd(n) ( (n)>99&&(n)<0xffff)
#define F_DUP_UNMAP 0XFFFF
#define ERR -1
#if 0
enum{
	N_PIPE=0x1,
	N_PROCFS=0x2,
	N_SOCKET=0x4,
	N_DEV=0x8,
	N_NORMAL=0x10,
//	N_DUP,
};
#endif

#define MAX_PROC 48
#define NR_INODE 64

#define MAXPATH                 256     // Maximum filename length (including trailing zero)
#define HD_SECTOR_SIZE 512
#define HD_SECTOR_SIZE_MASK 511



int thread_sleep_on(struct thread_wait *queue);
bool thread_waitq_empty(struct thread_wait *queue);
void thread_waitq_init(struct thread_wait *queue);
int thread_wakeup(struct thread_wait *queue);
int thread_wakeup_all(struct thread_wait *queue,int max);


struct super_block;

typedef  struct inode // This Struct Used by Directory
{
	unsigned short    i_dev;
	unsigned short    i_uid;
	unsigned short   i_count;
	unsigned long     i_mode;
	unsigned long     i_number;
	unsigned long     i_father;
	unsigned long   i_size;
	unsigned long   i_atime;
	unsigned long   i_mtime;
	unsigned long   i_ctime;
	unsigned char i_dirt;
	unsigned char i_lock;
	unsigned char i_mount;
	unsigned long i_seek; /*in father node entry offset*/
	unsigned char i_update;
	struct super_block *i_super;
	void *i_private_data;
	TAILQ_ENTRY(inode) next;
	//dev_prvi_t *i_dev_ptr;
	//void *i_priv_data;
} inode_t;

struct  stat{ /* kernel stat */
	unsigned short st_dev;
	unsigned short st___pad1;
	unsigned long st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	unsigned short st_rdev;
	unsigned short st___pad2;
	unsigned long  st_size;
	unsigned long  st_blksize;
	unsigned long  st_blocks;
	unsigned long  st_atime;
	unsigned long  st___unused1;
	unsigned long  st_mtime;
	unsigned long  st___unused2;
	unsigned long  st_ctime;
	unsigned long  st___unused3;
	unsigned long  st___unused4;
	unsigned long  st___unused5;
};

typedef struct  
{
	long f_type; /* 文件系统类型 */
	long f_bsize; /* 块大小*/
	long f_blocks; /* 块多少*/
	long f_bfree; /* 空闲的块（）*/
	long f_bavail; /* 可用块 */
	long f_files; /* 总文件节点 */
	long f_ffree; /* 空闲文件节点 */
	//fsid_t f_fsid; /* 文件系统id */
	unsigned int f_fsid_val[2];
	long f_namelen; /* 文件名的最大长度 */
	char f_volume_name[24]; /* spare for later */
}statfs_t;

#define FAT12_MAGIC	4085
#define FAT16_MAGIC	( ( unsigned ) 65526l )
#define FAT32_MAGIC	268435455l
#define PROC_MAGIC	'PROC'
#define NTFS_MAGIC	'NTFS'
#define ISO9660_MAGIC	'CDFS'
#define DEVFS_MAGIC	'DEVF'
#define ROMFS_MAGIC	'ROMF'
#define MINIXFS_MAGIC	'MINX'
#define EXT2_MAGIC	'EXT2'
#define NO_MAGIC	0
struct fs_dev_ops;

/*unix system v like*/
typedef struct super_block
{
	int m_magic;
	//dev_t m_dev;
	dev_prvi_t *m_dev;
	ino_t m_root_ino;
	inode_t *m_root;
	char *m_super;   /*super block*/
	int m_count;
	void *m_private_data;
	TAILQ_ENTRY(super_block) next;
	int m_blk_size;
	struct fs_dev_ops *m_ops;
}mount_t;


typedef struct
{
	int f_mode;
	//unsigned short f_flags;
	unsigned short f_count;
	inode_t * f_inode;
	off_t f_pos;  //or f_remap
	unsigned short reto;
	struct ioobject *f_iob;
	struct thread_wait f_wait;
	void *f_cookie;
} file_t;


#define NR_MOUNT_LIST 8

typedef enum{D_NONE=0,D_LOOP,D_PROC,D_DEV,D_FILE}m_dev_type_t;

struct mount_list
{
	char m_dev_name[255];
	char m_mnt_dir[255];
	mount_t *m_super;
	TAILQ_ENTRY(mount_list ) next;
};



//extern mount_t super[NR_SUPER];

typedef struct 
{
	int uid;
	u32_t fp_cloexec;
	file_t  *fp[NR_FILES];
	inode_t *pwd;
	inode_t *root;

}fs_task_t;

extern file_t  FilePool[NR_FPOOLS];



extern fs_task_t *t_current;
extern fs_task_t proc[MAX_PROC];
//extern inode_t vnode[NR_INODE];
       /*-----POSIZ Constants-----*/


inode_t * iget(const int dev, const int blk);
int iput(inode_t* inode);
void fd_reset(int proc_nr);


typedef struct
{
  char  l_long_name[260]; /* Null-terminated long file name in Unicode UTF-8 */
  unsigned short l_name_len;
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
}__attribute__ ((packed)) vfs_dirent_t;

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct fs_dev_ops
{
	//int dev_no;
	char *fs_name;
	char *fs_copyright;
	char *fs_author;
	char *fs_ver;
	//int fs_blksz;

	inode_t* (*fs_opendir)(inode_t* old, unsigned char *name);
	inode_t* (*fs_readlink)(inode_t* old, char *buff, size_t buf_size);
	int (*fs_closedir)(inode_t* old); //
	int (*fs_readdir)(file_t * fp, vfs_dirent_t *vd);

	inode_t* (*fs_get_inode_name)(inode_t* old, ino_t ino, unsigned char *name);
	//int (*fs_dirsz)(int dev, const unsigned long);

	int (*fs_openfile)(file_t * filp); //
	int (*fs_releasefile)(file_t * filp);

	int (*fs_bmap)(inode_t * inode, off_t pos, int create);
	int (*fs_read)(file_t * filp, char * buf, int count);
	int (*fs_write)(file_t * filp, char * buf, int count);
	int (*fs_write_inode)(inode_t * );

	int (*fs_creat) (inode_t* n, unsigned char* file,int mode);
	int (*fs_unlink )(inode_t* n, unsigned char* file);
	int (*fs_symlink )(inode_t* n, const char* file,const char*file2);
	int (*fs_rmdir)(inode_t* n, char* dname);
	int (*fs_mkdir)(inode_t* n, char* dname,int nPerms);
	int	(*fs_truncate)(mount_t *mp, unsigned long cluster );
	int (*fs_probe)(const mount_t *mp);
	int (*fs_mount)(mount_t *mp, void *arg);
	int (*fs_unmount)(mount_t *mp, void *);
	int (*fs_inorw)(mount_t *mp, inode_t *inode, int rw);
}fs_dev_ops_t;

/*enum{
	E_PROBE_NTFS,
	E_PROBE_FAT12,
	E_PROBE_FAT32,
	E_PROBE_EXT2,
	E_PROBE_ISO9660,
};
*/
int install_init();
int install_fs(fs_dev_ops_t *ops);
int deinstall_fs(fs_dev_ops_t *ops);
off_t vfs_bmap(inode_t * inode, off_t pos, int create);
inode_t* opendir(inode_t * inode, char *name);
int block_size(int dev);
//int dir_size(dev_t dev, const unsigned long c);

struct dirent
{
  unsigned int ino;
  unsigned int reclen;
  unsigned int namelen;
  char name[MAXPATH];
  unsigned long date,size;
};

enum{
	DISK_FDA=0,
	DISK_RAMDISK=1,
	DISK_HDC=2,
	DISK_CDROM=3,
};

//////////

int try_probe_fs(mount_t *mp);
int vfs_mount(mount_t *mp, void *arg);
void sync_inodes(void);
int vfs_probe(mount_t *mp);
void inode_init(void);

mount_t* get_superblock(dev_t dev);
mount_t* alloc_superblock(dev_prvi_t *dev_nr);
inode_t* vfs_opendir(inode_t * inode, char *name);
int vfs_write(file_t * fp,char* buffer, int count, int *);
int vfs_read(file_t * fp,char* buffer, int count,int*);
int vfs_readdir(file_t * fp, vfs_dirent_t *vd);
int vfs_write_inode(inode_t *inode);
int dev_block_size(int dev);
int open_namei(const char* path_name, 
	int flags,  inode_t** new_node);
mount_t* root_superblock();
fs_task_t *current_filp();
void chroot_filp(fs_task_t *fp);
 int do_rmdir(const char *path_name);
file_t* get_dev_filp(dev_prvi_t *dev);


int sys_open(unsigned char* file_name, int flags, int mode);
int sys_read(int fd, char * buf, int count, int);
int sys_write(int fd, const char * buf, int count, int);
int sys_chdir(unsigned char* new_path);
int sys_lseek(int fd, off_t pos, int where);
int sys_close (int);

#ifdef __cplusplus
};
#endif

#endif
