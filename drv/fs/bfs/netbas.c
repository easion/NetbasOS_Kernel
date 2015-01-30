
#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>
#include "netbas_bfs.h"
static void bfs_hook();
static void remove_bfs_hook();


//netbas utils

int bfs_probe(const mount_t *mp );
int bfs_mount(mount_t *mp, void *_data );
int bfs_unmount(mount_t *mp, void *pVolume );
int bfs_fread(file_t * filp, char * buf, int count);
int bfs_fwrite(file_t * filp, char * buf, int count);
inode_t* bfs_opendir(inode_t * inode, char *name);
int bfs_mkdir(inode_t* n, char* dname,int mode);
int bfs_readdir2(file_t* filp, vfs_dirent_t *dirent);



int dll_main(char **args)
{
	int error=0;

	kprintf("bfs Module Running ...\n");

	bfs_hook();

	//panic("error = %d\n", error);
	return error;
}


int dll_destroy()
{
	remove_bfs_hook();
	return 0;
}

/********************************************/
int bfs_closedir(inode_t* old); //
int bfs_openfile(file_t* ); //
int bfs_closefile(file_t* old); //
int bfs_unlink2 (inode_t* n, unsigned char* file);

static  fs_dev_ops_t _bfs=
{
	fs_name: "bfs",
	fs_copyright:"MIT License",
	fs_author:"Haiku(axeld@pinc-software.de)",
	fs_bmap: NULL,
	fs_opendir: bfs_opendir,
	fs_closedir:bfs_closedir,
	fs_openfile:bfs_openfile,
	fs_releasefile:bfs_closefile,

	fs_readdir: bfs_readdir2,
	fs_probe:bfs_probe,
	fs_mount:bfs_mount,
	fs_unmount:bfs_unmount,
	fs_read:bfs_fread,
	fs_write:bfs_fwrite,
	fs_mkdir:bfs_mkdir,
	fs_unlink:bfs_unlink2,
};


static void bfs_hook()
{
	install_fs(&_bfs);
}

static void remove_bfs_hook()
{
	deinstall_fs(&_bfs);
}



size_t	read_pos( Devfd_t nFile, off_t nPos, void* pBuffer, size_t nLength )
{
	return dev_read(nFile,nPos/512,pBuffer,nLength);
	BFS_TRACE;
}

size_t	write_pos( Devfd_t nFile, off_t nPos, const void* pBuffer, size_t nLength )
{
	return dev_write(nFile,nPos/512,pBuffer,nLength);
	BFS_TRACE;
}

int	get_vnode( int nDev, ino_t nInode, void** pNode )
{
	BFS_TRACE;
}

int	put_vnode( int nDev, ino_t nInode )
{
	BFS_TRACE;
}



/********************************************/

status_t shutdown_device_cache( Devfd_t nDevice )
{
	BFS_TRACE;
}

int setup_device_cache( Devfd_t nDevice, int nFS, off_t nBlockCount )
{
	BFS_TRACE;
	return 0;
}

int cached_read( Devfd_t nDev, off_t nBlockNum, void *pBuffer,
		      size_t nBlockCount, size_t nBlockSize )
{
	BFS_TRACE;
}

int cached_write( Devfd_t nDev, off_t nBlockNum, const void *pBuffer,
		       size_t nBlockCount, size_t nBlockSize )
{
	BFS_TRACE;
}

int  flush_cache_block( Devfd_t nDev, off_t nBlockNum )
{
	BFS_TRACE;
}

int  flush_locked_device_cache( Devfd_t nDevice, bool bOnlyLogBlocks )
{
	BFS_TRACE;
}

status_t set_blocks_info( Devfd_t nDev, off_t* panBlocks, size_t nCount, bool bDoClone,
	void (*func)(long unsigned int,  int, void*), void* pArg )
{
	BFS_TRACE;
}


int	set_inode_deleted_flag( int nDev, ino_t nInode, bool bIsDeleted )
{
	BFS_TRACE;
}

gid_t	getegid(void)
{
	return 0;
}

uid_t	geteuid(void)
{
	return 0;
}

void* get_empty_block( Devfd_t nDev, off_t nBlockNum, size_t nBlockSize )
{
	BFS_TRACE;
}



int  flush_device_cache( Devfd_t nDevice, bool bOnlyLogBlocks )
{
	BFS_TRACE;
}



#if 1

#define NTFS_TIME_OFFSET ((bigtime_t)(369*365 + 89) * 24 * 3600 * 10000000)

bigtime_t bfs_ntutc2unixutc( bigtime_t ntutc )
{
	return ( ntutc - NTFS_TIME_OFFSET ) * 10LL;
}

bigtime_t bfs_unixutc2ntutc( bigtime_t t )
{
	return ( t / 10LL ) + NTFS_TIME_OFFSET;
}

#undef NTFS_TIME_OFFSET


void release_cache_block(Devfd_t fd, off_t blk_no)
{
	buffer_t* bp;
	bp = buf_find(fd, blk_no);
	if (bp)
	{
		buf_release(bp);
	}
}

unsigned long block_to_sector(unsigned long block, int blocksize)
{
	 unsigned long sector;
	sector = block*((blocksize+HD_SECTOR_SIZE_MASK)/HD_SECTOR_SIZE);
	return sector;
}



void *get_cache_block(dev_prvi_t* fd, off_t blk_no, size_t blk_size)
{
	buffer_t* bp;

	assert(blk_no>=0);

	blk_no = block_to_sector(blk_no,  blk_size);
	bp = bread(fd, blk_no);

	if (bp)
	{
		//printk("data is %s\n", bp->b_data);
		return bp->b_data;
	}
	printk("get_cblock(): dev%x blk_no%x not exist\n", fd, blk_no);
	return NULL;
}

void mark_dirty( Devfd_t dev, off_t blk, size_t nBlockCount )
{
	buffer_t *bp;	/* buffer pointer */
	bp = buf_find(dev,blk);
	if (!bp)
	{
		return ;
	}
	mark_buf_dirty(bp);
}
#endif
