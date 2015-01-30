
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

static int buf_in_use;		/* # bufs 当前正在使用currently in use (not on free list)*/
static buffer_t fs_bufs[NR_BUFS];


static TAILQ_HEAD(,buf_struct) g_freelist;
static TAILQ_HEAD(,buf_struct) g_buflist;

static void *g_buffer_mutex;

int mem_isnull(const void *buf, int len )
{
	int i;
	char *ptr = buf;

	for (i=0; i<len; i++)
	{
		if (ptr[i])
		{
			return 0;
		}
	}
	return 1;
}


/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

void buffer_init()
{
  register buffer_t *bp;

  buf_in_use = 0;
  //buf_head = &fs_bufs[0];
  //buf_tail = &fs_bufs[NR_BUFS - 1];

  for (bp = &fs_bufs[0]; bp < &fs_bufs[NR_BUFS]; bp++) {
	bp->b_count = 0;
	bp->b_dirt = FALSE;
	bp->b_dev = NO_DEV;
	bp->b_size = 0;
	bp->b_data = NULL;
  }

  g_buffer_mutex = create_semaphore("fs_buffer",0,1);

}

void zero_block(buffer_t *bp)
{
/* Zero a block. */

  memset(bp->b_data, 0, bp->b_size);
  bp->b_dirt = 1;
}

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

buffer_t* buf_getblk(void)
{
	int i = 0;
	buffer_t* bp;

	lock_semaphore(g_buffer_mutex);
	
	for (bp = &fs_bufs[0]; bp < &fs_bufs[NR_BUFS]; ++bp,i++){
		if (bp->b_count == 0)	{
			bp->b_count = 1;
	         buf_in_use++;
			 unlock_semaphore(g_buffer_mutex);
			return bp;
			}
	}

	unlock_semaphore(g_buffer_mutex);
	return NULL_BUF;
}

void invalidate(dev_t device)
{
/* Remove all the blocks belonging to some device from the cache. */

  register buffer_t *bp;

  lock_semaphore(g_buffer_mutex);

  for (bp = &fs_bufs[0]; bp < &fs_bufs[NR_BUFS]; bp++)
	if (bp->b_dev == device){
	  buf_release(bp);
  }
  unlock_semaphore(g_buffer_mutex);
}



static void flush_buf(register buffer_t* bp)
{
	assert(bp);

	/*write memory to disk space*/
	if(!bp->b_dirt)return;
	dev_write(get_dev_desc(bp->b_dev), bp->b_block, bp->b_data, bp->b_size);
	bp->b_dirt = 0;

}

void buf_release(buffer_t* bp)
{
	assert(bp);

	bp->b_count--;

	if(bp->b_count <= 0){
	   -- buf_in_use;
	}
	else{
		return ;
	}
	
	flush_buf(bp);

#if 0
	if (bp->b_data)
	{
		/*free memory*/
	  kfree(bp->b_data);
	  bp->b_data = NULL;
	  bp->b_size = 0;
	}
#endif
	
	return;
}



buffer_t* buf_find(int dev, unsigned int block)
{
	register buffer_t* bp;

	lock_semaphore(g_buffer_mutex);

	for (bp = &fs_bufs[0]; bp < &fs_bufs[NR_BUFS]; bp++){
		if (bp->b_block == block && bp->b_dev == dev && bp->b_count > 0){
			unlock_semaphore(g_buffer_mutex);
			return bp;
		}		
	}

	unlock_semaphore(g_buffer_mutex);
	return NULL_BUF;
}



void flush_all(dev_t dev)
{
	register buffer_t* bp;

	lock_semaphore(g_buffer_mutex);

	for (bp = &fs_bufs[0]; bp < &fs_bufs[NR_BUFS]; bp++){
		if ( bp->b_dirt == 1 &&bp->b_dev == dev){
			buf_release(bp);
		}
	}

	unlock_semaphore(g_buffer_mutex);
}


off_t block_to_sector(off_t block, int blocksize)
{
	 off_t sector;
	sector = block*((blocksize+HD_SECTOR_SIZE_MASK)/HD_SECTOR_SIZE);
	return sector;
}

static int init_buf_blk(buffer_t *bp,int dev,  int blk, int size)
{
	assert(bp);

	bp->b_dev = dev;
	bp->b_block = blk;

	if (size != bp->b_size)
	{
	  if(bp->b_data)
		  kfree(bp->b_data);

	  bp->b_data = (void *)kmalloc(size,0);
	  bp->b_size = size;
	}

	if (bp->b_data == NULL)
	{
		printk("no mem at %x\n", dev);
		buf_release(bp);
		return -1;
	}

	memset(bp->b_data, 0, bp->b_size);
	return 0;
}


buffer_t* bread_with_size(dev_prvi_t* dev, off_t block, int sz)
{
	buffer_t *bp;	/* buffer pointer */
	int retval;
	int times = 5;

	if (!dev)
		return NULL;

	bp = buf_find(dev->devno, block);

    if(bp){
		bp->b_count ++;
	    return bp;
	}
	
	bp = buf_getblk();

	if (bp != NULL_BUF)
	{
		retval = init_buf_blk(bp, dev->devno, block,sz);
		if (retval)
		{
			return NULL_BUF;
		}

		if(dev_read((dev), block, (void *)bp->b_data, bp->b_size) < 0)
		{
			kprintf("bread_with_size: read %x %d failed\n",dev->devno, bp->b_size);
			buf_release(bp);
			return NULL_BUF;
		}
		
		return bp;
	}	
		
	
	panic("%s got line %d\n",__FUNCTION__, __LINE__);
	return NULL_BUF;
}


buffer_t* bread(dev_prvi_t* dev, off_t block)
{
	if(!dev)
	  return NULL;

	return bread_with_size(dev, block, dev_block_size(dev->devno));
}


void sync_blks()
{
  register buffer_t *bp;

  lock_semaphore(g_buffer_mutex);

  for (bp = &fs_bufs[0]; bp < &fs_bufs[NR_BUFS]; bp++) {
	if(bp->b_dirt)
		flush_buf(bp);
  }

  unlock_semaphore(g_buffer_mutex);
}

void mark_buf_dirty(buffer_t *bp)
{
	assert(bp);
	bp->b_dirt = 1;
}

void free_cblock(dev_prvi_t* fd, int blk_no)
{
	buffer_t* bp;
	bp = buf_find(fd->devno, blk_no);
	if (bp)
	{
		buf_release(bp);
	}
}

char *get_cblock(dev_prvi_t* fd, int blk_no, int blk_size)
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



void mark_dirty( dev_prvi_t* dev, off_t blk, size_t nBlockCount )
{
	buffer_t *bp;	/* buffer pointer */
	bp = buf_find(dev->devno,blk);
	if (!bp)
	{
		return ;
	}
	mark_buf_dirty(bp);
}


static buffer_t fs_bufs[NR_BUFS];
//buffer_t *buf_head;	/* points to最少 least recently used free block */
// buffer_t *buf_tail;	/* points to 最多most recently used free block */

