
/************************************************************
  Copyright (C), 2003-2010, Netbas OS Project.
  FileName: 
  Author:        Version :          Date:
  Description:    
  Version:        
  Function List:   
  History:         
      Easion   2010/2/6     1.0     build this moudle  
***********************************************************/
#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>
#include <errno.h>
#include <net/inet.h>
#include "minixfs.h"
#define CHAR_BIT 8

#define BITCHUNK_BITS	(usizeof(bitchunk_t) * CHAR_BIT)
#define BITS_PER_BLOCK	(BITMAP_CHUNKS * BITCHUNK_BITS)


zone_t rd_indir(bp, index)
buffer_t *bp;			/* pointer to indirect block */
int index;			/* index into *bp */
{
/* Given a pointer to an indirect block, read one entry.  The reason for
 * making a separate routine out of this is that there are four cases:
 * V1 (IBM and 68000), and V2 (IBM and 68000).
 */

  mount_t *mp; //
  mfs_sb_t *sp; //
  zone_t  *b_v2_ind;
  zone_t zone;			/* V2 zones are longs (shorts in V1) */

  b_v2_ind = bp->b_data;

  mp = get_superblock(bp->b_dev);	/* need super block to find file sys type */

  if (!mp)
  {
	  panic("check file system");
  }
  sp = mp->m_private_data;

  /* read a zone from an indirect block */
 
	zone = (zone_t) conv4( (long) b_v2_ind[index]);

  if (zone != NOZONE &&
		(zone < (zone_t) sp->s_firstdatazone || zone >= sp->s_zones)) {
	printf("Illegal zone number %ld in indirect block, index %d\n",
	       (long) zone, index);
	panic("check file system");
  }
  return(zone);
}

void wr_indir(bp, index, zone)
buffer_t *bp;			/* pointer to indirect block */
int index;			/* index into *bp */
zone_t zone;			/* zone to write */
{
/* Given a pointer to an indirect block, write one entry. */

  mount_t *mp; //
  mfs_sb_t *sp; //
  zone_t  *b_v2_ind;

  mp = get_superblock(bp->b_dev);	/* need super block to find file sys type */
  sp = mp->m_private_data;
  b_v2_ind = bp->b_data;

  /* write a zone into an indirect block */

	b_v2_ind[index] = (zone_t)  conv4( (long) zone);
}




bit_t alloc_bit(mount_t *mp, int map, bit_t origin)
		/* the filesystem to allocate from */
			/* IMAP (inode map) or ZMAP (zone map) */
			/* number of bit to start searching at */
{
/* Allocate a bit from a bit map and return its bit number. */

  block_t start_block;		/* first bit block */
  bit_t map_bits;		/* how many bits are there in the bit map? */
  unsigned bit_blocks;		/* how many blocks are there in the bit map? */
  unsigned block, word, bcount, wcount;
  buffer_t *bp;
  bitchunk_t *wptr, *wlim, k;
  bit_t i, b;
  mfs_sb_t *sp;

  sp = mp->m_private_data;

  //if (sp->s_rd_only)
	//panic("can't allocate bit on read-only filesys.", NO_NUM);

  if (map == IMAP) {
	start_block = SUPER_BLOCK + 1;
	map_bits = sp->s_ninodes + 1;
	bit_blocks = sp->s_imap_blocks;
  } else {
	start_block = SUPER_BLOCK + 1 + sp->s_imap_blocks;
	map_bits = sp->s_zones - (sp->s_firstdatazone - 1);
	bit_blocks = sp->s_zmap_blocks;
  }

  /* Figure out where to start the bit search (depends on 'origin'). */
  if (origin >= map_bits) origin = 0;	/* for robustness */

  /* Locate the starting place. */
  block = origin / BITS_PER_BLOCK;
  word = (origin % BITS_PER_BLOCK) / BITCHUNK_BITS;

  /* Iterate over all blocks plus one, because we start in the middle. */
  bcount = bit_blocks + 1;
  do {
	  bitchunk_t *b_bitmap ;
	bp = bread(mp->m_dev, BLOCK_TO_SECTOR(start_block + block));

	b_bitmap = bp->b_data;
	wlim = &b_bitmap[BITMAP_CHUNKS];

	/* Iterate over the words in block. */
	for (wptr = &b_bitmap[word]; wptr < wlim; wptr++) {

		/* Does this word contain a free bit? */
		if (*wptr == (bitchunk_t) ~0) continue;

		/* Find and allocate the free bit. */
		k = conv2( (int) *wptr);
		for (i = 0; (k & (1 << i)) != 0; ++i) {}

		/* Bit number from the start of the bit map. */
		b = ((bit_t) block * BITS_PER_BLOCK)
		    + (wptr - &b_bitmap[0]) * BITCHUNK_BITS
		    + i;

		/* Don't allocate bits beyond the end of the map. */
		if (b >= map_bits) break;

		/* Allocate and return bit number. */
		k |= 1 << i;
		*wptr = conv2( (int) k);
		bp->b_dirt = DIRTY;
		buf_release(bp);
		return(b);
	}
	buf_release(bp);
	if (++block >= bit_blocks) block = 0;	/* last block, wrap around */
	word = 0;
  } while (--bcount > 0);
  return(NO_BIT);		/* no bit could be allocated */
}


/*===========================================================================*
 *				free_bit				     *
 *===========================================================================*/
void free_bit(mount_t *mp, int map, bit_t bit_returned)
		/* the filesystem to allocate from */
			/* IMAP (inode map) or ZMAP (zone map) */
		/* number of bit to insert into the map */
{
/* Return a zone or inode by turning off its bitmap bit. */

  unsigned block, word, bit;
  buffer_t *bp;
  bitchunk_t k, mask;
  block_t start_block;
  mfs_sb_t *sp;		/* the filesystem to operate on */

  sp = mp->m_private_data;

	  bitchunk_t *b_bitmap ;

  if (map == IMAP) {
	start_block = SUPER_BLOCK + 1;
  } else {
	start_block = SUPER_BLOCK + 1 + sp->s_imap_blocks;
  }
  block = bit_returned / BITS_PER_BLOCK;
  word = (bit_returned % BITS_PER_BLOCK) / BITCHUNK_BITS;
  bit = bit_returned % BITCHUNK_BITS;
  mask = 1 << bit;

  bp = bread(mp->m_dev, BLOCK_TO_SECTOR(start_block + block));
	b_bitmap = bp->b_data;

  k = conv2( (int) b_bitmap[word]);
  if (!(k & mask)) {
	panic(map == IMAP ? "tried to free unused inode" :
	      "tried to free unused block");
  }

  k &= ~mask;
  b_bitmap[word] = conv2( (int) k);
  bp->b_dirt = DIRTY;

  buf_release(bp);
}

zone_t alloc_zone(mount_t *dev, zone_t z)
			/* device where zone wanted */
			/* try to allocate new zone near this one */
{
/* Allocate a new zone on the indicated device and return its number. */

  int major, minor;
  bit_t b, bit;
  mfs_sb_t *sp;

  /* Note that the routine alloc_bit() returns 1 for the lowest possible
   * zone, which corresponds to sp->s_firstdatazone.  To convert a value
   * between the bit number, 'b', used by alloc_bit() and the zone number, 'z',
   * stored in the inode, use the formula:
   *     z = b + sp->s_firstdatazone - 1
   * Alloc_bit() never returns 0, since this is used for NO_BIT (failure).
   */
  //sp = get_superblock(dev);		/* find the super_block for this device */
  sp = dev->m_private_data ;

  /* If z is 0, skip initial part of the map known to be fully in use. */
  if (z == sp->s_firstdatazone) {
	bit = sp->s_zsearch;
  } else {
	bit = (bit_t) z - (sp->s_firstdatazone - 1);
  }
  b = alloc_bit(sp, ZMAP, bit);
  if (b == NO_BIT) {
	//err_code = ENOSPC;
	//major = (int) (sp->s_dev >> MAJOR) & BYTE;
	//minor = (int) (sp->s_dev >> MINOR) & BYTE;
	printf("No space on %sdevice %d/%d\n",
		 "root " , major, minor);
	return(NOZONE);
  }
  if (z == sp->s_firstdatazone) sp->s_zsearch = b;	/* for next time */
  return(sp->s_firstdatazone - 1 + (zone_t) b);
}

void free_zone(mount_t*dev, zone_t numb)
				/* device where zone located */
				/* zone to be returned */
{
/* Return a zone. */

  register mfs_sb_t *sp;
  bit_t bit;
  sp = dev->m_private_data;

  /* Locate the appropriate super_block and return bit. */
  //sp = get_superblock(dev);
  if (numb < sp->s_firstdatazone || numb >= sp->s_zones) return;
  bit = (bit_t) (numb - (sp->s_firstdatazone - 1));
  free_bit(sp, ZMAP, bit);
  if (bit < sp->s_zsearch) sp->s_zsearch = bit;
}

int write_map(inode_t * inoptr, off_t position, zone_t new_zone)

			/* file address to be mapped */
		/* zone # to be inserted */
{
/* Write a new zone into an inode. */
  int scale, ind_ex, new_ind, new_dbl, zones, nr_indirects, single, zindex, ex;
  zone_t z, z1;
  register block_t b;
  long excess, zone;
  buffer_t *bp;
register mfs2_inode_mem_t *rip;	/* pointer to inode to be changed */
  mfs_sb_t *sp;

  rip = inoptr->i_private_data;

	sp = inoptr->i_super->m_private_data;

  //rip->i_dirt = 1;		/* inode will be changed */
  bp = NULL;
  scale = sp->s_log_zone_size;		/* for zone-block conversion */
  zone = (position/V2_BLOCK_SIZE) >> scale;	/* relative zone # to insert */
  zones = rip->i_ndzones;	/* # direct zones in the inode */
  nr_indirects = rip->i_nindirs;/* # indirect zones per indirect block */

  /* Is 'position' to be found in the inode itself? */
  if (zone < zones) {
	zindex = (int) zone;	/* we need an integer here */
	rip->i_zone[zindex] = new_zone;
	return(OK);
  }

  /* It is not in the inode, so it must be single or double indirect. */
  excess = zone - zones;	/* first Vx_NR_DZONES don't count */
  new_ind = FALSE;
  new_dbl = FALSE;

  if (excess < nr_indirects) {
	/* 'position' can be located via the single indirect block. */
	z1 = rip->i_zone[zones];	/* single indirect zone */
	single = TRUE;
  } else {
	/* 'position' can be located via the double indirect block. */
	if ( (z = rip->i_zone[zones+1]) == NOZONE) {
		/* Create the double indirect block. */
		if ( (z = alloc_zone(inoptr->i_super, rip->i_zone[0])) == NOZONE)
			return(-1);
		rip->i_zone[zones+1] = z;
		new_dbl = TRUE;	/* set flag for later */
	}

	/* Either way, 'z' is zone number for double indirect block. */
	excess -= nr_indirects;	/* single indirect doesn't count */
	ind_ex = (int) (excess / nr_indirects);
	excess = excess % nr_indirects;
	if (ind_ex >= nr_indirects) return(EFBIG);
	b = (block_t) z << scale;
	bp = bread(inoptr->i_super->m_dev, BLOCK_TO_SECTOR(b));
	if (new_dbl) zero_block(bp);
	z1 = rd_indir(bp, ind_ex);
	single = FALSE;
  }

  /* z1 is now single indirect zone; 'excess' is index. */
  if (z1 == NOZONE) {
	/* Create indirect block and store zone # in inode or dbl indir blk. */
	z1 = alloc_zone(inoptr->i_super, rip->i_zone[0]);
	if (single)
		rip->i_zone[zones] = z1;	/* update inode */
	else
		wr_indir(bp, ind_ex, z1);	/* update dbl indir */

	new_ind = TRUE;
	if (bp != NULL) bp->b_dirt = DIRTY;	/* if double ind, it is dirty*/
	if (z1 == NOZONE) {
		buf_release(bp);	/* release dbl indirect blk */
		return(-1);	/* couldn't create single ind */
	}
  }
  buf_release(bp);	/* release double indirect blk */

  /* z1 is indirect block's zone number. */
  b = (block_t) z1 << scale;
  bp = bread(inoptr->i_super->m_dev, BLOCK_TO_SECTOR(b) );
  if (new_ind) zero_block(bp);
  ex = (int) excess;			/* we need an int here */
  wr_indir(bp, ex, new_zone);
  bp->b_dirt = 1;
  buf_release(bp);

  return(OK);
}


/*===========================================================================*
 *				clear_zone				     *
 *===========================================================================*/
void clear_zone(inode_t * inoptr, off_t pos, int flag)

			/* points to block to clear */
			/* 0 if called by read_write, 1 by new_block */
{
/* Zero a zone, possibly starting in the middle.  The parameter 'pos' gives
 * a byte in the first block to be zeroed.  Clearzone() is called from 
 * read_write and new_block().
 */

register mfs2_inode_mem_t *rip;	/* inode to clear */
  register buffer_t *bp;
  register block_t b, blo, bhi;
  register off_t next;
  register int scale;
  register zone_t zone_size;
  mfs_sb_t *sp;

	sp = inoptr->i_super->m_private_data;
  rip = inoptr->i_private_data;

  /* If the block size and zone size are the same, clear_zone() not needed. */
  scale = sp->s_log_zone_size;
  if (scale == 0) return;

  zone_size = (zone_t) V2_BLOCK_SIZE << scale;
  if (flag == 1) pos = (pos/zone_size) * zone_size;
  next = pos + V2_BLOCK_SIZE - 1;

  /* If 'pos' is in the last block of a zone, do not clear the zone. */
  if (next/zone_size != pos/zone_size) return;
  if ( (blo = read_map(inoptr, next)) == NOBLOCK) return;
  bhi = (  ((blo>>scale)+1) << scale)   - 1;

  /* Clear all the blocks between 'blo' and 'bhi'. */
  for (b = blo; b <= bhi; b++) {
	bp = bread(inoptr->i_super->m_dev, BLOCK_TO_SECTOR(b));
	zero_block(bp);
	buf_release(bp);
  }
}



