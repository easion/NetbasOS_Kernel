/*
**     (R)Jicama OS
**     Ext2 FS File IO Interface
**     Copyright (C) 2003 DengPingPing
*/

#include "ext2.h"

 extern fs_dev_ops_t ext2_fs;

int ext2_bmap(inode_t* inode, int offset, int create)
{
 	int idx;
	struct ext2_inode *ext2_file = inode->i_private_data;

	idx = offset/ext2_fs.fs_blksz;

	if (idx<15){
		return ext2_file->i_block[idx];   
	}

	 return 0;   
}

#if 0
static void ext2_doread(inode_t* inode, unsigned char* file_buff)
{
	 int i;
    buffer_t* buff = NULL_BUF;
	struct ext2_inode *ext2_file = inode->i_private_data;

	for(i=0;i < ext2_file->i_blocks ; i++ )  ////////这里进入读数据的循环
	{	
        if(i > 11 ) return;
	     buff = bread (inode->i_dev, ext2_file->i_block[i]);     ///////读取直接数据块
		strcpy(&file_buff[i*1024], buff->b_data);  ///////核心语句，数据指向缓冲
		buf_release(buff);
	}
}
 
 void ext2_stat(inode_t* inode, unsigned char* file)
{
	inode_t* node;

    node = ext2_file2node(inode,file);
	if (!node)return;

	printk("%s Information:\n\n", file);
	
	out_stat(node);
	return;
}
#endif

