
/**
 * jicama.c - NTFS support Part of the Linux-NTFS project.
 *
 * Copyright (c) 2001-2005 Anton Altaparmakov
 * Copyright (c) 2006-2007 Easion Deng
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <ntfs.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>

static void ntfs_hook();
static void remove_ntfs_hook();

int dll_main(char **args)
{
	int error;

	kprintf("NTFS Module Running ...\n");

	ntfs_hook();

	//panic("error = %d\n", error);
	return error;
}


int dll_destroy()
{
	remove_ntfs_hook();
	return 0;
}

/********************************************/

static  fs_dev_ops_t _ntfs=
{
	fs_name: "ntfs",
	fs_copyright:"GPL",
	fs_author:"Anton Altaparmakov",
	fs_bmap: NULL,
	fs_opendir: ntfs_opendir,

	fs_readdir: ntfs_readdir2,
	fs_probe:ntfs_probe,
	fs_mount:ntfs_mount,
	fs_unmount:ntfs_unmount,
	fs_read:ntfs_fread,
	fs_write:ntfs_fwrite,
	fs_mkdir:_linux_ntfs_mkdir,
};


static void ntfs_hook()
{
	install_fs(&_ntfs);
}

static void remove_ntfs_hook()
{
	deinstall_fs(&_ntfs);
}



int dll_version()
{
	kprintf("NTFS File system Driver for Jicama OS\n");
	kprintf("Code Taken From Linux2.4.32 and unixutc0.61\n");
	return 0;
}

/********************************************/

void shutdown_device_cache( dev_t nDevice )
{
}

int setup_device_cache( dev_t nDevice, int nFS, off_t nBlockCount )
{
	return 0;
}

#define NTFS_TIME_OFFSET ((bigtime_t)(369*365 + 89) * 24 * 3600 * 10000000)

bigtime_t ntfs_ntutc2unixutc( bigtime_t ntutc )
{
	return ( ntutc - NTFS_TIME_OFFSET ) * 10LL;
}

bigtime_t ntfs_unixutc2ntutc( bigtime_t t )
{
	return ( t / 10LL ) + NTFS_TIME_OFFSET;
}

#undef NTFS_TIME_OFFSET


void free_cblock(int fd, int blk_no)
{
	buffer_t* bp;
	bp = buf_find(fd, blk_no);
	if (bp)
	{
		buf_release(bp);
	}
}

unsigned long block_to_sector(unsigned long block, int blocksize)
{
	 unsigned long sector;
	sector = block*((blocksize+HD_SECTOR_SIZE_MASK)/HD_SECTOR_SIZE);
	return sector;
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

void mark_dirty( dev_t dev, off_t blk, size_t nBlockCount )
{
	buffer_t *bp;	/* buffer pointer */
	bp = buf_find(dev,blk);
	if (!bp)
	{
		return ;
	}
	mark_buf_dirty(bp);
}

