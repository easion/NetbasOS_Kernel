/*
**     (R)Jicama OS
**     Microsoft File Allocation Table
**     Copyright (C) 2003 DengPingPing
*/


#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include "fat.h"
#include <assert.h>

/*
typedef struct 
{
	u8_t data[ FAT12_PHY_SIZE ];
}  __attribute__ ((packed)) FAT12_t;

typedef struct 
{
	u16_t data[ FAT12_LOG_ITEM ];
} __attribute__ ((packed)) logical_FAT12_t;
static FAT12_t fat_in_disk;
static logical_FAT12_t fat_logical;
*/

static u8_t* fat_in_disk;
static u16_t* fat_logical;



static bool fat12_dirty_flag = FALSE;

u16_t fat12_nextc(u16_t cluster)
{
	return fat_logical[ cluster ];
}

void fat12_setc(u16_t cluster, u16_t value)
{
	 fat_logical[ cluster ] = value;
	 fat12_dirty_flag = TRUE;
}


static int msdos_savefat12fd(mount_t*mp)
{
  int i;
  unsigned char *p = NULL;
  buffer_t*bp;
	struct msdos_info *fat_info_ptr = mp->m_private_data;

  p = (unsigned char*) fat_in_disk;
  if( !fat12_dirty_flag) return OK;

  //printf("start write fattable\n");

 	for(i=FAT_START_BLK; i<FAT_START_BLK+fat_info_ptr->fat_size;i++){
		bp=bread(mp->m_dev, i);
		if (!bp)
		{
			break;
		}
		memcpy(bp->b_data,p+(i-FAT_START_BLK)*bp->b_size,bp->b_size);
		mark_buf_dirty(bp);
		buf_release(bp);
	}

 	
	 fat12_dirty_flag = FALSE;
  printf("start write fattable ok\n");
	
  return OK;
}


int fat12_sync(mount_t*mp)
{
	int i, j;
	struct msdos_info *fat_info_ptr = mp->m_private_data;

  if( !fat12_dirty_flag)return OK;
	memset(fat_in_disk, 0, 512*fat_info_ptr->fat_size);

	// Converts the FAT from the logical structure into the
	// physical structure.
	for( i = 0, j = 0; j < 3072; j += 2 )
	{
		fat_in_disk[ i++ ] = (u8_t)fat_logical[j];
		fat_in_disk[ i++ ] = (u8_t)((fat_logical[j]>>8)&(0x0F))|((fat_logical[j+1]<<4)&(0xF0));
		fat_in_disk[ i++ ] = (u8_t)(fat_logical[j+1]>>4);
	}

	msdos_savefat12fd(mp);	
	return 0;
}

void fat12_conv()
{
	int i, j;

	memset((char *)fat_logical, 0, 3072*2);

	for( i = 0, j = 0; i < 4608; i += 3 )
	{
		fat_logical[ j++ ] = 
			(fat_in_disk[i] + (fat_in_disk[i+1] << 8)) & 0x0FFF;
		fat_logical[ j++ ] = 
			(fat_in_disk[i+1] + (fat_in_disk[i+2] << 8)) >> 4;
	}
}




int msdos_unload_fat(const int dev,int secs)
{
	if(fat_in_disk)
		kfree(fat_in_disk);
	if(fat_logical)
		kfree(fat_logical);
	return 0;
}

int msdos_load_fat12(const int dev,int secs)
{
	int i=0,  n=0, nxt=0, y=0;
	unsigned char *p ;
	int dsk_size=512*secs;
	int logi_size=512*secs*3/2;

	fat_in_disk = kmalloc(dsk_size,0);
	if (!fat_in_disk)
		return -1;

	fat_logical = kmalloc(logi_size,0);
	if (!fat_logical)
		return -1;

	p = fat_in_disk;

	memset(fat_in_disk, 0,  dsk_size);

	for(i=FAT_START_BLK; i<secs+FAT_START_BLK;i++)
		dev_read(dev,  i,  fat_in_disk+(i-FAT_START_BLK)*512, 512);

   if((i=(*(unsigned short *)(unsigned long)p & 0xff) )== 0xf0)
	   kprintf ("FAT  Media floppy\n");
  

	fat12_conv();

	for (i=0; i< logi_size/2; i++)
	{
	  nxt = fat12_nextc(i);
	  if(nxt == 0)	  n++;
	 // if(nxt>0 && nxt <20)
		 // kprintf("(%d=%d) ", i, nxt);

	  if(FAT12_EOC(nxt))
		  y++;

	// if(nxt>=2880)
		 // kprintf("%d=%Y ", i, nxt);
	}
	fat12_conv();

	kprintf("\nFAT Table: %d block used, %d for Free %d files\n",   logi_size/2, n, y);
	return 0;
}


struct msdos_info *msdos_dev2info(dev_t dev);
u32_t msdos_next(mount_t *mp, u32_t cluster);
bool msdos_eof(mount_t *mp, unsigned int cluster);

#define USING_DISK

//#ifndef USING_DISK
#if 0
static bool fat32_dirty_flag = FALSE;
static unsigned long *lfat32;

static u32_t fat32_nextc(u32_t actual)
{
	return lfat32[ actual ];
}

static void fat32_setc(u32_t actual, u32_t value)
{
	 lfat32[ actual ] = value;
	 fat32_dirty_flag = TRUE;
}



int msdos_load_fat32(const dev_prvi_t* dev)
{
	u32_t i,len;
	u32_t _start,_end;
	char *buffer;
	struct msdos_info *info = msdos_dev2info(dev->devno);

	assert (info != NULL);
	
	len=info->fat_size*HD_SECTOR_SIZE;

	_start=info->fat_base+1;
	_end=info->fat_base+1+info->fat_size;

	buffer=(char *)kmalloc(len,0);
	bzero(buffer,len);

	lfat32=(u32_t *)buffer;

	if (!buffer)
	{
		printf("%s(): no mem\n", __FUNCTION__);
		return;
	}

	kprintf("fat %d - length %d \nloading  fat:", _start, _end-_start);

	for (i= _start; i<_end; i++)
	{
		dev_read(dev, i, buffer, HD_SECTOR_SIZE);
		kprintf("\x1B[s [%d]", i);
		kprintf("\x1B[u");
		 buffer+=HD_SECTOR_SIZE;
		mdelay(500);
	}
		kprintf("\n");
}

void msdos_save_fat32(const dev_prvi_t* dev)
{
	u32_t i;
	u32_t start,end;
	char *buffer;
	struct msdos_info *info = msdos_dev2info(dev->devno);

	if (info == NULL)
	{
		panic("info error\n");
	}
	start=info->fat_base+1;
	end=info->fat_base+1+info->fat_size;

	buffer=(char *)lfat32;

	if (!buffer)
	{
		return;
	}

	for (i= start; i<end; i++)
	{
		 dev_write(dev, i, buffer, HD_SECTOR_SIZE);
		 buffer+=HD_SECTOR_SIZE;
	}

}


#else
int msdos_load_fat32(int dev)
{
	return 0;
}
#endif


#define FAT32_PER_SEC  (HD_SECTOR_SIZE/sizeof(unsigned long))
#define FAT16_PER_SEC  (HD_SECTOR_SIZE/sizeof(unsigned short))

typedef struct 
{
	unsigned long fatno:12;
}fat12_type_t __attribute__((packed));

#define	peek8(x)	(*(u8_t *)(x))
#define	peek12(x)	(*(fat12_type_t *)(x))
#define	peek16(x)	(*(u16_t *)(x))
#define	peek32(x)	(*(u32_t *)(x))
#define	peek64(x)	(*(u64_t *)(x))

#define	poke8(x,n)	(*(u8_t *)(x)=((u8_t)(n)))
#define	poke16(x,n)	(*(u16_t *)(x)=((u16_t)(n)))
#define	poke32(x,n)	(*(u32_t *)(x)=((u32_t)(n)))
#define	poke64(x,n)	(*(u64_t *)(x)=((u64_t)(n)))

static unsigned int last_fat = 2;
void fat12_setc(u16_t actual, u16_t value);

bool fat_next_useable(mount_t *mp, u32_t *next, u32_t actual);
struct msdos_info *msdos_dev2info(dev_t dev);



unsigned int msdos_12next(dev_prvi_t* dev, const int cluster)
{
	int blk=0, offset=0, next=0,t=0;
	struct msdos_info *info = msdos_dev2info(dev->devno);
	buffer_t* bp;

	assert (info);

	if(cluster > FAT12_MAGIC)
	   panic("%d: is a Bad Cluster!\n",cluster);

	blk = info->fat_base;
	blk += (cluster*3/2) / HD_SECTOR_SIZE;
	offset = (cluster*3/2) % HD_SECTOR_SIZE;

	bp = bread_with_size(dev, blk, HD_SECTOR_SIZE);
	if (!bp){
		return FAT32_MAGIC;
	}

	if((offset & 511) == 511){
		next = peek8(bp->b_data+offset);
		buf_release(bp);
		bp = bread_with_size(dev, blk+1, HD_SECTOR_SIZE);
		if (!bp){
			return FAT32_MAGIC;
		}
		next |= peek8(bp->b_data+0)<<8;
	}else{
		next = peek16(bp->b_data+offset);	
	}

	buf_release(bp);

    if (cluster & 1) {
      next >>=  4;
	}
	else {
		next &= 0x00000fff;
	}
	return next;
}

unsigned int msdos_12_setc(dev_prvi_t* dev, const int cluster, const int value)
{
	int blk=0, offset=0, next=0,t=0;
	struct msdos_info *info = msdos_dev2info(dev->devno);
	buffer_t* bp;
	u16_t val = value&0x0fff;

	assert (info);

	if(cluster > FAT12_MAGIC)
	   panic("%d: is a Bad Cluster!\n",cluster);

	blk = info->fat_base;
	blk += (cluster*3/2) / HD_SECTOR_SIZE;
	offset = (cluster*3/2) % HD_SECTOR_SIZE;

	bp = bread_with_size(dev, blk, HD_SECTOR_SIZE);
	if (!bp){
		return FAT32_MAGIC;
	}


	if((offset & 511) == 511){
		next = peek8(bp->b_data+offset);
		poke8(bp->b_data+offset, val&0xff);
		mark_buf_dirty(bp);
		buf_release(bp);

		bp = bread_with_size(dev, blk+1, HD_SECTOR_SIZE);
		if (!bp){
			return FAT32_MAGIC;
		}

		next = peek8(bp->b_data+0);
		poke8(bp->b_data+0, (val&0xf00)>>4 | (next&0x0f));
	}else{
		next = peek16(bp->b_data+offset);
		goto done;
	}

    if (cluster & 1) {
		//printk("*");
      val <<=  4;
	  val = val | (next & 0x0f);
	}
	else
	{
		next &= 0xf000;
		//printk("&");
		val &= 0x00000fff;
		val |= next;
	}

	poke16(bp->b_data+offset, val);
done:
	mark_buf_dirty(bp);
	buf_release(bp);

	return next;
}

unsigned long msdos_16next(dev_prvi_t* dev, unsigned int cluster)
{
	int blk, offset;
	unsigned long	get=0;
	buffer_t* bp;
	struct msdos_info *info = msdos_dev2info(dev->devno);

	assert(info);

	cluster &= 0xfffffff;

	if(cluster >= FAT16_MAGIC)
		panic("%d, is a Bad Cluster!\n",cluster);

	blk = (info->fat_base+(cluster/FAT16_PER_SEC));
	offset = (cluster % FAT16_PER_SEC);

	bp = bread_with_size(dev, blk, HD_SECTOR_SIZE);
	if (!bp)
	{
		return FAT32_MAGIC;
	}

	get = peek16(bp->b_data+offset*sizeof(u16_t));
	buf_release(bp);

	return get;
}


void fat16_setc(dev_prvi_t* dev, u32_t cluster, u32_t value)
{
	int blk, offset;
	unsigned long	get=0;
	struct msdos_info *info = msdos_dev2info(dev->devno);
	buffer_t* bp;

	assert (info );
	
	cluster &= 0xfffffff;
	if(cluster >= FAT16_MAGIC)
		panic("%d, is a Bad Cluster!\n",cluster);

	blk = (info->fat_base+(cluster/FAT16_PER_SEC));
	offset = (cluster % FAT16_PER_SEC);
	bp = bread_with_size(dev, blk, HD_SECTOR_SIZE);
	if (!bp)
	{
		return FAT32_MAGIC;
	}

	get = poke16(bp->b_data+offset*sizeof(u16_t), value);
	mark_buf_dirty(bp);
	buf_release(bp);
}



unsigned long msdos_32next(dev_prvi_t* dev, unsigned int cluster)
{
	int blk, offset;
	unsigned long	get=0;
	struct msdos_info *info = msdos_dev2info(dev->devno);
	buffer_t* bp;

	assert (info != NULL);

	cluster &= 0xfffffff;
	if(cluster >= FAT32_MAGIC)
		panic("%d, is a Bad Cluster!\n",cluster);

	blk = (info->fat_base+(cluster/FAT32_PER_SEC));
	offset = (cluster % FAT32_PER_SEC);

	bp = bread_with_size(dev, blk,HD_SECTOR_SIZE);
	if (!bp)
	{
		return FAT32_MAGIC;
	}

	get = peek32(bp->b_data+offset*sizeof(u32_t));
	buf_release(bp);	
	return get;
}


void fat32_setc(dev_prvi_t* dev, u32_t cluster, u32_t value)
{
	int blk, offset;
	unsigned long	get=0;
	buffer_t* bp;

	struct msdos_info *info = msdos_dev2info(dev->devno);

	assert (info );
	cluster &= 0xfffffff;
	if(cluster >= FAT32_MAGIC)
		panic("%d, is a Bad Cluster!\n",cluster);

	blk = (info->fat_base+(cluster/FAT32_PER_SEC));
	offset = (cluster % FAT32_PER_SEC);
	bp = bread_with_size(dev, blk,HD_SECTOR_SIZE);
	if (!bp)
	{
		return FAT32_MAGIC;
	}

	get = poke32(bp->b_data+offset*sizeof(u32_t), value);
	mark_buf_dirty(bp);
	buf_release(bp);	
}

