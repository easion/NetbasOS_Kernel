/*
* 
*Jicama OS  
* Copyright (C) 2001-2003  DengPingPing      All rights reserved.   
*/	

#include <drv/drv.h>
#include <drv/fs.h>
#include "fat.h"
#include <assert.h>
#include <drv/log.h>
u32_t seek_cluster(mount_t *mp, u32_t start, u32_t nr );

unsigned int msdos_dir_size(mount_t *mp, unsigned int cluster)
{
	unsigned int sz=0;
	u32_t c = 1, cl= cluster;
	struct msdos_info *info = msdos_dev2info(mp->m_dev->devno);

	if (info == NULL)
	{
		panic("msdos_dir_size(): info error at %x\n", mp->m_dev);
	}

	switch (mp->m_magic)
	{
	case FAT12_MAGIC:
		{			
			if( cluster < 2)	{return(info->dir_entries * 32);}
			//while( fat12_next_useable(&cl, cl)==TRUE )c++;
			//return( c* info->blk_size );
				}
		break;
	case FAT16_MAGIC:
			if( cluster < 2)	{return(info->dir_entries  * 32);}
		break;
	case FAT32_MAGIC:
		break;	
	default:
		panic("bad magic at hd0\n");
	break;
	}

	while( fat_next_useable(mp, &cl, cl)==TRUE )c++;
	sz =	( c* info->blk_size );	
	//kprintf("cluster is c%d %d sz%d\n",cluster, c, sz);
	return sz; 
}






int fat32_dirsz(mount_t *mp, u32_t start)
{
	u32_t c = 1, cl;
	int sz;
	struct msdos_info *info = msdos_dev2info(mp->m_dev);

	if (info == NULL)
	{
		panic("info error\n");
	}
	cl = start;

	while( fat_next_useable(mp, &cl, cl)==TRUE )c++;
	sz =	( c * info->blk_size );	

	return sz;
}


off_t fat_bmap(inode_t* inode, int offset, int writemode)
{
	u32_t sec, c, blk, blk_offset;
	int begin_block=inode->i_number;
	mount_t *mp;
	int num;
	struct msdos_info *info;

	mp = get_superblock(inode->i_dev);
	assert (mp != NULL);	
	info = msdos_dev2info(inode->i_dev);
	num = how_many_cluster(mp, begin_block);

	assert (info != NULL );

	if (begin_block < FAT12_ROOT && !writemode)
	{
		syslog(4,"fat_bmap(): root%d small than root @ dev%x\n", begin_block, inode->i_dev);
		return (off_t)-1;
	}

	blk=offset/ info->blk_size;

	if (FAT12_ROOT==begin_block && 
		(mp->m_magic == FAT12_MAGIC || mp->m_magic==FAT16_MAGIC))
	{
		sec= msdos_cluster2sec(mp, begin_block)+blk; 
		//printf("fat_bmap try map %d sec=%d\n",inode->i_number,sec );
		return sec;
	}


	if (begin_block>1 && blk >= num&&!writemode)
	{
		#if 0
		kprintf("fat_bmap(): begin_block=%d blk%d error, num is %d info->blk_size is %d\n",
			begin_block, blk,num, info->blk_size);
		#endif
		return 0;
	}

	blk_offset  = (offset %info->blk_size)/HD_SECTOR_SIZE;
#if 0
	/*block at a new file*/
	if(!begin_block&&writemode){
		begin_block = fat_find_free(mp);
		if(!begin_block)return 0;
		inode->i_number = begin_block;
		//kprintf("alloc at %x ", begin_block);
		sec= msdos_cluster2sec(mp, begin_block); 
		return sec;
	}
#endif
	c  = seek_cluster(mp, begin_block, blk );
	
	if(msdos_eof(mp, c) && writemode){
		c = fat_add_tail(mp, begin_block);
		//kprintf("fat_bmap(): add block %x\n", c);
		if(!c)return 0;
	}

	sec= msdos_cluster2sec(mp, c); 
	//if(blk_offset != 0)
	//printf("bmap %d sec is %d offset is %d\n",c,  sec, blk_offset);
	//return (sec+blk_offset);
	return (sec);
}



