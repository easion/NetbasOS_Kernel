/*
**     (R)Jicama OS
**     Microsoft FAT file Create
**     Copyright (C) 2003 DengPingPing
*/

#include <drv/drv.h>
#include <drv/fs.h>
#include "fat.h"
#include <drv/errno.h>
#include <drv/buffer.h>
#include <drv/ansi.h>
#include <assert.h>

int  dir_update(int dev, int block, char *n1, char* n2, int f_size, int a);
int msdos_modify_dir(struct msdos_dir *item, char *name, int cluster, int attribute, int size);
 int fat_find_free (mount_t *);
int  msdos_create_file(int dev, int block, char *name, int ftype);
u32_t msdos_next(mount_t *mp, u32_t cluster);
bool msdos_eof(mount_t *mp, unsigned int cluster);
void strtofile(const char *str,char *s);



static int msdos_modify_dir(struct msdos_dir *item, char *name,
	int cluster, int attribute, int size)
{
	bzero(item, sizeof(struct msdos_dir));

    strtofile(name, item->file_name);

    item->file_size = size;
    item->first_high = cluster>>16;
    item->first_cluster = cluster;

	set_fftime(&item->date, &item->time);
	set_fftime(&item->cdate, &item->ctime);
	set_ffattribute(item, attribute);
    return OK;
}

void dosconv(vfs_dirent_t* dirent_entry, fat_dir_entry * fatdir)
{
	fatdir->attribute=dirent_entry->l_attribute;
	*(unsigned short*)&fatdir->cdate = dirent_entry->l_ctime >> 16;
	*(unsigned short*)&fatdir->ctime = dirent_entry->l_ctime  & 0xffff;
	*(unsigned short*)&fatdir->adate = dirent_entry->l_atime >> 16;

	*(unsigned short*)(&fatdir->time)  = dirent_entry->l_mtime & 0xff;
	*(unsigned short*)(&fatdir->date) = dirent_entry->l_mtime>>16;

	fatdir->file_size =  dirent_entry->l_size_low;
	fatdir->first_cluster = *(unsigned long*)&dirent_entry->l_res[0];
	fatdir->first_high = *(unsigned long*)&dirent_entry->l_res[4];
	 //kprintf(" *(unsigned long*)&dirent_entry->l_res[0]  are %d\n",  *(unsigned long*)&dirent_entry->l_res[0] );
}

void convdos(vfs_dirent_t* dirent_entry, fat_dir_entry * fatdir)
{
	  dirent_entry->l_attribute   = fatdir->attribute;
	  dirent_entry->l_ctime  = *(unsigned short*)&fatdir->ctime + (*(unsigned short*)&fatdir->cdate << 16);
	  dirent_entry->l_atime  = *(unsigned short*)&fatdir->adate << 16;
	   dirent_entry->l_mtime  = *(unsigned short*)(&fatdir->time) 
		  + ((*(unsigned short*)(&fatdir->date)) << 16);
	  dirent_entry->l_size_high = 0;
	  dirent_entry->l_size_low = fatdir->file_size;
	 dirent_entry->d.l_ino = fatdir->first_cluster | fatdir->first_high << 16;
	 //kprintf(" *(unsigned long*)&dirent_entry->l_res[0]  is %d\n",  *(unsigned long*)&dirent_entry->l_res[0] );
}

int  msdos_find_file(file_t *fp,char *name, struct msdos_dir *buf_addr)
{
	int off=0;
	int ret;
	vfs_dirent_t vfd;
	char *buff;

	while (1)
	{
		memset(&vfd,0,sizeof(vfd));
		ret = msdos_do_readdir(fp, &vfd, FALSE, &off);

		buff=vfd.l_long_name;

		if (ret<0)
			break;

		if (!buff[0])
			continue;

		dosconv(&vfd, buf_addr);

		if (strcmp(strlwr(name),strlwr(buff))==0){
				return off;
		}
	}


	return -ENOENT;
}


static int  msdos_get_dir_slot(file_t *fp,char *name, struct msdos_dir *buf_addr)
{
	int off=0;
	int ret;
	vfs_dirent_t vfd;
	char *buff;

	while (1)
	{
		memset(&vfd,0,sizeof(vfd));
		ret = msdos_do_readdir(fp, &vfd, TRUE, &off);

		buff=vfd.l_long_name;

		if (ret<0)
			break;

		if (!buff[0])
			break;

		dosconv(&vfd, buf_addr);

		if (strcmp(strlwr(name),strlwr(buff))==0){		
			return -EEXIST;
		}
	}

	return off;
}


buffer_t *fat_find_file(inode_t *inode, char *name, int *pos)
{
	int i, find=0;
	buffer_t * bh;
	int block=inode->i_number;
    struct msdos_dir *dir;
	unsigned char buff[20];
	mount_t *mp = get_superblock(inode->i_dev);

	if (!mp)
	{
		kprintf("no found super\n");
		return NULL;
	}

start:
	if (!(bh = bread(get_dev_desc(inode->i_dev),block)))
		panic("unable to read i-node block");

	dir = (struct msdos_dir *)bh->b_data;

	for (i=0; i<dev_block_size(inode->i_dev)/32+1; i++){
		if(dir->file_name[0] == 0xe5)	continue;
		fat_expand_name(dir, buff);
		if (!strcmp(strlwr(name),strlwr(buff))){
			find=1;
			*pos=i;
			break;
		}
		dir++;
	}
	
	if(find==0){ /*if found it*/
			if(msdos_eof(mp, block))
			return (buffer_t *)0;
	else {
			buf_release(bh);
			block = msdos_next(mp, block);
			goto start;
		}
	}

	return bh;
}

int  msdos_create_file(int dev, int block, char *name, int ftype)
{
	file_t fp;
	struct msdos_dir new_dir;
	int  offset,cur_blk=0;
	int father_blk = block;
	inode_t *n=iget(dev, block);
	mount_t *mp = get_superblock(dev);

	if (!mp)
	  return -1;

   bzero((void *)&fp,  sizeof(file_t));

	fp.f_mode = O_RDWR;

	fp.f_pos = 0;
	fp.f_inode = n;
	n->i_size=512;

	printf("msdos_create_file %s\n",name);

	offset = msdos_get_dir_slot(&fp,  name, &new_dir);

	if(offset<0)return ERR;	

	cur_blk = fat_find_free(mp);
	msdos_set_end(mp, cur_blk);

	if(ftype & MSDOS_ARCH){
		msdos_modify_dir(&new_dir, name, cur_blk, ftype, 0);	
	}
	else  if(ftype & MSDOS_DIR){
	if(!cur_blk)return 0;
		msdos_modify_dir(&new_dir, name, cur_blk, ftype, 0);	
	}else{
		return -1;
	}

	if(fp.f_pos>=offset)
	{
		fp.f_pos-=32;
	}
	else
	{
		panic("%s(): pos < offset[%d]", __FUNCTION__, offset);
		return;
	}

	fwrite(&fp, (void *)&new_dir, 32);
	memset((void *)&new_dir, 0,32);
	fwrite(&fp, (void *)&new_dir, 32);  //fixit!

		/*here has a bug*/
	 if(ftype & MSDOS_DIR){
		iput(n);
		n=iget(dev, cur_blk);

		fp.f_mode = O_RDWR;
		fp.f_pos = 0;
		fp.f_inode = n;

		msdos_modify_dir(&new_dir, ".          ", cur_blk, MSDOS_DIR, 0);	
		memcpy(new_dir.file_name, ".          ", 11);
		fwrite(&fp, (void *)&new_dir, 32);

		msdos_modify_dir(&new_dir, "..         ", father_blk, MSDOS_DIR, 0);	
		memcpy(new_dir.file_name, "..         ", 11);
		fwrite(&fp, (void *)&new_dir, 32);

		bzero(&new_dir, 32);  /*weite the end*/
		fwrite(&fp, (void *)&new_dir, 32);

		kprintf("Dir  %s creat on node %d ok!", name, father_blk);
	 }

	iput(n);

	printf("sync fats\n");
	fat_table_sync(mp);
	printf("sync blks\n");
	sync_blks();
	return OK;
	
}

int  dir_update(int dev, int block, char *n1, char* n2, int f_size, int a)
{
	file_t fp;
	int  offset;
	struct msdos_dir _dir;
	inode_t *n=iget(dev, block);

   memset((void *)&fp, 0, sizeof(file_t));

	fp.f_inode = n;
	fp.f_mode = O_RDONLY;
	fp.f_pos = 0;

	offset = msdos_find_file(&fp,  n1, &_dir);

	if(offset<0)	return ERR;

	if(n2)   strtofile( n2, _dir.file_name);

	if(f_size)	_dir.file_size = f_size;

	if(a)	_dir.attribute = a;	

	if(fp.f_pos>=offset)
		fp.f_pos-=32;
	else
		panic("%s: pos < offset[%d]", __FUNCTION__, offset);

	fwrite(&fp, (void *)&_dir, 32);
	iput(n);
	sync_blks();
	return OK;
	
}

int fat_create (inode_t* n, unsigned char* file)
{
  return msdos_create_file(n->i_dev, n->i_number, file, MSDOS_ARCH);
}

int fat_mkdir(inode_t* n, char* dname)
{
   return msdos_create_file(n->i_dev, n->i_number, dname, MSDOS_DIR);
}









