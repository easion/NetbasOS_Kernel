
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------

#include "ext2.h"


inode_t* ext2_file2node(inode_t* current, unsigned char* file);

static int ext2_getentry(inode_t* inode, unsigned char* filename)
{
	int i,  node_no;
	int n = 	sp_info.block_size * 512;
	buffer_t* buff = NULL_BUF;
	struct ext2_entry2* dir;
	struct ext2_inode *node = inode->i_private_data;

	if (!filename[0])
		return 0;

	for(i=0;i < node->i_blocks/2 ; i++ )  ////////�����������ݵ�ѭ��
	{	
	   unsigned char *buf;

		if(i > 11 ) return 0;

		buff = bread (get_dev_desc(inode->i_dev), node->i_block[i]);     ///////��ȡֱ�����ݿ�
		buf = buff->b_data;  ///////������䣬����ָ�򻺳�

		while (n>0)
		{
			dir = (struct ext2_entry2 *)buf;
			if (dir -> rec_len == 0) 
			break;   ////Ŀ¼����Ϊ0������

			dir->name[dir->name_len] = '\0'; 
			if (!strcmp(filename,dir->name)){
			node_no = dir->inode;
			return node_no;
		}
		 
		   buf = buf + dir->rec_len;  ////////ʹ����ָ��ѹ��
	      n = n - dir->rec_len;  ////////û�д�ӡ��ʣ���ֽ�
		}
		buf_release(buff);
	}

	printk("can't find file%s\n", filename);
	return 0;
}
 


inode_t* ext2_file2node(inode_t*fp, unsigned char* file)
{
	int nr;
	inode_t* node;

	nr = ext2_getentry(fp, file);
	if (nr<2){
		printk("get file %s  inode number faild!\n",file);
		return 0;
	}

	node = ext2_iget(fp->i_dev, nr);
	if (!node){
		printk("read inode data faild!\n");
		return 0;
	}
	return node;
}

