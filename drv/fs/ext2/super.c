/*
**     (R)Jicama OS
**     Ext2 File system Super Block
**     Copyright (C) 2003 DengPingPing
*/
#include "ext2.h"

 // Old file losted, rewrite this.
struct ext2_info sp_info;
struct ext2_super* ext2;
inode_t* ext2_root;
int ext2_probe(mount_t *mp);
int ext2_init(mount_t *mp);
int ext2_bmap(inode_t* inode, int offset, int create);
inode_t* ext2_opendir(inode_t* old, unsigned char *name);


 fs_dev_ops_t ext2_fs=
{
	fs_name: "ext2fs",
	fs_bmap: ext2_bmap,
	fs_probe:ext2_probe,
	fs_mount:ext2_init,
	fs_opendir: ext2_opendir,

	/*
	fs_readdir: ext2_readdir,
	fs_write_inode:ext2_write_node,

	fs_creat:ext2_create,
	fs_unlink:ext2_unlink,
	fs_mkdir:ext2_mkdir,
	fs_truncate:ext2_truncate,
	*/
};

void ext2_hook()
{
	install_fs(&ext2_fs);
}

void remove_ext2_hook()
{
	deinstall_fs(&ext2_fs);
}



extern void ext2_load_gdesc(int dev);

int ext2_probe(mount_t *mp)
{
	buffer_t* buff;

	buff = bread(mp->m_dev, 1);  //Super include begin sector!
	ext2 = (struct ext2_super*)buff->b_data;

	if (ext2->s_magic != EXT2_MAGIC)
		return	-1;
	release_blk(buff);
	return 0;
}

int ext2_init(mount_t *mp)
{
	buffer_t* buff;

	buff = bread(mp->m_dev, 1);  //Super include begin sector!
	ext2 = (struct ext2_super*)buff->b_data;

	if (ext2->s_magic != EXT2_MAGIC)
		panic("Spurious Ext2 Partition!\n");

    sp_info.block_size = (1<<ext2->s_log_block_size);
	sp_info.s_inodes_per_group = ext2->s_inodes_per_group;
	sp_info.s_inodes_per_block = sp_info.block_size*512/sizeof(struct ext2_inode);
	sp_info.s_block_per_group = ext2->s_inodes_per_group / sp_info.s_inodes_per_block;
    sp_info.s_desc_per_block = (sp_info.block_size*512) / sizeof(struct ext2_group);
	sp_info.group_nr = (ext2->s_blocks_count - ext2->s_first_data_block+ext2->s_blocks_per_group - 1)
		/ ext2->s_blocks_per_group;
    
	ext2_load_gdesc((int)mp->m_dev);
	ext2_root = ext2_iget((int)mp->m_dev->devno, EXT2_ROOT);
	mp->m_root = ext2_root;

	 //mp->m_root = ext2_root;

	 buf_release(buff);
	 return OK;
}


