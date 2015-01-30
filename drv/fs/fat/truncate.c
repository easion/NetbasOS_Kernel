

#include <drv/drv.h>
#include <drv/fs.h>
#include "fat.h"
#include <drv/errno.h>
#include <drv/buffer.h>
#include <assert.h>
void fat12_delc( unsigned short cluster );
int fat12_sync();

extern void set_fftime(struct fdate* fdate, struct ftime* ftime);

int fat_write_node(inode_t * inode)
{
	int i;
	buffer_t * bh;
	int block = inode->i_father;
	int node=inode->i_number;
   struct msdos_dir *dir;
	mount_t *mp = get_superblock(inode->i_dev);

	if (!mp)
	{
	  return -1;
	}

	i = msdos_cluster2sec(mp, block);
	if(!i)return -1;

	if (!(bh=bread(get_dev_desc(inode->i_dev),i)))
		panic("unable to read i-node block");

	i=(inode->i_seek%HD_SECTOR_SIZE);

	dir = (struct msdos_dir *)(&bh->b_data[i]);
	
	mark_buf_dirty(bh);
	dir->file_size=inode->i_size;
	dir->first_cluster=node;
	set_fftime(&dir->date, &dir->time);

	buf_release(bh);
	sync_blks();
	return 0;

}


int fat_unlink (inode_t* p_inode, unsigned char* name)
{
	file_t fp;
	int  offset;
	struct msdos_dir rdir;
	dev_t dev=p_inode->i_dev;
	int block=p_inode->i_number;
	inode_t *inode=iget(dev, block);
	mount_t *mp = get_superblock(inode->i_dev);

	if (!mp)
	{
	  return -1;
	}

	memset((void *)&fp, 0, sizeof(file_t));

	fp.f_mode = O_RDONLY;
	fp.f_pos = 0;
	fp.f_inode = inode;

	offset = msdos_find_file(&fp,  name, &rdir);

	if(offset<0){
		printk("%s(): %s not found [block: %d]\n", __FUNCTION__, name, block);
		return ERR;
	}

	rdir.file_name[0] = 0xe5;
	//fat_truncate(mp, rdir.first_high<<16|rdir.first_cluster);
	fat_free_file(mp, rdir.first_high<<16|rdir.first_cluster);
	rdir.first_high=0;
	rdir.first_cluster = 0;

	if(fp.f_pos>=offset)
		fp.f_pos-=32;
	else	{
		panic("%s: pos < offset[%d]", __FUNCTION__, offset);
		return -1;
	}

	fwrite(&fp, (void *)&rdir, 32);

	iput(inode);
	fat_table_sync(mp);
	sync_blks();

	return OK;
}
