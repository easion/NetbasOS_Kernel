/*
**     (R)Jicama OS
**     Microsoft File Directory Table
**     Copyright (C) 2003 DengPingPing
*/

#include <drv/drv.h>
#include <drv/fs.h>
#include "fat.h"
#include <drv/errno.h>
#include <drv/buffer.h>
#include <drv/log.h>

 int fat_dir_update(file_t* f)
{
	file_t fa;
	int n;
	struct msdos_dir dirent;
	inode_t *node = iget(f->f_inode->i_dev, f->f_inode->i_father);
	int sz=MAX(f->f_inode->i_size,f->f_pos);
	u8_t att=0;
	unsigned long start = f->f_inode->i_number;
	u16_t date=0;
	u16_t time=0;

	fa.f_mode = O_RDWR;

	fa.f_inode = node;
	fa.f_pos = 0;

	while (TRUE)
	{
		n = do_filp_read(&fa, (void *)&dirent, sizeof(struct msdos_dir));
		if(n <= 0)
			break;
		if(dirent.file_name[0] == 0xe5)
			continue;
		if(dirent.first_cluster == start){
			printf("found cluster %d size=%d\n",start,sz);
			break;
		}
	}

	if(n<=0){
		syslog(4,"%s():read null, father block is %d\n", __FUNCTION__, f->f_inode->i_father);
		return ERR;
	 }

	/*if(start != 0){
		f->f_inode->i_number=start;
	}*/
	dirent.first_cluster=start;

   if(att != 0){
	   if(MSDOS_ISREG(dirent.attribute) &&MSDOS_ISDIR(att)) 
		   goto err;
		if (MSDOS_ISREG(att) &&MSDOS_ISDIR(dirent.attribute))
			goto err;
		dirent.attribute=att;
	}

	dirent.file_size=sz;
	//f->f_inode->i_size=sz;

	fa.f_pos -= 32;
	fwrite(&fa, (void *)&dirent, sizeof(struct msdos_dir));
	iput(node);
	return OK;

err:
	syslog(4,"ops not supported!\n");
	iput(node);
	return ERR;
}

//get_unix_time();

