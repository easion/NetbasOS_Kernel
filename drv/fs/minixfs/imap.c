#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>
#include <net/inet.h>
#include "minixfs.h"


void wipe_inode(inoptr)
register inode_t *inoptr;	/* the inode to be erased */
{
/* Erase some fields in the inode.  This function is called from alloc_inode()
 * when a new inode is to be allocated, and from truncate(), when an existing
 * inode is to be truncated.
 */

  mfs2_inode_mem_t * rip;

  rip = inoptr->i_private_data;


  register int i;

  inoptr->i_size = 0;
  //inoptr->i_update = ATIME | CTIME | MTIME;	/* update all times later */
  //inoptr->i_dirt = DIRTY;
  for (i = 0; i < V2_NR_TZONES; i++) rip->i_zone[i] = NOZONE;

}



/*===========================================================================*
 *				alloc_inode				     *
 *===========================================================================*/
inode_t *alloc_inode(mount_t *dev,mode_t  bits)
			/* device on which to allocate the inode */
			/* mode of the inode */
{
/* Allocate a free inode on 'dev', and return a pointer to it. */

  register mfs2_inode_mem_t *rip;
  register mfs_sb_t *sp;
  int major, minor, inumb;
  bit_t b;
  inode_t *inoptr;

  sp = dev->m_private_data;
  //sp = get_super(dev);	/* get pointer to super_block */
  //if (sp->s_rd_only) {	/* can't allocate an inode on a read only device. */
	//err_code = EROFS;
	//return(NIL_INODE);
 // }

  /* Acquire an inode from the bit map. */
  b = alloc_bit(dev, IMAP, sp->s_isearch);
  if (b == NO_BIT) {
	//err_code = ENFILE;
	//major = (int) (sp->s_dev >> MAJOR) & BYTE;
	//minor = (int) (sp->s_dev >> MINOR) & BYTE;
	printf("Out of i-nodes on %sdevice %d/%d\n",
		 "root " , major, minor);
	return(NULL);
  }
  sp->s_isearch = b;		/* next time start here */
  inumb = (int) b;		/* be careful not to pass unshort as param */

#if 1
  /* Try to acquire a slot in the inode table. */
  if ((inoptr = iget(NO_DEV, inumb)) == NULL) {
	/* No inode table slots available.  Free the inode just allocated. */
	free_bit(dev, IMAP, b);
  } else {
	/* An inode slot is available. Put the inode just allocated into it. */
	inoptr->i_mode = bits;		/* set up RWX bits */
	inoptr->i_uid = 0;	/* file's uid is owner's */
	//inoptr->i_gid = fp->fp_effgid;	/* ditto group id */
	inoptr->i_dev = dev->m_dev->devno;		/* mark which device it is on */
	inoptr->i_super = dev;			/* pointer to super block */
	rip = (void*)kmalloc(sizeof(mfs2_inode_mem_t),0);//
	rip->i_ndzones = sp->s_ndzones;	/* number of direct zones */
	rip->i_nindirs = sp->s_nindirs;	/* number of indirect zones per blk*/
	rip->i_nlinks = 0;		/* initial no links */

	inoptr->i_private_data=rip;

	/* Fields not cleared already are cleared in wipe_inode().  They have
	 * been put there because truncate() needs to clear the same fields if
	 * the file happens to be open while being truncated.  It saves space
	 * not to repeat the code twice.
	 */
	wipe_inode(inoptr);
  }
#endif
  return(inoptr);
}

 void free_inode(dev, inumb)
mount_t* dev;			/* on which device is the inode */
ino_t inumb;			/* number of inode to be freed */
{
/* Return an inode to the pool of unallocated inodes. */

  register mfs_sb_t *sp;
  bit_t b;


  sp = dev->m_private_data;


  /* Locate the appropriate super_block. */
  //sp = get_super(dev);
  if (inumb <= 0 || inumb > sp->s_ninodes) return;
  b = inumb;
  free_bit(sp, IMAP, b);
  if (b < sp->s_isearch) sp->s_isearch = b;
}


/* Notice: zone_t will only support V2 */
#define HOWMUCH	(BLOCKSIZ / (zone_t))

#define get_zone(data, index) (((zone_t *)data)[index])



#define icopy(dest, src) \
do{ \
	dest->i_mode = src->i_mode; \
	dest->i_uid = src->i_uid; \
	dest->i_size = src->i_size; \
	dest->i_atime = src->i_atime; \
	dest->i_mtime = src->i_mtime; \
	dest->i_ctime = src->i_ctime; \	
}while(0)

#define iprvicopy(dest, src) \
do{ \
	dest->i_nlinks = src->i_nlinks; \
	dest->i_gid = src->i_gid; \
	dest->i_gid = src->i_gid; \
	for(i = 0; i < V2_NR_TZONES; i++) \
		dest->i_zone[i] = src->i_zone[i]; \
}while(0)



/*
 * inoptr is not locked
 */
static void iread(inode_t * inoptr)
{
	int i;
	ino_t ino;
	block_t block;
	buffer_t * bh;
	mfs_sb_t * sp;
	mfs2_inode_t * tmp;
	mfs2_inode_mem_t * prvi;

	prvi = kmalloc(sizeof(mfs2_inode_mem_t),0);
	if (!prvi)
	{
		return ;
	}

	ino = inoptr->i_number;

	assert(inoptr->i_super);

	sp = inoptr->i_super->m_private_data;

	/* 2 for boot and super block */
	block = 2 + sp->s_imap_blocks + sp->s_zmap_blocks;	


	/* 0 inode number is saved, but inode, use it */
	block += (ino - 1) / INODES_PER_BLOCK;

	bh = bread(inoptr->i_super->m_dev, BLOCK_TO_SECTOR(block));
	if(!bh){
		panic("error happen when read inode%d, on block%d!\n",ino,block);
		return;
	}

	/* "- 1" for number-0 which is not-used */
	tmp = (mfs2_inode_t *)
		(&bh->b_data[((ino - 1) % INODES_PER_BLOCK) * V2_INODE_SIZE]);

	icopy(inoptr, tmp);
	iprvicopy(prvi, tmp);

	prvi->i_ndzones = V2_NR_DZONES;
	prvi->i_nindirs = V2_INDIRECTS;

	inoptr->i_private_data=prvi;

	//printf("ino%d blk%d mode is %x i_size %x\n",ino,block, tmp->i_mode,tmp->i_size);

	//inoptr->i_sp = sp;

	buf_release(bh);
}

/*
 * inoptr is not locked
 */
static void iwrite(inode_t * inoptr)
{
	int i;
	ino_t ino;
	block_t block;
	buffer_t * bh;
	mfs_sb_t * sp;
	mfs2_inode_t * tmp;

	ino = inoptr->i_number;

	sp = inoptr->i_super->m_private_data;

	block = 2 + sp->s_imap_blocks + sp->s_zmap_blocks;

	/* number-0 is reserved */
	block += (ino - 1) / INODES_PER_BLOCK;
	bh = bread(inoptr->i_super->m_dev, BLOCK_TO_SECTOR(block));
	if(!bh){
		panic("error happen when write inode!\n");
		return;
	}

	tmp = (mfs2_inode_t *)
		(&bh->b_data[((ino - 1) % INODES_PER_BLOCK) * V2_INODE_SIZE]);

#if 0
	if(inoptr->i_update){
		time_t now;

		now = curclock();

		if(inoptr->i_update & ATIME)
			inoptr->i_atime = now;

		if(inoptr->i_update & MTIME)
			inoptr->i_mtime = now;

		if(inoptr->i_update & CTIME)
			inoptr->i_ctime = now;
	}
#endif
	icopy(tmp, inoptr);



	mark_buf_dirty(bh);
	buf_release(bh);



	//inoptr->i_update = 0;
}

int minixfs_inorw(mount_t *mp, inode_t *inode, int cmd)
{
	mfs2_inode_mem_t * rip;

	rip = inode->i_private_data;

	if (cmd==0)
	{
		iread(inode);
	}
	else if (cmd==1){
		iwrite(inode);
	}
	else if (cmd==2){
		//when iput
		if (rip->i_nlinks == 0) {
			/* i_nlinks == 0 means free the inode. */
			//truncate(rip);	/* return all the disk blocks */
			//rip->i_mode = I_NOT_ALLOC;	/* clear I_TYPE field */
			inode->i_dirt = 1;
			free_inode(inode->i_super, inode->i_number);
		}
		//kfree(inode->i_private_data);
	}

	return 0;
}

block_t mfs_bmap(inode_t * inoptr, off_t pos)
{
	block_t block_pos, block;
	zone_t zone_pos, zone, excess;
	int scale, block_off, dzones, nindirs, index;
	buffer_t * bh;
	mfs_sb_t *sp;
	mfs2_inode_mem_t * prvi;

	prvi = inoptr->i_private_data;

	assert(prvi);

	sp = inoptr->i_super->m_private_data;

	scale = sp->s_log_zone_size;
	block_pos = pos / BLOCKSIZ;
	zone_pos = block_pos >> scale;
	block_off = block_pos - (zone_pos << scale);

	dzones = V2_NR_DZONES;//inoptr->i_ndzones;
	nindirs =V2_INDIRECTS;// inoptr->i_nindirs;

	if(zone_pos < dzones){
		 zone = prvi->i_zone[zone_pos];
		 if(zone == NOZONE)
			 return NOBLOCK;

		 block = (zone << scale) + block_off;

		 return block;
	}

	excess = zone_pos - dzones;

	/* single indirect */
	if(excess < nindirs)
		zone = prvi->i_zone[dzones];
	else{
		/* double indirect */
		zone = prvi->i_zone[dzones + 1];
		if(zone == NOZONE)
			return NOBLOCK;

		/* double indirect offset */
		excess -= nindirs;

		block = zone << scale;
		bh = bread(inoptr->i_super->m_dev, BLOCK_TO_SECTOR(block));
		if(!bh){
			printk("error when reading block\n");
			return NOBLOCK;
		}

		index = (excess / nindirs);
		zone = get_zone(bh->b_data, index);

		buf_release(bh);

		excess %= nindirs;
	}

	if(zone == NOZONE)
		return NOBLOCK;

	block = zone << scale;

	bh = bread(inoptr->i_super->m_dev, BLOCK_TO_SECTOR(block));
	if(!bh){
		printk("error when reading block\n");
		return NOBLOCK;
	}

	zone = get_zone(bh->b_data, excess);

	buf_release(bh);

	if(zone == NOZONE)
		return NOBLOCK;

	block = (zone << scale) + block_off;

	return block;
}


block_t read_map(inode_t *inoptr , off_t position)
	/* ptr to inode to map from */
		/* position in file whose blk wanted */
{
/* Given an inode and a position within the corresponding file, locate the
 * block (not zone) number in which that position is to be found and return it.
 */

	register buffer_t *bp;
	register zone_t z;
	int scale, boff, dzones, nr_indirects, index, zind, ex;
	block_t b;
	long excess, zone, block_pos;
	mfs_sb_t *sp;
	mfs2_inode_mem_t * rip;

	rip = inoptr->i_private_data;

	sp = inoptr->i_super->m_private_data; 
  scale = sp->s_log_zone_size;	/* for block-zone conversion */
  block_pos = position/V2_BLOCK_SIZE;	/* relative blk # in file */
  zone = block_pos >> scale;	/* position's zone */
  boff = (int) (block_pos - (zone << scale) ); /* relative blk # within zone */
  dzones = rip->i_ndzones;
  nr_indirects = rip->i_nindirs;

  /* Is 'position' to be found in the inode itself? */
  if (zone < dzones) {
	zind = (int) zone;	/* index should be an int */
	z = rip->i_zone[zind];
	if (z == NOZONE) return(NOBLOCK);
	b = ((block_t) z << scale) + boff;
	return(b);
  }

  /* It is not in the inode, so it must be single or double indirect. */
  excess = zone - dzones;	/* first Vx_NR_DZONES don't count */

  if (excess < nr_indirects) {
	/* 'position' can be located via the single indirect block. */
	z = rip->i_zone[dzones];
  } else {
	/* 'position' can be located via the double indirect block. */
	if ( (z = rip->i_zone[dzones+1]) == NOZONE) return(NOBLOCK);
	excess -= nr_indirects;			/* single indir doesn't count*/
	b = (block_t) z << scale;
	bp = bread(inoptr->i_super->m_dev, BLOCK_TO_SECTOR(b));	/* get double indirect block */
	index = (int) (excess/nr_indirects);
	z = rd_indir(bp, index);		/* z= zone for single*/
	buf_release(bp);		/* release double ind block */
	excess = excess % nr_indirects;		/* index into single ind blk */
  }

  /* 'z' is zone num for single indirect block; 'excess' is index into it. */
  if (z == NOZONE) return(NOBLOCK);
  b = (block_t) z << scale;			/* b is blk # for single ind */
  bp = bread(inoptr->i_super->m_dev, BLOCK_TO_SECTOR(b));	/* get single indirect block */
  ex = (int) excess;				/* need an integer */
  z = rd_indir(bp, ex);				/* get block pointed to */
  buf_release(bp);		/* release single indir blk */
  if (z == NOZONE) return(NOBLOCK);
  b = ((block_t) z << scale) + boff;
  return(b);
}


off_t minixfs_bmap(inode_t* inode, int position, int create)
{
	off_t blk = 0;// + position/CDROM_SECTOR_SIZE;
	mfs2_inode_mem_t * rip;
	mfs_sb_t *sp;
	int r,scale;


	if (position<0)
		return (off_t)-1;

	if ( inode->i_size <= position )
		return (off_t)-1;

	//blk = mfs_bmap(inode,  position);
	blk = read_map(inode,  position);

  /* Is another block available in the current zone? */
	if (blk==NOBLOCK)
	{
		zone_t z, zone_size;
		block_t  base_block;

		rip = inode->i_private_data;
		if (!create)
		{
			return -1;
		}


	/* Choose first zone if possible. */
	/* Lose if the file is nonempty but the first zone number is NO_ZONE
	 * corresponding to a zone full of zeros.  It would be better to
	 * search near the last real zone.
	 */
	if (rip->i_zone[0] == NOZONE) {
		sp = inode->i_super->m_private_data;
		//sp = rip->i_sp;
		z = sp->s_firstdatazone;
	} else {
		z = rip->i_zone[0];	/* hunt near first zone */
	}
	if ( (z = alloc_zone(inode->i_super, z)) == NOZONE) return(-1);
	if ( (r = write_map(inode, position, z)) != OK) {
		free_zone(inode->i_super, z);
		return(-1);
	}

	/* If we are not writing at EOF, clear the zone, just to be safe. */
	if ( position != inode->i_size) clear_zone(rip, position, 1);
	scale = sp->s_log_zone_size;
	base_block = (block_t) z << scale;
	zone_size = (zone_t) V2_BLOCK_SIZE << scale;
	blk = base_block + (block_t)((position % zone_size)/V2_BLOCK_SIZE);

	}

	//printf("minixfs_bmap blk=%d, off %d\n",blk,position);
	return BLOCK_TO_SECTOR(blk);
}


