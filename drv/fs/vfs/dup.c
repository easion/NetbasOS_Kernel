/*
**     (R)Jicama OS
**     File IO Operating
**     Copyright (C) 2003 DengPingPing
*/

#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/errno.h>
#include <signal.h>
#include <assert.h>
#include "sys.h"
static unsigned buffer_sem_lock;

static inode_t * get_dup_node(int dev, int cnt);

static int dupfd(unsigned int to_fd, unsigned int base_fd)
{
	int from_fd=base_fd;
	fs_task_t *task_space = current_filp();

	if (from_fd >= NR_FILES || to_fd >= NR_FILES){
		printf("dupfd() error to_fd%d\n",to_fd);
		return -EINVAL;
	}

	LOCK_SCHED(buffer_sem_lock);

	while (from_fd < NR_FILES){
		if (task_space->fp[from_fd])
			from_fd++;
		else
			break;
	}

	if (from_fd >= NR_FILES){
		UNLOCK_SCHED(buffer_sem_lock);
		printf("dupfd() error alloc fd\n");
		return -EMFILE;
	}

	//确保在exec的时候不会被关闭
	fd_clear_close_exec(task_space,from_fd);
	task_space->fp[from_fd]=task_space->fp[to_fd];
	task_space->fp[from_fd]->f_count++;

	dup_inode(task_space->fp[from_fd]->f_inode);
	UNLOCK_SCHED(buffer_sem_lock);

	return from_fd;
}




int sys_dup2(unsigned int oldfd, unsigned int from_fd)
{
	int ret_fd;
	inode_t *inode;
	inode_t *to;
	dev_prvi_t *dp;
	dev_prvi_t *todp;
	fs_task_t *task_space = current_filp();

	LOCK_SCHED(buffer_sem_lock);

	inode =task_space->fp[oldfd]->f_inode;
	dp = task_space->fp[oldfd]->f_iob;	

	do_close(from_fd,0);

	ret_fd = dupfd(oldfd,from_fd);
	if (ret_fd<0)
	{
		UNLOCK_SCHED(buffer_sem_lock);
		printk("dup2(%d,%d)=%x\n", oldfd, from_fd,ret_fd );
		return -1;
	}

	todp = task_space->fp[ret_fd]->f_iob;
	UNLOCK_SCHED(buffer_sem_lock);
	//if (S_ISFIFO(inode->i_mode) && from_fd<3)
	//{
	//	printf("error devno\n");
		//todp->devno = 0X400;
	//}
	return ret_fd;
}

int sys_dup(unsigned int fildes)
{
	int ret_fd;

	ret_fd=dupfd(fildes,0);

	if (ret_fd<0)
	{
	printk("dup(%d) error=%x\n", fildes, ret_fd );
	}
	return ret_fd;
}

