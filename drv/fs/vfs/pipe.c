/*
**     (R)Jicama OS
**     File IO Operating
**     Copyright (C) 2003 DengPingPing
*/

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
#include <signal.h>
#include <assert.h>
#include "sys.h"

#define PIPE_DATA(inode) ((inode)->pipe->data)
#define PIPE_HEAD(inode) ((inode)->pipe->pipe_head)
#define PIPE_TAIL(inode) ((inode)->pipe->pipe_tail)
#define PIPE_SIZE(inode) ((PIPE_HEAD(inode)-PIPE_TAIL(inode))& PAGE_MASK)
#define PIPE_EMPTY(inode) (PIPE_HEAD(inode)==PIPE_TAIL(inode))
#define PIPE_FULL(inode) (PIPE_SIZE(inode)==(PAGE_MASK))


#define INC_PIPE(head) do{\
	(head)++;\
	head &= PAGE_MASK;\
	}\
	while (0)

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int read_pipe(file_t * filp, char * buf, int count)
{
	inode_t * inode=filp->f_inode;
	char *b=buf;
	char *p;
	pipe_prvi_t *prvi;
	int ret=0;


	assert(filp!=NULL);
	prvi = (char*)filp->f_iob;
	if(!prvi)
		return -1;	

	assert(prvi->peer);
	p=PIPE_DATA(prvi);
	//printf("read pipe %d, %d,%d\n",count,PIPE_HEAD(prvi), PIPE_TAIL(prvi));
	
	do  {
		int pos;


		while (PIPE_EMPTY(prvi))
		{

			if (filp->f_mode&O_NONBLOCK)
			{
				if (!ret){
					printf("pipe : retry ...\n");
					return -EINTR;
				}
				goto done;
			}

			if (inode->i_count<2)
			{
				//kprintf("read_pipe() break %d\n",inode->i_count);
				return ret;
			}

			//kprintf("read_pipe() sleep %d ====\n",PIPE_HEAD(prvi));
			//thread_wakeup(&prvi->peer->f_wait);
			thread_wakeup(&prvi->peer->f_wait);
			thread_sleep_on(&filp->f_wait);
		}

		pos=PIPE_TAIL(prvi);

		//printf("%c",p[pos]);

		*(b++) = p[pos];
		INC_PIPE( PIPE_TAIL(prvi) );
		count --;
		ret++;
	}
	while(count>0);

done:
	if(set_io_event(prvi->peer->f_iob, IOEVT_WRITE)==0){
		thread_wakeup(&prvi->peer->f_wait);
	//printf("read_pipe called %s\n",buf);
	}

	if(PIPE_EMPTY(prvi))
		clear_io_event(prvi, IOEVT_READ);
	
	return ret;
}
	

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int write_pipe(file_t * filp, char * buf, int count)
{
	char * b=buf;
	int pos = 0;
	inode_t * inode=filp->f_inode;
	char *p;
	pipe_prvi_t *prvi;
	long flags = filp->f_mode;

	assert(filp!=NULL);
	prvi = (char*)filp->f_iob;

	//printf("write_pipe %s, %d\n",buf,count);
	if(!prvi)
		return -1;

	if (inode->i_count<2){
		//printf("pipewrite: just me ??\n");
		//return -1;
	}

	assert(prvi->peer);

	p=PIPE_DATA(prvi);

	while (count>0) {



		while (PIPE_FULL(prvi)) {
			if (inode->i_count<2)
			{
				//kprintf("write_pipe() break%d \n",inode->i_count);
				return (int)(b-buf);
			}

			//kprintf("write_pipe sleep %d*****\n",PIPE_HEAD(prvi));
			thread_wakeup(&prvi->peer->f_wait);			
			thread_sleep_on(&filp->f_wait);
		}


		//kprintf("%c",*(b));

		pos = PIPE_HEAD(prvi);
		((char *)p)[pos] = *(b++);
		//printf("[%c]",((char *)p)[pos]);
		INC_PIPE( PIPE_HEAD(prvi) );
		count--;
	}

	if(set_io_event(prvi->peer->f_iob, IOEVT_READ)==0){
		thread_wakeup(&prvi->peer->f_wait);

	//printf("write_pipe called %s\n",buf);
	}

	if(PIPE_FULL(prvi))
		clear_io_event(prvi, IOEVT_WRITE);

	return (int)(b-buf);
}

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int close_pipe(file_t * filp)
{
	pipe_prvi_t *self_prvi;
	inode_t * inode=filp->f_inode;

	if (!inode)
	{
		printf("close_pipe error\n");
		return -1;
	}

	self_prvi = filp->f_iob;
	//printf("close_pipe done %d\n", inode->i_count);

	if (inode->i_count<1)
	{
		//free_fiob(peer);
		//iput(inode);		
	}
	else{
		//printf("close_pipe wakeup %d\n", inode->i_count);
		//»½ÐÑ¶Ô¶Ë

	thread_wakeup(&self_prvi->peer->f_wait);
		return 1;
	}

	free_fiob(filp);
	return 0;
}

int sys_pipe(long * fildes)
{
	int i;
	file_t* fp[2];
	pipe_prvi_t *prvi;
	pipe_prvi_t *prvi2;
	fs_task_t *task_space = current_filp();

	assert(fildes!=NULL);

	for (i=0; i<NR_FILES; i++)
	{
		if(task_space->fp[i] == NULL 
			&& task_space->fp[i+1]==NULL)
		break;
	}

	if(i==NR_FILES)
	{
		syslog(4,"pipe(): task_space task no file space!\n");
		return -EINVAL;
	}
	
	fp[0] = get_empty_filp();
	init_fiob(fp[0],sizeof(pipe_prvi_t),"PIP");

	fp[1]=get_empty_filp();
	init_fiob(fp[1],sizeof(pipe_prvi_t),"PIP");

	struct pipe *pipe = kmalloc(sizeof(struct pipe),0);

	prvi2 = fp[1]->f_iob;
	prvi2->peer = fp[0];
	prvi2->pipe = pipe;


	prvi = fp[0]->f_iob;
	prvi->peer = fp[1];
	prvi->pipe = pipe;


	if(fp[0]&&fp[1]){
		task_space->fp[i]=fp[0];
		task_space->fp[i+1]=fp[1];
	}
	else{
		exit:
		if(fp[0])free_fpool(fp[0]);
		if(fp[1])free_fpool(fp[1]);
		kprintf("free_fpool");
		return -1;
	}

    fp[0]->f_pos = fp[1]->f_pos = 0;
	fp[0]->f_mode = O_RDONLY;
	fp[1]->f_mode = O_WRONLY;
	
	fp[1]->f_inode = fp[0]->f_inode = create_pipe_node(2);
	if (!fp[0]->f_inode)
	{
		goto exit;
	}

	//PIPE_HEAD(fp[0]->f_inode) = PIPE_TAIL(fp[0]->f_inode) = 0;

	fd_clear_close_exec(task_space,i);
	fd_clear_close_exec(task_space,i+1);

	fildes[0] = i;
	fildes[1] = i+1;
	return 0;
}



