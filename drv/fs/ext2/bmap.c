
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------
#include "ext2.h"

//zmap.c is implementation block allot and search free block

extern struct ext2_info sp_info;
extern struct ext2_super* ext2_sp;

int  ext2_readbmap(int dev, unsigned int block_group, unsigned int nr)
{
	buffer_t * buff;
	struct ext2_group * group;
    unsigned char* bitmap_buf;
    unsigned long group_desc;
	unsigned long desc;

	group_desc = block_group / sp_info.s_desc_per_block; ////��������ڿ�
	desc = block_group % sp_info.s_desc_per_block;  ////�����λ��

	#if IN_DISK       //////////////read information from disk
	buff = bread(get_dev_desc(dev), 2+group_desc);
	group = (struct ext2_group*)buff->b_data;
	#else      //////////////read information from memory
		group = (struct ext2_group *)sp_info.group_desc[group_desc];
		if (!group){
		printk ("Group Descriptor not loaded\n");
		return -1;
		}
	#endif

	#if IN_DISK
	memset(buff->b_data,0,1024);  //////////reset buffer!
	#endif

	buff = bread (get_dev_desc(dev), group[desc].bg_block_bitmap);
	if (!(buff)){
	printk (" Unable to read disk \n");
	return -1;
	}
	sp_info.loaded_bmaps = nr;
	bitmap_buf = buff->b_data;
	//buf_release(buff);	
	return 0;
}

static void ext2_loadimap(unsigned int block_group)
{
}

int ext2_allocblock(int dev)//
{
	unsigned long block_group;
	int i;
	unsigned long group_desc, desc;
	buffer_t * buff;
	struct ext2_group* group;
	//if (ext2_sp->s_free_blocks_count <= ext2_sp->s_r_blocks_count ) return 0;
	 
	 ///32���������������һ������
	desc = 0;
	group_desc = 0;
	for (i = 0; i < sp_info.group_nr; i++){

		buff = bread(get_dev_desc(dev), 2+group_desc);
		group = (struct ext2_group*)buff->b_data;

		if (group[desc].bg_free_blocks_count > 0) /////����п��п�
			break;   ///stop loop

		desc ++;    /////ָ����һ�������
		if (desc == sp_info.s_desc_per_block) { ////�����Ѿ�����32��
			group_desc ++;  /////ָ����һ����
			desc = 0;   /////�������Ϊ��0��
		}
	}

	if (group_desc > sp_info.group_nr){
		 printk("a bad group desc\n");
		 return 0;
	 }

	block_group = i;

	printk ("find new block: using block group %d(desc:%d,free block count:%d)\n", 
	block_group, desc, group[desc].bg_free_blocks_count);////�¿�λ��

	ext2_loadimap(block_group);
	return 1;
}