/*
**     (R)Jicama OS
**     Ext2 FS Directory
**     Copyright (C) 2003 DengPingPing
*/

#include "ext2.h"

#define EXT2_DIR_BUF_SIZE     1024*12
static unsigned char Ext2_Dir_Buf[EXT2_DIR_BUF_SIZE];

int ext2_dir(inode_t *inode)
{
	  buffer_t* buff = NULL_BUF;
      struct ext2_entry2 *temp_dir = {0,};   //a ext2 directory
      int  j, node_no, nr = 0, i=0;
	  struct ext2_inode *node = inode->i_private_data;

	    for(j=0; j <node->i_blocks ; j++ )  ///reading data...
	    {	
		if(j > 11 ) break;

		buff = bread (inode->i_dev,  node->i_block[i]);     ////for data  block
		memcpy(&Ext2_Dir_Buf[i*1024], buff->b_data, 1024);    ///copy it,
	    }

	  while ( 1 ){ 
         unsigned char dirname[256];  ///Directory name
	     temp_dir = (struct ext2_entry2 *)&Ext2_Dir_Buf[nr]; 

	     if (temp_dir -> rec_len == 0) 
			 break;   ///if be zero, stop it

		for(j = 0; j<temp_dir->name_len; j++)
	     	dirname[j] = temp_dir->name[j];  //////copy file name
	        
			dirname[temp_dir->name_len] = '\0';  ////add EOF flag
		    node_no = temp_dir->inode;
 
	     if (temp_dir->file_type == 2)	
			 printk("   %s     ", dirname);  ////out dir name
	     else 
			 printk("    %s     ",dirname);  ///out name

		 if((++i)%5==0)printk("\n");
	     nr += temp_dir->rec_len;  ///read next
      }

       buf_release(buff);

		return SUCCESS;
}


inode_t* ext2_opendir(inode_t* prev, unsigned char* name)
{
	inode_t* src;

	if (!name)return NULL;

	src = ext2_file2node(prev,name);
	if (!src)return NULL;

	if (src->i_mode != EXT2_DIR)
	return NULL;

	src->i_mode |= I_REGULAR;
	return src;
}



