/*
**     (R)Jicama OS
**     File Node Access
**     Copyright (C) 2003 DengPingPing
*/


#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <drv/errno.h>
#include <assert.h>
#include "sys.h"
static inode_t vnode[NR_INODE];

static TAILQ_HEAD(,inode) g_freelist;
static TAILQ_HEAD(,inode) g_inodelist;
static unsigned buffer_sem_lock;

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

void write_inode(inode_t * inode)
{
	buffer_t * bh;
	int block;

	inode->i_lock=1;
	if (!inode->i_dirt || !inode->i_dev) {
	inode->i_lock=0;
		return;
	}

    vfs_write_inode (inode);
	inode->i_dirt=0;
	inode->i_lock=0;
}

void dup_inode(ip)
inode_t *ip;		/* The inode to be duplicated. */
{
/* This routine is a simplified form of get_inode() for the case where
 * the inode pointer is already known.
 */
 if (!ip)
 {
	 return;
 }

  ip->i_count++;
}

inode_t * ifind(const int dev, const int blk)
{
	int i;
	inode_t *result;

	LOCK_SCHED(buffer_sem_lock);

     for (i = 0; i < NR_INODE; i++)
     {
		result = &vnode[i];
		if (result->i_number == blk
			&& result->i_dev == dev)
			break;
    }

	if (i != NR_INODE){
	      //printk("use old d=%d b=%d ____\n", dev, blk);
		  UNLOCK_SCHED(buffer_sem_lock);
		  return result;
	}
	UNLOCK_SCHED(buffer_sem_lock);
	return NULL;
}

inode_t * iget(const int dev, const int blk)
{
	int i;
	inode_t *result;

	result = ifind(dev,blk);

	if (result)
	{
		dup_inode(result);
		return result;
	}

	LOCK_SCHED(buffer_sem_lock);

     for (i = 0; i < NR_INODE; i++)
     {
		result = &vnode[i];
		if (result->i_count == 0)
			break;
    }

	if (i == NR_INODE){
		UNLOCK_SCHED(buffer_sem_lock);
		kprintf("iget(): no memory!\n");
		return NULL;
	}
	UNLOCK_SCHED(buffer_sem_lock);

	result->i_dev = dev;
	result->i_super = get_superblock(dev);
	result->i_mode = 0;
	result->i_number = blk;
	result->i_count=1; /*do not forget it!*/
	result->i_private_data = NULL;
	vfs_ino_rw(result->i_super, result,0);
	return result;
}



int init_fiob(file_t *fp,int size,char *type)
{
	if (size<sizeof(struct ioobject))
	{
		size = sizeof(struct ioobject);
	}
	LOCK_SCHED(buffer_sem_lock);
	fp->f_iob = kmalloc(size,0);
	memset(fp->f_iob,0,(size));
	init_ioobject(fp->f_iob,type);
	UNLOCK_SCHED(buffer_sem_lock);
	return 0;
}

void free_fiob(file_t *fp)
{
	//return; 

	if(!fp->f_iob)
	return; 

#if 1//fixme
	detach_ioobject(fp->f_iob);
#endif
	
	kfree(fp->f_iob);
	fp->f_iob=NULL;
}

static void free_inode_data(inode_t* node)
{
	assert(node);
	if (node->i_private_data)
	{		
		node->i_private_data = NULL;
	}

	node->i_dev = 0;
	node->i_number = 0;
	node->i_count = 0;
	node->i_mode = 0;
	node->i_dirt = 0;

}

static void init_inode_data(inode_t* node)
{
	assert(node);
	node->i_dev = 0;
	node->i_number = 0;
	node->i_count = 0;
	node->i_mode = 0;
	node->i_dirt = 0;
	
	node->i_private_data = NULL;
}


int iput(inode_t* inode)
{
	int ino = inode->i_number;
	//assert(inode != NULL);

	if (!inode)
		return OK;

	LOCK_SCHED(buffer_sem_lock);

	if (inode->i_dirt && (S_ISREG(inode->i_mode)||S_ISDIR(inode->i_mode)))	
		write_inode(inode);

	if(inode->i_count>1){
		inode->i_count--;
		UNLOCK_SCHED(buffer_sem_lock);
		
		return -EBUSY;
	}


	//vfs_ino_rw(inode->i_super, inode,1);
	if (inode->i_private_data != NULL){
		//printf("free page %x\n", inode->i_private_data);
		//kfree((u32_t)inode->i_private_data);
		inode->i_private_data=NULL;
	}


	free_inode_data(inode);
	UNLOCK_SCHED(buffer_sem_lock);
	//kprintf("free inode(%d) ok\n",ino);
	return 0;
}




void inode_init(void)
{
	int i;
	inode_t* node;

	for (i=0; i<NR_INODE; i++)
	{
		node = &vnode[i];
		init_inode_data(node);
	}
}



void sync_inodes(void)
{
	int i;
	inode_t * inode;

	inode = 0+vnode;
	for(i=0 ; i<NR_INODE ; i++,inode++) {
		if (inode->i_dirt && (S_ISREG(inode->i_mode)||S_ISDIR(inode->i_mode)) )
			write_inode(inode);
	}
}

inode_t * create_pipe_node(int cnt)
{
	static int nodeval=0;
	//pipe_prvi_t* prvi;
	inode_t *node=iget(0x700, nodeval++);
	if (!node)
	{
		return NULL;
	}
	node->i_father= 0;
    node->i_seek = 0;
	node->i_mode= typemode(S_IFIFO, 0666) ;
	//prvi = (unsigned long)kmalloc(sizeof(pipe_prvi_t),0);

	node->i_uid=current_filp()->uid;
	node->i_ctime= startup_ticks();
	node->i_atime= startup_ticks();
	node->i_mtime= startup_ticks();

	node->i_count= cnt;
	//init_ioobject(&prvi->base.iob,"PIP");
	return node;
}

inode_t * create_socket_node(long type)
{
	static int nodeval=0;
	inode_t *node=iget(0x800, nodeval++);
	fs_task_t *task_space = current_filp();
	if (!node)
	{
		return NULL;
	}
	node->i_father= 0;
    node->i_seek = 0;
	node->i_private_data= NULL;
	
	node->i_uid=task_space->uid;
	node->i_ctime= startup_ticks();
	node->i_atime= startup_ticks();
	node->i_mtime= startup_ticks();
	node->i_mode = type|0666;
	node->i_count= 1;
	return node;
}




