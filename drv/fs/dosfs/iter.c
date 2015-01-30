/*
	Copyright 1999, Be Incorporated.   All Rights Reserved.
	This file may be used under the terms of the Be Sample Code License.
*/

typedef long long dr9_off_t;




#include "iter.h"
#include "dosfs.h"
#include "fat.h"
#include "util.h"

#define DPRINTF(a,b) if (debug_iter > (a)) printk b

CHECK_MAGIC(diri,struct diri,DIRI_MAGIC)


status_t  setup_device_cache( dev_t nDevice, int nFS, off_t nBlockCount ){return 0;}
//status_t  flush_cache_block( dev_t nDev, off_t nBlockNum ){return 0;}
status_t  flush_device_cache( dev_t nDevice, bool bOnlyLogBlocks ){return 0;}
//status_t  flush_locked_device_cache( dev_t nDevice, bool bOnlyLogBlocks ){return 0;}
status_t  shutdown_device_cache( dev_t nDevice ){return 0;}
status_t notify_node_monitors( int nEvent, dev_t nDevice, ino_t nOldDir, ino_t nNewDir, 
	ino_t nNode, const char* pzName, int nNameLen )
{
	printk("notify_node_monitors called\n");
	return 0;
}

int	set_inode_deleted_flag( int nDev, ino_t nInode, bool bIsDeleted )
{
	printk("%s called\n", __FUNCTION__);
	return 0;
}

status_t cached_read( void* nDev, off_t nBlockNum, void *pBuffer,
		      int nBlockCount, size_t nBlockSize )
 {
  register buffer_t *bp;
  int i=0;

 
 bp = bread(nDev, nBlockNum);
 if (bp)
 {
	 memcpy(pBuffer,bp->b_data,nBlockSize);
	 buf_release(bp);
 }
 else
	 return -1;


	 return 0;
 }

status_t cached_write( dev_t nDev, off_t nBlockNum, const void *pBuffer,
		       int nBlockCount, size_t nBlockSize )
 {
  register buffer_t *bp;
  int i=0;

 
 bp = bread(nDev, nBlockNum);
 if (bp)
 {
	 memcpy(bp->b_data,pBuffer,nBlockSize);
	 mark_buf_dirty(bp);
 }
 else
	 return -1;


	 return 0;
 }



#ifdef __MODULE__
unsigned long block_to_sector(unsigned long block, int blocksize)
{
	 unsigned long sector;
	sector = block*((blocksize+HD_SECTOR_SIZE_MASK)/HD_SECTOR_SIZE);
	return sector;
}
#endif


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

int mark_blocks_dirty( dev_prvi_t* dev, off_t blk, size_t nBlockCount )
{
	buffer_t *bp;	/* buffer pointer */
	bp = buf_find(dev->devno,blk);
	if (!bp)
	{
		return -1;
	}
	mark_buf_dirty(bp);
	return 0;
}


void release_cache_block(dev_prvi_t* fd, off_t blk_no)
{
	buffer_t* bp;
	bp = buf_find(fd->devno, blk_no);
	if (bp)
	{
		buf_release(bp);
	}
}

int	get_vnode( int nDev, ino_t nInode, void** pNode )
{
	printk("%s called\n", __FUNCTION__);
	* pNode = iget(nDev,nInode);
	return 0;
}

int	put_vnode(  dev_prvi_t* fd,ino_t i )
{
	inode_t *ino;
	int nDev=fd->devno;
	printk("%s called\n", __FUNCTION__);

	ino = ifind(nDev,i);
	if (ino)
	{
		iput(ino);
		return 0;
	}
	return -1;
	//BFS_TRACE;
}



static int _validate_cs_(nspace *vol, uint32 cluster, uint32 sector)
{
	if (sector < 0) return -1;

	if ((vol->fat_bits != 32) && IS_FIXED_ROOT(cluster)) { // fat12 or fat16 root
		if (sector >= vol->root_sectors)
			return -1;
		return 0;
	}

	if (sector >= vol->sectors_per_cluster) return -1;

	if (!IS_DATA_CLUSTER(cluster)) return -1;

	return 0;
}

static dr9_off_t _csi_to_block_(struct csi *csi)
{
	// presumes the caller has already called _validate_cs_ on the argument
	ASSERT(_validate_cs_(csi->vol, csi->cluster, csi->sector) == 0);

	if (IS_FIXED_ROOT(csi->cluster))
		return csi->vol->root_start + csi->sector;

	return csi->vol->data_start + 
		(dr9_off_t)(csi->cluster - 2)* csi->vol->sectors_per_cluster + 
		csi->sector;
}

int init_csi(nspace *vol, uint32 cluster, uint32 sector, struct csi *csi)
{
	int ret;
	if ((ret = _validate_cs_(vol,cluster,sector)) != 0)
		return ret;
	
	csi->vol = vol; csi->cluster = cluster; csi->sector = sector;
	
	return 0;
}

int iter_csi(struct csi *csi, int sectors)
{
	if (csi->sector == 0xffff) // check if already at end of chain
		return -1;

	if (sectors < 0)
		return -EINVAL;
	
	if (sectors == 0)
		return 0;
		
	if (IS_FIXED_ROOT(csi->cluster)) {
		csi->sector += sectors;
		if (csi->sector < csi->vol->root_sectors) 
			return 0;
	} else {
		csi->sector += sectors;
		if (csi->sector < csi->vol->sectors_per_cluster)
			return 0;
		csi->cluster = get_nth_fat_entry(csi->vol, csi->cluster, csi->sector / csi->vol->sectors_per_cluster);

		if ((int32)csi->cluster < 0) {
			csi->sector = 0xffff;
			return csi->cluster;
		}

		if (vIS_DATA_CLUSTER(csi->vol,csi->cluster)) {
			csi->sector %= csi->vol->sectors_per_cluster;
			return 0;
		}
	}

	csi->sector = 0xffff;
	
	return -1;
}


uint8 *csi_get_block(struct csi *csi)
{
	if (_validate_cs_(csi->vol, csi->cluster, csi->sector) != 0)
		return NULL;

	return get_cache_block(csi->vol->fd, _csi_to_block_(csi), csi->vol->bytes_per_sector);
}

status_t csi_release_block(struct csi *csi)
{
	if (_validate_cs_(csi->vol, csi->cluster, csi->sector) != 0)
		return -EINVAL;

	release_cache_block(csi->vol->fd, _csi_to_block_(csi));
	return( 0 );
}

status_t csi_mark_block_dirty(struct csi *csi)
{
	if (_validate_cs_(csi->vol, csi->cluster, csi->sector) != 0)
		return -EINVAL;

	return mark_blocks_dirty(csi->vol->fd, _csi_to_block_(csi), 1);
}

status_t csi_read_block(struct csi *csi, uint8 *buffer)
{
	if (_validate_cs_(csi->vol, csi->cluster, csi->sector) != 0)
		return -EINVAL;

	return cached_read(csi->vol->fd, _csi_to_block_(csi), buffer, 1, csi->vol->bytes_per_sector);
}

status_t csi_write_block(struct csi *csi, uint8 *buffer)
{
	if (_validate_cs_(csi->vol, csi->cluster, csi->sector) != 0)
		return -EINVAL;

	return cached_write(csi->vol->fd, _csi_to_block_(csi), buffer, 1, csi->vol->bytes_per_sector);
}

static void _diri_release_current_block_(struct diri *diri)
{
	if (diri->current_block) {
		if (diri->dirty) {
			csi_mark_block_dirty(&(diri->csi));
			DPRINTF(1, ("Writing updated directory entries\n"));
		}
		csi_release_block(&(diri->csi));
	}
}

uint8 *diri_init(nspace *vol, uint32 cluster, uint32 index, struct diri *diri)
{
	diri->magic = DIRI_MAGIC;
	diri->current_block = NULL;

	if (cluster >= vol->total_clusters + 2)
		return NULL;

	if (init_csi(vol,cluster,0,&(diri->csi)) != 0)
		return NULL;

	diri->dirty = false;	
	diri->starting_cluster = cluster;
	diri->current_index = index;
	if (index >= vol->bytes_per_sector / 0x20) {
		if (iter_csi(&(diri->csi), diri->current_index / (vol->bytes_per_sector / 0x20)) != 0)
			return NULL;
	}

	// get current sector
	diri->current_block = csi_get_block(&(diri->csi));

	if (diri->current_block == NULL)
		return NULL;

	return diri->current_block + (diri->current_index % (diri->csi.vol->bytes_per_sector / 0x20))*0x20;
}

int diri_free(struct diri *diri)
{
	if (check_diri_magic(diri, "diri_free")) return -EINVAL;

	diri->magic = ~DIRI_MAGIC; // trash magic number

	if (diri->current_block) {
		_diri_release_current_block_(diri);
		diri->current_block = NULL;
	}

	return 0;
}

uint8 *diri_current_entry(struct diri *diri)
{
	if (check_diri_magic(diri, "diri_current_entry")) return NULL;
	
	if (diri->current_block == NULL)
		return NULL;

	return diri->current_block + (diri->current_index % (diri->csi.vol->bytes_per_sector / 0x20))*0x20;
}

uint8 *diri_next_entry(struct diri *diri)
{
	if (check_diri_magic(diri, "diri_next_entry")) return NULL;
	
	if (diri->current_block == NULL)
		return NULL;

	if ((++diri->current_index % (diri->csi.vol->bytes_per_sector / 0x20)) == 0) {
		_diri_release_current_block_(diri);
		diri->current_block = NULL;
		diri->dirty = false;
		if (iter_csi(&(diri->csi), 1) != 0)
			return NULL;
		diri->current_block = csi_get_block(&(diri->csi));
		if (diri->current_block == NULL)
			return NULL;
	}
	
	return diri->current_block + (diri->current_index % (diri->csi.vol->bytes_per_sector / 0x20))*0x20;
}

uint8 *diri_rewind(struct diri *diri)
{
	if (check_diri_magic(diri, "diri_rewind")) return NULL;

	if (diri->current_index > (diri->csi.vol->bytes_per_sector / 0x20 - 1)) {
		if (diri->current_block)
			_diri_release_current_block_(diri);
		diri->dirty = false;
		diri->current_block = NULL;
		if (init_csi(diri->csi.vol, diri->starting_cluster, 0, &(diri->csi)) != 0)
			return NULL;
		diri->current_block = csi_get_block(&(diri->csi));
	}
	diri->current_index = 0;
	return diri->current_block;
}

void diri_mark_dirty(struct diri *diri)
{
	diri->dirty = true;
}
