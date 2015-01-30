
/*
**     (R)Jicama OS
**     Path Name Conversion
**     Copyright (C) 2003 DengPingPing
*/


#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/errno.h>
#include <drv/fnctl.h>
#include <assert.h>
#include "sys.h"


typedef struct fd_set {
	unsigned long __bits [8];
} fd_set;
#define FD_ISSET(fd,fdsetp) ({ \
		char __result; \
		__asm__ __volatile__("btl %1,%2 ; setb %0" \
			:"=q" (__result) :"r" ((int) (fd)), \
			"m" (*(fd_set *) (fdsetp))); \
		__result; })


/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int sys_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
	int err=0;
	fs_task_t *task_space = current_filp();
	file_t *filp=task_space->fp[fd];


	if (fd >= NR_FILES || !(filp )){
		printf("sys_fcntl: error %d cmd%d\n",fd,cmd);
		return -EBADF;
	}

	switch (cmd)
	{
	case F_DUPFD:
		{
		int to = arg;
		err = sys_dup2(fd,to);
		break;
		}
	case F_GETFD:
		return  fd_close_exec_set(task_space,fd);;
	case F_GETFL:
		//printf("F_GETFL fp[fd]->f_mode= %x\n", task_space->fp[fd]->f_mode);
		return task_space->fp[fd]->f_mode;
	case F_SETFL:
			task_space->fp[fd]->f_mode &= ~(O_APPEND | O_NONBLOCK);
			//task_space->fp[fd]->f_mode |= arg;
			task_space->fp[fd]->f_mode |= arg & (O_APPEND | O_NONBLOCK);
		//printf("F_SETFL fp[fd]->f_mode= %x\n", task_space->fp[fd]->f_mode);
			return 0;

	case F_SETFD:
		if (arg&1)
				fd_setup_close_exec(task_space,fd);
			else
			fd_clear_close_exec(task_space,fd);
		return 0;
	//case F_SETLK:
	//	return 0;
	//	break;

	//case F_GETLK:
	//	return 0;


	default:
		printf("fcntl EPERM %d\n",cmd);
		return -EPERM;
	}
	//printf("fcntl ret %d\n",err);
	return err;

}

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

inline mode_t typemode(int t, mode_t m)
{
	return t | (m & ~S_IFMT);
}



int procfs_stat(const char * pathname, struct  stat * stat)
{
	if (IS_PROC_DIR(pathname) || IS_DEV_DIR(pathname))
	{	
	stat->st_mode = typemode(S_IFDIR, 0666);
	}
	else if (IS_DEV_FILE(pathname) || IS_PROC_FILE(pathname))
	{
	stat->st_mode = typemode(S_IFREG, 0666);
	}
	else{
		return -1;
	}

	stat->st_uid=0;
	stat->st_dev = 0x0;
	stat->st_rdev = 0x0;


	stat->st_size = 0;
	stat->st_ino = 0;
	stat->st_blksize = 0;
	stat->st_blocks = 0;
	stat->st_ctime = get_unix_time();
	stat->st_atime = get_unix_time();
	stat->st_mtime = get_unix_time();

	return 0;
}


int  sys_stat(const char * pathname, struct  stat * stat)
{
	int error;
	int res, i_mode;
	inode_t *inode;

	if (!procfs_stat(pathname,stat))
	{
		return 0;
	}

	error = open_namei(pathname, O_RDONLY,  &inode);
	if(error != OK){
		//kprintf("sys_stat() open_namei error for path %s\n", pathname);
		return -ENOENT;	
	}

	memset(stat,0,sizeof(*stat));
	

	stat->st_uid=inode->i_uid;
	stat->st_dev = inode->i_dev;
	stat->st_rdev = inode->i_dev;
	stat->st_mode =inode->i_mode;// typemode(inode->i_mode, 0777);	
	stat->st_size = inode->i_size;
	stat->st_ino = inode->i_number;
	stat->st_blksize = inode->i_super->m_blk_size;
	stat->st_blocks = 1+ inode->i_size/inode->i_super->m_blk_size;
	stat->st_ctime = inode->i_ctime;
	stat->st_atime = inode->i_atime;
	stat->st_mtime = inode->i_mtime;

	//printf("%s inode->i_mode = %x, %d %d\n", pathname,inode->i_mode, inode->i_size,inode->i_number);

	iput(inode);
	return 0;
}

int  sys_fstat(const int fd, struct  stat * stat)
{
	inode_t *inode;
	file_t *file;
	fs_dev_ops_t *op;
	fs_task_t *task_space = current_filp();

	if (fd>=NR_FILES || fd<0){
		return -EINVAL;//
	}

	file = task_space->fp[fd];

	if (file==NULL)
		return -1;

	inode = file->f_inode;
	op = get_inode_ops(inode);
	if (!op )
		return -1;


	stat->st_uid=inode->i_uid;
	stat->st_dev = inode->i_dev;
	stat->st_rdev = inode->i_dev;
	stat->st_mode =inode->i_mode;// typemode(inode->i_mode, 0777);
	//printf("mode=%o imode=%o\n", stat->mode, inode->i_mode);
	//stat->mode = typemode(S_IFREG,0111);
	stat->st_size = inode->i_size;
	stat->st_ino = inode->i_number;
	stat->st_blksize = inode->i_super->m_blk_size;
	stat->st_blocks = 1+ inode->i_size/inode->i_super->m_blk_size;
	stat->st_ctime = inode->i_ctime;
	stat->st_atime = 0;
	stat->st_mtime = 0;
	return 0;
}

int  sys_stat64(const char * pathname, stat64_t * stat)
{
	return 0;
}

char *getmonthname(int month)
  {
     switch (month)
       {
         case 1 : return "January";
         case 2 : return "Febuary";
         case 3 : return "March";
         case 4 : return "April";
         case 5 : return "May";
         case 6 : return "June";
         case 7 : return "July";
         case 8 : return "August";
         case 9 : return "September";
         case 10: return "October";
         case 11: return "November";
         case 12: return "Decemeber";
		 default:
               return NULL;
       };
    return NULL;
  };

