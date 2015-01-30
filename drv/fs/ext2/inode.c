/*
**     (R)Jicama OS
**     Ext2 Index Node 
**     Copyright (C) 2003 DengPingPing
*/
#include "ext2.h"

extern int ext2_readdir(inode_t* inode);

int ext2_readnode(inode_t* inode)
{
	buffer_t * buff;
	struct ext2_inode * raw_inode;
	struct ext2_group * group;
	unsigned long block_group;
	unsigned long group_desc;
	unsigned long desc;
	unsigned long block;
	int i;

	block_group = (inode->i_number - 1) / sp_info.s_inodes_per_group;  ///所在组
	if (block_group >= sp_info.group_nr){
		printk ("read inode:Biger than group number\n");
		return -1;
	}
	
	group_desc = block_group / sp_info.s_desc_per_block; ////描叙符所在块
	desc = block_group % sp_info.s_desc_per_block;  ////描叙符位置
	
#if IN_DISK       //////////////read information from disk
	buff = bread(inode->i_dev, 2+group_desc);
	group = (struct ext2_group*)buff->b_data;
#else      //////////////read information from memory
	group = (struct ext2_group *)sp_info.group_desc[group_desc];
	if (!group){
	printk ("Group Descriptor not loaded\n");
	return;
	}
#endif
		
	block = group[desc].bg_inode_table +
		(((inode->i_number - 1) % sp_info.s_inodes_per_group)
		 /  sp_info.s_inodes_per_block);    ///////具体节点

       #if IN_DISK
		memset(buff->b_data,0,1024);  //////////reset buffer!
	   #endif

	if (!(buff = bread (inode->i_dev, block))){
		printk (" Unable to read disk \n");
		return -1;
	}
	
	raw_inode = ((struct ext2_inode *) buff->b_data) +
		(inode->i_number - 1) %  sp_info.s_inodes_per_block;

	//printk("file mode is:%i\n",raw_inode->i_mode);

	//inode->i_size = raw_inode->i_size;
	inode->i_mode = raw_inode->i_mode;
	//inode->i_ctime = raw_inode->i_ctime;

	inode->i_private_data = buff;

	//buf_release(buff);
	return 0;
}


inode_t *ext2_iget(const int dev, const int inode_nr)
{
		inode_t* newnode;

		newnode = iget(dev, inode_nr);
		ext2_readnode(newnode);

         return newnode;
}