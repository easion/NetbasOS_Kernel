/*
**     (R)Jicama OS
**     Microsoft File Allocation Table
**     Copyright (C) 2003 DengPingPing
*/


#include <drv/drv.h>
#include <drv/fs.h>
#include <assert.h>
#include <drv/buffer.h>
#include "fat.h"


u32_t msdos_next(mount_t *mp, u32_t cluster)
{
	unsigned int next = 0;
	u16_t fat12_nextc(u16_t actual);

	switch (mp->m_magic)
	{
	case FAT12_MAGIC:
		#if 0
			next = msdos_12next(mp->m_dev, cluster);			
		#else
			next = fat12_nextc((unsigned short)cluster);
		#endif

		break;
	case FAT16_MAGIC:
		next = msdos_16next(mp->m_dev, cluster);
		break;
	case FAT32_MAGIC:
		next = msdos_32next(mp->m_dev, cluster);
		break;	
	default:
		panic("bad magic at hd0\n");
	break;
	}

	return next; 
}



int msdos_setc(mount_t *mp, unsigned int cluster, unsigned int c)
{
	switch (mp->m_magic)
	{
	case FAT12_MAGIC:
			fat12_setc(cluster, c);
		break;
	case FAT16_MAGIC:
		fat16_setc(mp->m_dev, cluster, c);
		break;
	case FAT32_MAGIC:
		fat32_setc(mp->m_dev, cluster, c);
		break;	
	default:
		panic("bad magic at hd0\n");
	break;
	}
	return 0; 
}

int msdos_set_end(mount_t *mp, unsigned int cluster)
{

	switch (mp->m_magic)
	{
	case FAT12_MAGIC:
				fat12_setc(cluster, EOF_FAT12);
		break;
	case FAT16_MAGIC:
		fat16_setc(mp->m_dev, cluster, EOF_FAT16);
		break;
	case FAT32_MAGIC:
		fat32_setc(mp->m_dev, cluster, EOF_FAT32);
		break;	
	default:
		panic("bad magic at hd0\n");
	break;
	}
	return 0;
}

bool msdos_eof(mount_t *mp, unsigned int cluster)
{
	bool eof = 0;

	switch (mp->m_magic)
	{
	case FAT12_MAGIC:
		if ((unsigned short)cluster>=0x0FF8)
		{
			eof = 1;
		}
		break;
	case FAT16_MAGIC:
		if ((unsigned short)cluster>=0xFFF8)
		{
			eof = 1;
		}
		break;
	case FAT32_MAGIC:
		if (cluster>=0x0FFFFFF8)
		{
			eof = 1;
		}
		break;	
	default:
		panic("bad magic at hd0\n");
	break;
	}
	return eof; 
}

unsigned int msdos_cluster2sec(mount_t *mp, unsigned int cluster)
{
	unsigned int sec = 0;
	struct msdos_info *info = msdos_dev2info(mp->m_dev->devno);

	assert (info != NULL);	

	if(msdos_eof(mp, cluster) ){
		kprintf("msdos_cluster2sec: cluster %d eof\n",cluster);
		return 0;	}

	if(cluster<0 || cluster > info->fat_entries){
		panic("%x: is a Bad Cluster! max is %x\n",cluster,  info->fat_entries);
		//return 0;//
	}

	switch (mp->m_magic)
	{
	case FAT12_MAGIC:
		if ( cluster > FAT12_ROOT ){
			sec = info->blk_base+(cluster-2) * info->cluster_size;
			//printk("info->cluster_size is %d sec=%d\n", info->cluster_size, sec);
			return sec;
		}    
		break;

	case FAT16_MAGIC:	
		if ( cluster > FAT12_ROOT ){
			sec = info->blk_base+(cluster-2) * info->cluster_size;			
			return sec;
		}
    
		break;

		case FAT32_MAGIC:
		if ( cluster > 1 ){
			sec = info->blk_base+(cluster-2) * info->cluster_size;
			return sec;
		}
		panic("FAT32_MAGIC error%d\n", cluster);
		break;	
	default:
		panic("bad magic at hd0\n");
	break;
	}
	sec = info->fat_root;  //root sector
	return sec; 
}

static unsigned long last_fat = 5;

int how_many_cluster(mount_t *mp,u32_t start )
{
	u32_t c = 1, cl;

	// Calculate the size of the directory... it's not the root!
	cl = start;
	while( fat_next_useable(mp, &cl, cl)==TRUE )c++;

	return( c );
}

int fat_pos_value(mount_t *mp,const u32_t cluster, int left)
{
	int i=0;
	u32_t tmp=cluster;	

	while( fat_next_useable(mp, &tmp, tmp) == TRUE && i<left)
	{
		i++;
	}
	return (int)tmp;
}
//这个函数有问题

int fat_add_tail(mount_t* mp, int start)
{
	int i=0;
	u32_t t, end_cluster=start;
	int new_cluster;

	while (1)
	{
		t = msdos_next(mp, end_cluster );

		if( (t == 0) || msdos_eof(mp, t) )
		{
			break;	
		}
		else{
			end_cluster=t;
			i++;
		}

	}

	new_cluster = fat_find_free(mp);
	if(!new_cluster)return 0;

	//printf("end_cluster:%d - %d - %d\n", i,end_cluster,new_cluster);

	msdos_setc(mp, end_cluster,new_cluster);
	msdos_set_end(mp, new_cluster);
	return new_cluster;
 }

int fat_free_file (mount_t *mp, long start_cluster)
{
	u32_t last_cluster;
	u32_t next_cluster = start_cluster;
	
	// Loop until end of chain
	while ( !msdos_eof(mp, next_cluster) && (next_cluster!=0x00000000) )
	{
		last_cluster = next_cluster;

		// Find next link
		next_cluster = msdos_next(mp, next_cluster );//

		// Clear last link
		msdos_setc(mp, last_cluster, 0x00000000);
	}

	return TRUE;
}


int fat_find_free (mount_t *mp)
{
	unsigned int c= last_fat;

	while(!msdos_eof(mp, c)) {
		if(msdos_next( mp, c ) == 0)  break;
		c++;
	}

	if(msdos_eof(mp, c)) {
		kprintf("msdos_find_fat32(): no free blocks\n");
		return (0);
	}

	last_fat = c;
	return (c);
}

int fat_truncate(mount_t *mp, unsigned long cluster )
{
	u32_t tmp;
	int i, n=how_many_cluster(mp, cluster);

	for (i=n-1; i>=0; i--)
	{
		tmp = fat_pos_value(mp, cluster, i);
		if (msdos_eof(mp,tmp))
		{
			kprintf("fat_truncate(): pos%d overd (%d)\n",i, n);
			continue;
		}
		msdos_setc(mp, tmp, 0x000000);
	}
	return 0;	
}

bool fat_next_useable(mount_t *mp, u32_t *next, u32_t actual)
{
	assert(mp!=NULL);
	assert(next!=NULL);

	*next = msdos_next(mp, actual );

	if( (*next == 0) || msdos_eof(mp, *next) )
		return( FALSE );
	else
		return( TRUE );
}

u32_t seek_cluster(mount_t *mp, u32_t start, u32_t nr )
{
	u32_t cl = start;
	u32_t  blks = nr;

	assert(cl );

	switch (mp->m_magic)
	{
	case FAT12_MAGIC:	
	case FAT16_MAGIC:
		if (start == FAT12_ROOT)
		{
			return FAT12_ROOT;
		}
		break;
	case FAT32_MAGIC:
		break;	
	default:
		panic("seek_cluster(): bad magic \n");
	break;
	}

   while (blks){
	  cl = msdos_next(mp, cl);

	  if(msdos_eof(mp, cl) || cl == 0) 
		  {
		  //kprintf("seekc %d to %d err\n", start, nr);
		  break;
		  }
	  blks--;
	 }
	return( cl );
}
