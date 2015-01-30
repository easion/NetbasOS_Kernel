#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <drv/errno.h>
#include <assert.h>
#include "sys.h"


inode_t * create_socket_node(long);
static unsigned buffer_sem_lock;

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

file_t* get_dev_filp(dev_prvi_t *dev)
{
	int fd = dev->fd;
	file_t* fp;
	fs_task_t *task_space = current_filp();


	if (fd>=NR_FILES || fd<0){
		return NULL;
	}

	fp=task_space->fp[fd];
	if (!fp){
		return NULL;
	}

	return fp;
}


/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int create_file_with_data(void *prvi_data, int devtype)
{
	int i;
	file_t* fp;
	fs_task_t *task_space = current_filp();
	dev_prvi_t *dev=prvi_data;

	assert(prvi_data!=NULL);

	LOCK_SCHED(buffer_sem_lock);

	for (i=0; i<NR_FILES; i++)
	{
		if(task_space->fp[i] == NULL )
		break;
	}
	UNLOCK_SCHED(buffer_sem_lock);

	if(i==NR_FILES)
	{
		syslog(4,"create_file_with_data(): current_filp() task no file space!\n");
		return -EINVAL;
	}
	
	fp=get_empty_filp();
	if (!fp)
	{
		return -1;
	}
	fp->f_iob = prvi_data;
	//init_fiob(fp,sizeof(struct ioobject),"NET");

	if(fp){
		LOCK_SCHED(buffer_sem_lock);
		task_space->fp[i]=fp;
		UNLOCK_SCHED(buffer_sem_lock);
		fd_setup_close_exec(task_space,i);
	}
	else{
		exit:
		if(fp)free_fpool(fp);
		kprintf("free_fpool\n");
		return -1;
	}

    fp->f_pos= 0;

	if(devtype==1){
		fp->f_inode  = create_socket_node(S_IFSOCK);
	}
	else if(devtype==2){
		init_ioobject(fp->f_iob,"DEV");
		fp->f_inode  = create_socket_node(S_IFBLK);
	}
	else if(devtype==3){
		init_ioobject(fp->f_iob,"PRO");
		fp->f_inode  = create_socket_node(S_IFWHT);
	}
	else{
		//printf("create_file_with_data %d\n",devtype);
	}

	if (!fp->f_inode)
	{
		goto exit;
	}

	

	return i;
}

void* fs_get_priv_data(int fd,u32_t *t)
{
	file_t* fp;
	inode_t *inode;
	fs_task_t *task_space = current_filp();

	if (fd<0||fd>NR_FILES)
	{
		return NULL;
	}
	LOCK_SCHED(buffer_sem_lock);

	fp = task_space->fp[fd];
	if (!fp)
	{
		UNLOCK_SCHED(buffer_sem_lock);
		return NULL;
	}
	inode = fp->f_inode;

	if (!(S_ISBLK(inode->i_mode) || S_ISCHR(inode->i_mode)))
	{
		//printf("fs_get_priv_data fd=%d type %d\n", fd,inode->i_mode);
	}
	if(t)*t=inode->i_mode;
	UNLOCK_SCHED(buffer_sem_lock);
	return fp->f_iob;
}

int remove_socket_filp(int fd)
{
	return 0;
}


