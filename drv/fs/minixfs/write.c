
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
char dot1[2] = ".";	/* used for search_dir to bypass the access */
char dot2[3] = "..";	/* permissions for . and ..		    */

/*===========================================================================*
 *				truncate				     *
 *===========================================================================*/
void mfs_truncate(inode_t *inoptr)
	/* pointer to inode to be truncated */
{
/* Remove all the zones from the inode 'rip' and mark it dirty. */

  register block_t b;
  zone_t z, zone_size, z1;
  off_t position;
  int i, scale, file_type, waspipe, single, nr_indirects;
  buffer_t *bp;
  mount_t* dev;

  mfs_sb_t *sp;
  mfs2_inode_mem_t * rip;

  rip = inoptr->i_private_data;
  sp = inoptr->i_super->m_private_data;

  file_type = inoptr->i_mode & I_TYPE;	/* check to see if file is special */
  if (file_type == I_CHAR_SPECIAL || file_type == I_BLOCK_SPECIAL) return;
  dev = inoptr->i_super;		/* device on which inode resides */
  scale = sp->s_log_zone_size;
  zone_size = (zone_t) V2_BLOCK_SIZE << scale;
  nr_indirects = rip->i_nindirs;

  /* Pipes can shrink, so adjust size to make sure all zones are removed. */
  //waspipe = rip->i_pipe == I_PIPE;	/* TRUE is this was a pipe */
 // if (waspipe) rip->i_size = PIPE_SIZE;

  /* Step through the file a zone at a time, finding and freeing the zones. */
  for (position = 0; position < inoptr->i_size; position += zone_size) {
	if ( (b = read_map(rip, position)) != NOBLOCK) {
		z = (zone_t) b >> scale;
		free_zone(dev, z);
	}
  }

  /* All the data zones have been freed.  Now free the indirect zones. */
  //rip->i_dirt = DIRTY;
  if (waspipe) {
	wipe_inode(inoptr);	/* clear out inode for pipes */
	return;			/* indirect slots contain file positions */
  }
  single = rip->i_ndzones;
  free_zone(dev, rip->i_zone[single]);	/* single indirect zone */
  if ( (z = rip->i_zone[single+1]) != NOZONE) {
	/* Free all the single indirect zones pointed to by the double. */
	b = (block_t) z << scale;
	bp = bread(dev->m_dev, BLOCK_TO_SECTOR(b) );	/* get double indirect zone */
	for (i = 0; i < nr_indirects; i++) {
		z1 = rd_indir(bp, i);
		free_zone(dev, z1);
	}

	/* Now free the double indirect zone itself. */
	buf_release(bp);
	free_zone(dev, z);
  }

  /* Leave zone numbers for de(1) to recover file after an unlink(2).  */
}



/*===========================================================================*
 *				remove_dir				     *
 *===========================================================================*/
int remove_dir(rldirp, rip, dir_name)
inode_t *rldirp;		 	/* parent directory */
inode_t *rip;			/* directory to be removed */
char dir_name[NAME_MAX];		/* name of directory to be removed */
{
  /* A directory file has to be removed. Five conditions have to met:
   * 	- The file must be a directory
   *	- The directory must be empty (except for . and ..)
   *	- The final component of the path must not be . or ..
   *	- The directory must not be the root of a mounted file system
   *	- The directory must not be anybody's root/working directory
   */

  int r;
 // register struct fproc *rfp;

  /* search_dir checks that rip is a directory too. */
  if ((r = search_dir(rip, "", (ino_t *) 0, IS_EMPTY)) != OK) return r;

  if (strcmp(dir_name, ".") == 0 || strcmp(dir_name, "..") == 0)return(EINVAL);
  if (rip->i_number == ROOT_INODE) return(EBUSY); /* can't remove 'root' */
	
#if 0
  for (rfp = &fproc[INIT_PROC_NR + 1]; rfp < &fproc[NR_PROCS]; rfp++)
	if (rfp->fp_workdir == rip || rfp->fp_rootdir == rip) return(EBUSY);
#endif
				/* can't remove anybody's working dir */

  /* Actually try to unlink the file; fails if parent is mode 0 etc. */
  if ((r = unlink_file(rldirp, rip, dir_name)) != OK) return r;

  /* Unlink . and .. from the dir. The super user can link and unlink any dir,
   * so don't make too many assumptions about them.
   */
  (void) unlink_file(rip, NULL, dot1);
  (void) unlink_file(rip, NULL, dot2);
  return(OK);
}


/*===========================================================================*
 *				unlink_file				     *
 *===========================================================================*/
int unlink_file(dirp, rip, file_name)
inode_t *dirp;		/* parent directory of file */
inode_t *rip;		/* inode of file, may be NIL_INODE too. */
char file_name[NAME_MAX];	/* name of file to be removed */
{
	int err_code;
	 mfs2_inode_mem_t * ripmem;

  
/* Unlink 'file_name'; rip must be the inode of 'file_name' or NIL_INODE. */

  ino_t numb;			/* inode number */
  int	r;

  /* If rip is not NIL_INODE, it is used to get faster access to the inode. */
  if (rip == NULL) {
  	/* Search for file in directory and try to get its inode. */
	err_code = search_dir(dirp, file_name, &numb, LOOK_UP);
	if (err_code == OK) rip = iget(dirp->i_dev, (int) numb);
	if (err_code != OK || rip == NULL) return(-1);
  } else {
	dup_inode(rip);		/* inode will be returned with iput */
  }

  r = search_dir(dirp, file_name, (ino_t *) 0, DELETE);

  if (r == OK) {
	  ripmem = rip->i_private_data;
	ripmem->i_nlinks--;	/* entry deleted from parent's dir */
	rip->i_update |= CTIME;
	rip->i_dirt = DIRTY;
  }

  iput(rip);
  return(r);
}


 buffer_t *new_block(inode_t *inode, off_t position)
	/* pointer to inode */
			/* file pointer */
{
/* Acquire a new block and return a pointer to it.  Doing so may require
 * allocating a complete zone, and then returning the initial block.
 * On the other hand, the current zone may still have some unused blocks.
 */

  register buffer_t *bp;
  block_t b, base_block;
  zone_t z;
  zone_t zone_size;
  int scale, r;
  mfs_sb_t *sp;
  mfs2_inode_mem_t * rip;

  rip = inode->i_private_data;

  /* Is another block available in the current zone? */
  if ( (b = read_map(rip, position)) == NOBLOCK) {
	/* Choose first zone if possible. */
	/* Lose if the file is nonempty but the first zone number is NO_ZONE
	 * corresponding to a zone full of zeros.  It would be better to
	 * search near the last real zone.
	 */
	if (rip->i_zone[0] == NOZONE) {
		sp = inode->i_super->m_private_data;
		z = sp->s_firstdatazone;
	} else {
		z = rip->i_zone[0];	/* hunt near first zone */
	}
	if ( (z = alloc_zone(inode->i_super, z)) == NOZONE) return(NULL);
	if ( (r = write_map(inode, position, z)) != OK) {
		free_zone(inode->i_super, z);
		//err_code = r;
		return(NULL);
	}

	/* If we are not writing at EOF, clear the zone, just to be safe. */
	if ( position != inode->i_size) clear_zone(inode, position, 1);
	scale = sp->s_log_zone_size;
	base_block = (block_t) z << scale;
	zone_size = (zone_t) V2_BLOCK_SIZE << scale;
	b = base_block + (block_t)((position % zone_size)/V2_BLOCK_SIZE);
  }

  bp = bread(inode->i_super->m_dev, BLOCK_TO_SECTOR( b));
  zero_block(bp);
  return(bp);
}

#if 0
int do_unlink()
{
/* Perform the unlink(name) or rmdir(name) system call. The code for these two
 * is almost the same.  They differ only in some condition testing.  Unlink()
 * may be used by the superuser to do dangerous things; rmdir() may not.
 */

  register struct inode *rip;
  struct inode *rldirp;
  int r;
  char string[NAME_MAX];

  /* Get the last directory in the path. */
  if (fetch_name(name, name_length, M3) != OK) return(err_code);
  if ( (rldirp = last_dir(user_path, string)) == NIL_INODE)
	return(err_code);

  /* The last directory exists.  Does the file also exist? */
  r = OK;
  if ( (rip = advance(rldirp, string)) == NIL_INODE) r = err_code;

  /* If error, return inode. */
  if (r != OK) {
	put_inode(rldirp);
	return(r);
  }

  /* Do not remove a mount point. */
  if (rip->i_num == ROOT_INODE) {
	put_inode(rldirp);
	put_inode(rip);
	return(EBUSY);
  }

  /* Now test if the call is allowed, separately for unlink() and rmdir(). */
  if (fs_call == UNLINK) {
	/* Only the su may unlink directories, but the su can unlink any dir.*/
	if ( (rip->i_mode & I_TYPE) == I_DIRECTORY && !super_user) r = EPERM;

	/* Don't unlink a file if it is the root of a mounted file system. */
	if (rip->i_num == ROOT_INODE) r = EBUSY;

	/* Actually try to unlink the file; fails if parent is mode 0 etc. */
	if (r == OK) r = unlink_file(rldirp, rip, string);

  } else {
	r = remove_dir(rldirp, rip, string); /* call is RMDIR */
  }

  /* If unlink was possible, it has been done, otherwise it has not. */
  put_inode(rip);
  put_inode(rldirp);
  return(r);
}
#endif
