


#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <errno.h>
#include <assert.h>
#include <net/inet.h>
#include "minixfs.h"

void rw_inode(rip, rw_flag)
register  inode_t *rip;	/* pointer to inode to be read/written */
int rw_flag;			/* READING or WRITING */
{
/* An entry in the inode table is to be copied to or from the disk. */

  register buffer_t *bp;
  register mfs_sb_t *sp;
  register mount_t *mp;
  block_t b, offset;
  mfs2_inode_t *dip2;

#if 0

  /* Get the block where the inode resides. */
  mp = get_super(rip->i_dev);	/* get pointer to super block */
  sp = mp->m_private_data;

  rip->i_sp = sp;		/* inode must contain super block pointer */
  offset = sp->s_imap_blocks + sp->s_zmap_blocks + 2;
  b = (block_t) (rip->i_number - 1)/sp->s_inodes_per_block + offset;
  bp = bread(rip->i_dev, BLOCK_TO_SECTOR( b) );

  dip2 = bp->b_v2_ino + (rip->i_number - 1) % V2_INODES_PER_BLOCK;

  /* Do the read or write. */
  if (rw_flag) {
	if (rip->i_update) update_times(rip);	/* times need updating */
	//if (sp->s_rd_only == FALSE) 
		bp->b_dirt = DIRTY;
  }

  /* Copy the inode from the disk block to the in-core table or vice versa.
   * If the fourth parameter below is FALSE, the bytes are swapped.
   */
 
	new_icopy(rip, dip2, rw_flag, sp->s_native);
  
  buf_release(bp);
  rip->i_dirt = CLEAN;
#endif
}


int search_dir(ldir_ptr, string, numb, flag)
register  inode_t *ldir_ptr;	/* ptr to inode for dir to search */
char string[NAME_MAX];		/* component to search for */
ino_t *numb;			/* pointer to inode number */
int flag;			/* LOOK_UP, ENTER, DELETE or IS_EMPTY */
{
/* This function searches the directory whose inode is pointed to by 'ldip':
 * if (flag == ENTER)  enter 'string' in the directory with inode # '*numb';
 * if (flag == DELETE) delete 'string' from the directory;
 * if (flag == LOOK_UP) search for 'string' and return inode # in 'numb';
 * if (flag == IS_EMPTY) return OK if only . and .. in dir else ENOTEMPTY;
 *
 *    if 'string' is dot1 or dot2, no access permissions are checked.
 */

  register mfs_direct_t *dp;
  register buffer_t *bp;
  int i, r, e_hit, t, match;
  mode_t bits;
  off_t pos;
  unsigned new_slots, old_slots;
  block_t b;
  mfs_sb_t *sp;
  int extended = 0;

  /* If 'ldir_ptr' is not a pointer to a dir inode, error. */
  if ( (ldir_ptr->i_mode & I_TYPE) != I_DIRECTORY) return(ENOTDIR);

  r = OK;

#if 0
  if (flag != IS_EMPTY) {
	bits = (flag == LOOK_UP ? X_BIT : W_BIT | X_BIT);

	if (string == dot1 || string == dot2) {
		if (flag != LOOK_UP) r = read_only(ldir_ptr);
				     /* only a writable device is required. */
        }
	else r = forbidden(ldir_ptr, bits); /* check access permissions */
  }

#endif
  if (r != OK) return(r);
  
  /* Step through the directory one block at a time. */
  old_slots = (unsigned) (ldir_ptr->i_size/DIR_ENTRY_SIZE);
  new_slots = 0;
  e_hit = FALSE;
  match = 0;			/* set when a string match occurs */

  for (pos = 0; pos < ldir_ptr->i_size; pos += V2_BLOCK_SIZE) {

	  mfs_direct_t *b_dir;

	b = read_map(ldir_ptr, pos);	/* get block number */

	/* Since directories don't have holes, 'b' cannot be NO_BLOCK. */
	bp = bread(ldir_ptr->i_super->m_dev, BLOCK_TO_SECTOR(b));	/* get a dir block */

	b_dir = bp->b_data;

	/* Search a directory block. */
	for (dp = &b_dir[0]; dp < &b_dir[NR_DIR_ENTRIES]; dp++) {
		if (++new_slots > old_slots) { /* not found, but room left */
			if (flag == ENTER) e_hit = TRUE;
			break;
		}

		/* Match occurs if string found. */
		if (flag != ENTER && dp->d_ino != 0) {
			if (flag == IS_EMPTY) {
				/* If this test succeeds, dir is not empty. */
				if (strcmp(dp->d_name, "." ) != 0 &&
				    strcmp(dp->d_name, "..") != 0) match = 1;
			} else {
				if (strncmp(dp->d_name, string, NAME_MAX) == 0)
					match = 1;
			}
		}

		if (match) {
			/* LOOK_UP or DELETE found what it wanted. */
			r = OK;
			if (flag == IS_EMPTY) r = ENOTEMPTY;
			else if (flag == DELETE) {
				/* Save d_ino for recovery. */
				t = NAME_MAX - sizeof(ino_t);
				*((ino_t *) &dp->d_name[t]) = dp->d_ino;
				dp->d_ino = 0;	/* erase entry */
				bp->b_dirt = 1;
				ldir_ptr->i_update |= CTIME | MTIME;
				ldir_ptr->i_dirt = DIRTY;
			} else {
				sp = ldir_ptr->i_super->m_private_data;	/* 'flag' is LOOK_UP */
				*numb = conv2( (int) dp->d_ino);
			}
			buf_release(bp);
			return(r);
		}


		/* Check for free slot for the benefit of ENTER. */
		if (flag == ENTER && dp->d_ino == 0) {
			e_hit = TRUE;	/* we found a free slot */
			break;
		}
	}

	/* The whole block has been searched or ENTER has a free slot. */
	if (e_hit) break;	/* e_hit set if ENTER can be performed now */
	buf_release(bp);	/* otherwise, continue searching dir */
  }

  /* The whole directory has now been searched. */
  if (flag != ENTER) return(flag == IS_EMPTY ? OK : ENOENT);

  /* This call is for ENTER.  If no free slot has been found so far, try to
   * extend directory.
   */
  if (e_hit == FALSE) { /* directory is full and no room left in last block */
	new_slots++;		/* increase directory size by 1 entry */
	if (new_slots == 0) return(EFBIG); /* dir size limited by slot count */
	if ( (bp = new_block(ldir_ptr, ldir_ptr->i_size)) == NULL)
		return(-1);
	dp = bp->b_data;
	extended = 1;
  }

  /* 'bp' now points to a directory block with space. 'dp' points to slot. */
  (void) memset(dp->d_name, 0, (size_t) NAME_MAX); /* clear entry */
  for (i = 0; string[i] && i < NAME_MAX; i++) dp->d_name[i] = string[i];
  sp = ldir_ptr->i_super->m_private_data; 
  dp->d_ino = conv2( (int) *numb);
  bp->b_dirt = DIRTY;
  buf_release(bp);
  ldir_ptr->i_update |= CTIME | MTIME;	/* mark mtime for update later */
  ldir_ptr->i_dirt = DIRTY;
  if (new_slots > old_slots) {
	ldir_ptr->i_size = (off_t) new_slots * DIR_ENTRY_SIZE;
	/* Send the change to disk if the directory is extended. */
	if (extended) rw_inode(ldir_ptr, 1);//WRITING
  }
  return(OK);
}
#if 0
#endif
inode_t *new_node(string, bits, z0)
char *string;			/* pointer to path name */
mode_t bits;			/* mode of the new inode */
zone_t z0;			/* zone number 0 for new inode */
{
/* New_node() is called by common_open(), do_mknod(), and do_mkdir().  
 * In all cases it allocates a new inode, makes a directory entry for it on 
 * the path 'path', and initializes it.  It returns a pointer to the inode if 
 * it can do this; otherwise it returns NULL.  It always sets 'err_code'
 * to an appropriate value (OK or an error code).
 */

  register inode_t *rlast_dir_ptr, *rip;
  register int r;
  //char string[NAME_MAX];
  int err_code=ENOENT;
  mfs2_inode_mem_t * ripmem;


  /* See if the path can be opened down to the last directory. */
  //if ((rlast_dir_ptr = last_dir(path, string)) == NULL) return(NULL);

  /* The final directory is accessible. Get final component of the path. */
  //rip = advance(rlast_dir_ptr, string);
  if ( rip == NULL && err_code == ENOENT) {
	/* Last path component does not exist.  Make new directory entry. */
	if ( (rip = alloc_inode(rlast_dir_ptr->i_dev, bits)) == NULL) {
		/* Can't creat new inode: out of inodes. */
		iput(rlast_dir_ptr);
		return(NULL);
	}
  ripmem = rip->i_private_data;

	/* Force inode to the disk before making directory entry to make
	 * the system more robust in the face of a crash: an inode with
	 * no directory entry is much better than the opposite.
	 */
	ripmem->i_nlinks++;
	ripmem->i_zone[0] = z0;		/* major/minor device numbers */
	rw_inode(rip, 1);		/* force inode to disk now */

	/* New inode acquired.  Try to make directory entry. */
	if ((r = search_dir(rlast_dir_ptr, string, &rip->i_number,ENTER)) != OK) {
		iput(rlast_dir_ptr);
		ripmem->i_nlinks--;	/* pity, have to free disk inode */
		rip->i_dirt = DIRTY;	/* dirty inodes are written out */
		iput(rip);	/* this call frees the inode */
		err_code = r;
		return(NULL);
	}

  } else {
	/* Either last component exists, or there is some problem. */
	if (rip != NULL)
		r = EEXIST;
	else
		r = err_code;
  }

  /* Return the directory inode and exit. */
  iput(rlast_dir_ptr);
  err_code = r;
  return(rip);
}


int minixfs_mkdir(inode_t* ldirp, char* string, mode_t mode)
{
#if 0
	int error = 0;
	inode_t *  inoptr;
	mode_t mode1;
	mode1 = ((mode & I_UGRWX) & (~(current->umask))) | I_DIR;
	inoptr = iinsert(ldirp, string, mode1, &error, 0);

	if(!inoptr){
		iput(ldirp);
		return error;
	}	

	mode1 = ((mode & I_UGRWX) & (~(current->umask))) | I_DIR;
	iinsert(inoptr, ".", mode1, &error, inoptr->i_number);
	iinsert(inoptr, "..", mode1, &error, ldirp->i_number);

	inoptr->i_nlinks += 1; // ialloc will set nlinks to 1
	inoptr->i_dirty = 1;
	iput(inoptr);

	ldirp->i_nlinks++;
	ldirp->i_update |= MTIME;
	ldirp->i_dirty = 1;
	//iput(ldirp);
#else
/* Perform the mkdir(name, mode) system call. */

  int r1, r2;			/* status codes */
  ino_t dot, dotdot;		/* inode numbers for . and .. */
  mode_t bits;			/* mode bits for the new inode */
  //char string[NAME_MAX];	/* last component of the new dir's path name */
  register  inode_t *rip;
  mfs2_inode_mem_t * ripmem;
  mfs2_inode_mem_t * ipmem;
  int err_code = EEXIST;

#if 0
  /* Check to see if it is possible to make another link in the parent dir. */
  if (fetch_name(name1, name1_length, M1) != OK) return(err_code);
  ldirp = last_dir(user_path, string);	/* pointer to new dir's parent */
  if (ldirp == NULL) return(err_code);
  if (ldirp->i_nlinks >= (ldirp->i_sp->s_version == V1 ? CHAR_MAX : SHRT_MAX)) {
	iput(ldirp);	/* return parent */
	return(EMLINK);
  }
#endif

  /* Next make the inode. If that fails, return error code. */
  bits = I_DIRECTORY | (mode & RWX_MODES );
  rip = new_node(string, bits, (zone_t) 0);
  if (rip == NULL || err_code == EEXIST) {
	iput(rip);		/* can't make dir: it already exists */
	//iput(ldirp);	/* return parent too */
	return(err_code);
  }

  /* Get the inode numbers for . and .. to enter in the directory. */
  dotdot = ldirp->i_number;	/* parent's inode number */
  dot = rip->i_number;		/* inode number of the new dir itself */

  /* Now make dir entries for . and .. unless the disk is completely full. */
  /* Use dot1 and dot2, so the mode of the directory isn't important. */
  rip->i_mode = bits;	/* set mode */
  r1 = search_dir(rip, dot1, &dot, ENTER);	/* enter . in the new dir */
  r2 = search_dir(rip, dot2, &dotdot, ENTER);	/* enter .. in the new dir */
	  ripmem = rip->i_private_data;
	  ipmem = ldirp->i_private_data;

  /* If both . and .. were successfully entered, increment the link counts. */
  if (r1 == OK && r2 == OK) {
	/* Normal case.  It was possible to enter . and .. in the new dir. */
	ripmem->i_nlinks++;	/* this accounts for . */
	ipmem->i_nlinks++;	/* this accounts for .. */
	ldirp->i_dirt = DIRTY;	/* mark parent's inode as dirty */
  } else {
	/* It was not possible to enter . or .. probably disk was full. */
	(void) search_dir(ldirp, string, (ino_t *) 0, DELETE);
	ripmem->i_nlinks--;	/* undo the increment done in new_node() */
  }
  rip->i_dirt = DIRTY;		/* either way, i_nlinks has changed */

  //iput(ldirp);		/* return the inode of the parent dir */
  iput(rip);		/* return the inode of the newly made dir */
  return(err_code);		/* new_node() always sets 'err_code' */
#endif

	return 0;
}




inode_t* minixfs_opendir(inode_t * inode, char *filepath)
{

	inode_t* child_vnode = NULL;
	mount_t *sp;
	u32_t c_ino=0;
	off_t block,pos;	
	int i,count,retval;
	buffer_t * bh;
	mfs_direct_t* dirptr;

	//printf("minixfs_opendir: %s inode->i_size %d\n", filepath,inode->i_size);

	/* No blank filenames */
	if (!filepath[0])
		return 0;

	for(pos = 0; pos < inode->i_size; pos += BLOCKSIZ){
		block = read_map(inode, pos);
		if(block == NOBLOCK){
			printk("can't get NOBLOCK\n");
			//iput(read_map); /* XXX */
			return NULL;
		}

		bh = bread(inode->i_super->m_dev, BLOCK_TO_SECTOR(block));
		if(!bh){
			printk("error when reading block\n");
			//iput(inode); /* XXX */
			return NULL;
		}
	
		count = BLOCKSIZ / DIR_ENTRY_SIZE;
		if(inode->i_size - pos < BLOCKSIZ)
			count = (inode->i_size - pos) / DIR_ENTRY_SIZE;
	//printf("minixfs_opendir: %s %d\n", filepath,count);

		dirptr = (struct direct *)bh->b_data;

		for(i = 0; i < count; i++, dirptr++){

			if(dirptr->d_ino == 0)
				continue;

			//printf("%s %d~~", dirptr->d_name,dirptr->d_ino);


			if(strncmp(filepath, dirptr->d_name, NAME_MAX) == 0){
				c_ino = dirptr->d_ino;		
				break;
			}
		}
				

		buf_release(bh);

		if (c_ino)
		{
			child_vnode = iget(inode->i_dev, c_ino);
	//printf("\nis found %s on %d size%d\n",filepath,c_ino,child_vnode->i_size);
			child_vnode->i_father = inode->i_number;
			break;
		}
	}

	
	return child_vnode;
err:
	return NULL;
}



int minixfs_readdir(file_t* filp, vfs_dirent_t *dirent)
{
	inode_t *inode = filp->f_inode;
	u32_t		i, ni, type;
	off_t block;
	buffer_t * bh;
	mfs_direct_t* dirptr;

	int blk,blk_offset;
	int retval,count;
	u32_t offset = filp->f_inode->i_number;
	//int fd = filp->f_inode->i_dev;
	int pos = filp->f_pos;
		

	memset(dirent, 0, sizeof(vfs_dirent_t));

	if (pos>inode->i_size)
	{
		return -1;
	}

	block = read_map(inode, pos);
	if(block == NOBLOCK){
		printk("can't get NOBLOCK\n");
		//iput(read_map); /* XXX */
		return NULL;
	}

	bh = bread(inode->i_super->m_dev, BLOCK_TO_SECTOR(block));
	if(!bh){
		printk("error when reading block\n");
		//iput(inode); /* XXX */
		return NULL;
	}

	count = pos / DIR_ENTRY_SIZE;
	
	dirptr = (struct direct *)bh->b_data;
	for(i = 0; i < count; i++, dirptr++){
		if(dirptr->d_ino == 0)
			continue;
		}
	

	buf_release(bh);
	
	strncpy(dirent->l_long_name, dirptr->d_name,sizeof(dirent->l_long_name));
	//dirent->l_attribute   = romfs2dostype(type);
	dirent->l_ctime  = 0;
	dirent->l_atime  = 0;
	dirent->l_mtime  = 0;
	dirent->l_size_high = 0;
	//dirent->l_size_low = ntohl_32(&fhdr->size);
	*(unsigned long*)&dirent->l_res[0] = 0;
	*(unsigned long*)&dirent->l_res[4] = 0;


	filp->f_pos+=DIR_ENTRY_SIZE;
	
	return 0;
}

