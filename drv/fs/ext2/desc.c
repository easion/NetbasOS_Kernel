
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------
#include "ext2.h"

extern struct ext2_info sp_info;

void ext2_load_gdesc(int dev)
{
	int i, desc_block = 0;
	buffer_t* buff;
	
	cli();
	buff = NULL_BUF;
	desc_block = (sp_info.group_nr + sp_info.s_desc_per_block) /
		   sp_info.s_desc_per_block;

    #if DEBUG
	printk("group descriptor count : %i, the will read %i block\n",
		                                 sp_info.group_nr, desc_block);
	#endif

    for (i =0; i<desc_block; i++)
    {
		//must rewrite it 
		//sp_info.group_desc[i] = malloc (1024);
		buff = bread(get_dev_desc(dev), 2+i);

		memcpy((void*)&sp_info.group_desc[i], buff->b_data, 1024);
		if (!sp_info.group_desc[i])
			printk("can't loading group descriptive\n");
		return;
	}   
        buf_release(buff);
		sti();
}

