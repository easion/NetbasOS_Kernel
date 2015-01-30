#ifndef __NETBAS_BFS_H__
#define __NETBAS_BFS_H__

//#define malloc kcalloc
#ifdef __cplusplus
extern "C" {
#endif


typedef long long int64;
typedef long  int32;
typedef long  vint32;
typedef short  int16;
typedef char  int8;
typedef unsigned int  uint;
typedef unsigned int  vint;

typedef unsigned long long uint64;
typedef unsigned long  uint32;
typedef unsigned short  uint16;
typedef unsigned char  uint8;
typedef unsigned short  kdev_t;
typedef int sem_id;
typedef  long bigtime_t;
//typedef uint8 wchar_t;
typedef int status_t;

#ifdef __USER_MODE__

#define kfree free
#define printk printf
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
//#include <stdint.h>
#else

#include <drv/drv.h>

#ifdef __cplusplus
#include <drv/cpplib.h>
#endif

#include <drv/fnctl.h>
#include <drv/fs.h>
#include <linux/atomic.h>
#include <errno.h>
#include <string.h>
#endif

#define R_OK	0x04
#define W_OK	0x02
#define X_OK	0x01
#define F_OK	0x00


#ifdef __USER_MODE__
typedef int Devfd_t;
#else
typedef dev_prvi_t* Devfd_t;
#endif

#define BFS_TRACE 		do{\
	kprintf("%s: called at line%d\n", __FUNCTION__,__LINE__);\
}\
while (0)

//status_t  setup_device_cache( Devfd_t nDevice, fs_id nFS, off_t nBlockCount );
status_t  flush_cache_block( Devfd_t nDev, off_t nBlockNum );
status_t  flush_device_cache( Devfd_t nDevice, bool bOnlyLogBlocks );
status_t  flush_locked_device_cache( Devfd_t nDevice, bool bOnlyLogBlocks );
status_t  shutdown_device_cache( Devfd_t nDevice );

void* get_empty_block( Devfd_t nDev, off_t nBlockNum, size_t nBlockSize );
void* get_cache_block( Devfd_t nDev, off_t nBlockNum, size_t nBlockSize );
status_t mark_blocks_dirty( Devfd_t nDev, off_t nBlockNum, size_t nBlockCount );
status_t set_blocks_info( Devfd_t nDev, off_t* panBlocks, size_t nCount, bool bDoClone, void (*func)(long unsigned int,  int, void*), void* pArg );
void  release_cache_block( Devfd_t nDev, off_t nBlockNum );
status_t  setup_device_cache( Devfd_t nDevice, int nFS, off_t nBlockCount );

#define get_system_time() (startup_ticks()*(1000000/HZ))

size_t	read_pos( Devfd_t nFile, off_t nPos, void* pBuffer, size_t nLength );
size_t	write_pos( Devfd_t nFile, off_t nPos, const void* pBuffer, size_t nLength );
int	get_vnode( int nDev, ino_t nInode, void** pNode );
int	put_vnode( int nDev, ino_t nInode );



int	set_inode_deleted_flag( int nDev, ino_t nInode, bool bIsDeleted );
int	get_inode_deleted_flag( int nDev, ino_t nInode );
int	flush_inode( int nDev, ino_t nInode );
gid_t	getegid(void);
uid_t	geteuid(void);
gid_t	getfsgid(void);
uid_t	getfsuid(void);
gid_t	getgid(void);
status_t cached_read( Devfd_t nDev, off_t nBlockNum, void *pBuffer,
		      size_t nBlockCount, size_t nBlockSize );
status_t cached_write( Devfd_t nDev, off_t nBlockNum, const void *pBuffer,
		       size_t nBlockCount, size_t nBlockSize );

#ifdef __cplusplus
}
#endif

#endif	/* __NETBAS_BFS_H__ */
