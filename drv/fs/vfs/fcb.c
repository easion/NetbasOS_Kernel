
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
#include <drv/errno.h>
#include <drv/log.h>
#include "sys.h"

static unsigned buffer_sem_lock;

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

file_t *fdopen(int fd)
{
	fs_task_t *fp = current_filp();

	if(fd>=0&&fd<NR_FILES)
		return (fp->fp[fd]);

	return NULL;
}

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

void fd_reset(int proc_nr)
{
	int i;
	fs_task_t *task;

	if ((proc_nr)>MAX_PROC|| proc_nr < 1)return;

	LOCK_SCHED(buffer_sem_lock);

	task=&proc[proc_nr];

	for (i=0; i<NR_FILES; i++)
	{
		task->fp_cloexec =(u32_t)-1;
		//if(task->fp[i])
		//	task->fp[i]->f_count--;
	}	
	UNLOCK_SCHED(buffer_sem_lock);
}

int device_open(char *filename,  int mode)
{
	dev_prvi_t *dev;
	int fd;
	char *base=basename(filename);

	dev = kmalloc(sizeof(dev_prvi_t),0);

	if (!dev){
		return -1;
	}


	if (!*base || IS_DEV_DIR(filename))
	{
	//printf("dir base = %s ---------\n",base);
		dev_dir_open(filename,dev,0);
		fd=0;
	}
	else{
		fd = dev_open(filename, mode,dev);

	}
	
	if (fd<0){
		kfree(dev);
		return -1;
	}

	fd = create_file_with_data(dev,2);
	dev->fd=fd;

	return fd;
}

int proc_open(char *filename,  int mode)
{
	dev_prvi_t *dev;
	int fd;

	dev = kmalloc(sizeof(dev_prvi_t),0);

	if (!dev){
		return -1;
	}

	//printf("proc_open %s\n",filename);
	
	fd = proc_file_open(filename, mode,dev);

	
	if (fd<0){
		kfree(dev);
		return -1;
	}

	fd = create_file_with_data(dev,3);
	dev->fd=fd;

	return fd;
}


int file_open(unsigned char* file_name, int mode, int flags)
{
	int i;
	int result;
	file_t* fp;
	inode_t *node;
	fs_task_t *current = current_filp();

	for (i=0; i<NR_FILES; i++){
		if(!current->fp[i])break;
	}

	if(i==NR_FILES){
		printk("file_open(): current task no file space!\n");
		return -EINVAL;
	}

	fp=get_file_filp();
	if(fp){
		//init_fiob(fp,sizeof(dev_prvi_t),"FIL");
		current->fp[i]=fp;
		fd_setup_close_exec(current,i);
	}
	else
		return -1;


	result = open_namei(file_name, flags,  &node) ;

	if(result != OK){
		current->fp[i]=NULL;
	   fp->f_inode=NULL;
	   sys_close(i);
	   //printf("file_open: %s not found\n",file_name);
		return -ENOENT;
	}

	fp->f_pos = 0;
	fp->f_mode = mode;
	fp->f_inode = node; //iget(node->i_dev, node->i_number);



	if (flags & O_TRUNC){
		//printf("O_TRUNC open %s mode = %x, size=%d\n",file_name,mode,node->i_size);
		//	truncate (fp->f_inode);
	}

	vfs_openfile(fp);
	//printf("file_open() fd%d,  file %s, ino %d\n",i, file_name, fp->f_inode->i_number);

	 return i;
}

int sys_open(unsigned char* file_name,  int flags, int mode )
{
	int ret;

	if(file_name[0] == 0)
		return -1;

	//printf("file_name = %s\n",file_name);

	if(IS_DEV_DIR(file_name) || IS_DEV_FILE(file_name)){
		 ret = device_open(file_name,  mode);
	}
	else if(IS_PROC_DIR(file_name) || IS_PROC_FILE(file_name)){		
		 ret = proc_open(file_name,  mode);
	}
	else{
		if(flags & O_CREAT){
			ret = sys_create(file_name, mode);
			//printf("create file %s %d\n",file_name,ret);
			ret = file_open(file_name, flags, mode);
		}
		else
			ret = file_open(file_name, flags, mode);
	}

	if (ret<0)
	{
	printf("sys_open: error open %s  flag %x - mode %x - ret%d\n", file_name, flags, mode, ret);
	}
	else{
	//printf("sys_open: succ open %s  flag %x - mode %x - ret%d\n", file_name, flags, mode, ret);
	}

	return ret;
}

int do_close(int fd, int freeall)
{
	int err=-1;
	file_t* fp;
	fs_task_t *task_space = current_filp();

	//kprintf("do_close = %d\n", fd);


	if (fd>=NR_FILES || fd<0){
		return -EINVAL;
	}

	LOCK_SCHED(buffer_sem_lock);

	fp=task_space->fp[fd];
	if (!fp){
		UNLOCK_SCHED(buffer_sem_lock);
		return -1;
	}

	if (fp->f_count>0)
	{
	fp->f_count--;
	}

	task_space->fp[fd] = NULL;
	fp->f_pos = 0;
	UNLOCK_SCHED(buffer_sem_lock);

	if (!freeall)
	{
		//printf("notfree all %d\n", fd);
		return 0;
	}

	if (S_ISSOCK(fp->f_inode->i_mode))
	{
	//printf("do_close  S_ISSOCK %d, task %d,%d\n", fd,current_thread_id(NULL),fp->f_inode->i_count);
	}


	vfs_release(fp);

	//if(fp->f_count==0&&fp->f_inode)
	if(fp->f_inode)
		err=iput(fp->f_inode);

	if (err)
	{
	vfs_release_buttom(fp);
	}


	
	return OK;
}


int sys_close(int fd)
{
	return do_close(fd,1);
}

