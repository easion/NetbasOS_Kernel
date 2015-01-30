
#define BFS_TRACE 		do{\
	kprintf("%s: called at line%d\n", __FUNCTION__,__LINE__);\
}\
while (0)

size_t	read_pos( int nFile, off_t nPos, void* pBuffer, size_t nLength )
{
}

size_t	write_pos( int nFile, off_t nPos, const void* pBuffer, size_t nLength )
{
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

int setup_device_cache( dev_t nDevice, int nFS, off_t nBlockCount )
{
	BFS_TRACE;
	return 0;
}

int cached_read( dev_t nDev, off_t nBlockNum, void *pBuffer,
		      size_t nBlockCount, size_t nBlockSize )
{
	BFS_TRACE;
}

int cached_write( dev_t nDev, off_t nBlockNum, const void *pBuffer,
		       size_t nBlockCount, size_t nBlockSize )
{
	BFS_TRACE;
}

int  flush_cache_block( dev_t nDev, off_t nBlockNum )
{
	BFS_TRACE;
}

int  flush_locked_device_cache( dev_t nDevice, bool bOnlyLogBlocks )
{
	BFS_TRACE;
}

int set_blocks_info( dev_t nDev, off_t* panBlocks, size_t nCount, 
int bDoClone, void (*func)(long unsigned int a,  int b, void*c), void* pArg )
{
	BFS_TRACE;
}

void* get_cache_block( dev_t nDev, off_t nBlockNum, size_t nBlockSize )
{
	BFS_TRACE;
}
int	set_inode_deleted_flag( int nDev, ino_t nInode, bool bIsDeleted )
{
	BFS_TRACE;
}

void* get_empty_block( dev_t nDev, off_t nBlockNum, size_t nBlockSize )
{
	BFS_TRACE;
}

void  release_cache_block( dev_t nDev, off_t nBlockNum )
{
	BFS_TRACE;
}

int  flush_device_cache( dev_t nDevice, bool bOnlyLogBlocks )
{
	BFS_TRACE;
}



