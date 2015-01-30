
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
#include <net/inet.h>
#include "minixfs.h"

/********************************************/
off_t minixfs_bmap(inode_t* inode, int offset, int create);
int minixfs_inorw(mount_t *mp, inode_t *inode, int rw);
int minixfs_mkdir(inode_t* inode, char* dname,int nPerms);

static  fs_dev_ops_t _minixfs=
{
	fs_name: "minixfs",
	fs_copyright:"BSDL",
	fs_author:"Easion",
	fs_bmap: minixfs_bmap,
	fs_opendir: minixfs_opendir,
	fs_readlink:	minixfs_readlink,

	fs_readdir: minixfs_readdir,
	fs_probe:minixfs_probe,
	fs_mount:minixfs_mount,
	fs_unmount:minixfs_unmount,
	fs_inorw:minixfs_inorw,
	//fs_read:minixfs_fread,
	//fs_write:NULL,
	fs_mkdir:minixfs_mkdir,
};


void minixfs_hook()
{
	install_fs(&_minixfs);
}

void remove_minixfs_hook()
{
	deinstall_fs(&_minixfs);
}








#define LM_MAGIC	0x2478	/* V2, 30 char names */

int minixfs_probe(const mount_t *mp )
{
	char bootblock[512];
	int blocksize=512;
	int cnt;
	mfs_sb_t *minixsb = NULL;


	cnt = dev_read( mp->m_dev, 2, ( void * )bootblock, blocksize );

	if (cnt<=0){
		printf("minix probe error\n");
		return -1;
	}

		/* Check and print some info about it */
	minixsb = (mfs_sb_t *)bootblock;

#if 0
	printf("minixsb->s_ninodes = %d\n", conv2(minixsb->s_ninodes));
	printf("minixsb->s_nzones = %d\n", conv2(minixsb->s_nzones));

	printf("minixsb->s_imap_blocks = %d\n", conv2(minixsb->s_imap_blocks));
	printf("minixsb->s_zmap_blocks = %d\n", conv2(minixsb->s_zmap_blocks));
	printf("minixsb->s_firstdatazone = %d\n", conv2(minixsb->s_firstdatazone));
	printf("minixsb->s_log_zone_size = %d\n", conv2(minixsb->s_log_zone_size));
	printf("minixsb->s_max_size = %d\n", conv4(minixsb->s_max_size));
	printf("minixsb->s_magic = %x\n", conv2(minixsb->s_magic));
#endif




	if (minixsb->s_magic!=LM_MAGIC) {
		DBGOUT( "Ram disk image is not a minix image %x on fd %x\r\n", minixsb->s_magic,mp->m_dev->devno);
		return -1;
	}


	/* basic check */
	if((minixsb->s_imap_blocks < 1) || (minixsb->s_zmap_blocks < 1)
		|| (minixsb->s_ninodes < 1) || (minixsb->s_zones < 1)
		|| (minixsb->s_log_zone_size != 0)) /* XXX */
		{
			printf("error minixfs\n");
			return -1;
		}

	//CHECK CRC

	return 0;
}

int minixfs_mount(mount_t *mp, void *_data )
{
	int error;
	inode_t *tmp_root_inode;
	char bootblock[512];
	int blocksize=512;


	int i;
	mfs_sb_t *minixsb = NULL;

	blocksize = dev_read( mp->m_dev, 2, ( void * )bootblock, blocksize );

	if (blocksize<=0)return -1;

	/* Check and print some info about it 
	minixsb = (mfs_sb_t *)bootblock;
	if (minixsb->s_magic!=LM_MAGIC) {
		DBGOUT( "minixfs_mount: magic %x is not a minix fs by fd %x\r\n", minixsb->s_magic,mp->m_dev->devno);
		return -1;
	}*/

	minixsb = kmalloc(sizeof(mfs_sb_t),0);
	if (!minixsb)
	{
		return -1;
	}	
	memcpy(minixsb,(mfs_sb_t *)bootblock,sizeof(mfs_sb_t));
	mp->m_root_ino=ROOT_INODE;
	mp->m_count ++;
	mp->m_magic = MINIXFS_MAGIC;
	mp->m_private_data = minixsb;
	mp->m_blk_size = 1024;

	tmp_root_inode = iget(mp->m_dev->devno, ROOT_INODE);
	
	if (!tmp_root_inode)
		goto err1;

	mp->m_root=tmp_root_inode;

	return 0;

err1:
	return error;
}





int minixfs_unmount(mount_t *m, void *pVolume )
{
	return 0;
}
int minixfs_fread(file_t * filp, char * buf, int count)
{
	char tmpbuf[512];
	int blk,blk_offset;
	int retval;
	int bytes;
	u32_t ino = filp->f_inode->i_number;
	int fd = filp->f_inode->i_dev;
	int n=0;



	return n;
}


