

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
#include <stdarg.h>
#include <drv/buffer.h>
#include <drv/fs.h>
#include <drv/unistd.h>
#include <drv/sym.h>
#include <drv/errno.h>
#include <assert.h>
#include "sys.h"


/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int do_access (const char *filename, int mode);
int  sys_stat(const char * pathname, struct  stat * stat);
int  sys_fstat(int fd, struct  stat * stat);
int  sys_stat64(const char * pathname, stat64_t * stat);
int sys_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg);
int fs_root_dev(char **ret, long* mode);

int remove_socket_filp(int fd);
void* fs_get_priv_data(int fd,u32_t*t);
int create_file_with_data(void *prvi_data, int devtype);
int sys_readlink(const char * path, char * buff, size_t buf_size);

static const fsapi_ops_t default_fs=
{
	fsname:"file system ver 0.20",
	author:"easion",
	copyright:"copyright 2005-2007 the jicama os project",
	mkdir:do_mkdir,
	rmdir:do_rmdir,
	unlink:do_unlink,
	link:	do_link,
	readlink:sys_readlink,
	write:sys_write,
	read:sys_read,
	getpwd:do_get_pwd,
	lseek:sys_lseek,
	rename:do_rename,
	create:sys_create,
	open:sys_open,
	close:sys_close,
	mount:sys_mount,
	unmount:sys_unmount,
	chdir:sys_chdir,
	dup:sys_dup,
	dup2:sys_dup2,
	sendfsinfo:clone_fd,
	readdir:sys_readdir,
	pipe:sys_pipe,
	chroot:sys_chroot,
	access:do_access,
	fcntl:sys_fcntl,
	stat:sys_stat,
	fstat:sys_fstat,
	stat64:sys_stat64,
	create_fd:	create_file_with_data,
	get_socket_data:	fs_get_priv_data,
	remove_socket:	remove_socket_filp,
};
inode_t * ifind(const int dev, const int blk);

static struct _export_table_entry fs_sym_table []=
{
	EXPORT_PC_SYMBOL(sys_mount),
	EXPORT_PC_SYMBOL(fs_root_dev),
	EXPORT_PC_SYMBOL(install_fs),
	EXPORT_PC_SYMBOL(deinstall_fs),
	EXPORT_PC_SYMBOL(try_probe_fs),
	EXPORT_PC_SYMBOL(get_superblock),

	//EXPORT_PC_SYMBOL(get_ops),
	EXPORT_PC_SYMBOL(vfs_bmap),
	EXPORT_PC_SYMBOL(sys_open),
	EXPORT_PC_SYMBOL(sys_lseek),
	EXPORT_PC_SYMBOL(sys_read),
	EXPORT_PC_SYMBOL(sys_write),
	EXPORT_PC_SYMBOL(sys_close),


	EXPORT_PC_SYMBOL(vfs_opendir),
	EXPORT_PC_SYMBOL(vfs_read),
	EXPORT_PC_SYMBOL(vfs_write),
	EXPORT_PC_SYMBOL(vfs_readdir),
	EXPORT_PC_SYMBOL(dev_block_size),
	//EXPORT_PC_SYMBOL(vfs_probe),
	EXPORT_PC_SYMBOL(vfs_write_inode),


	EXPORT_PC_SYMBOL(buf_release),
	EXPORT_PC_SYMBOL(bread),
	//EXPORT_PC_SYMBOL(buf_try_remove),
	EXPORT_PC_SYMBOL(flush_all),
	EXPORT_PC_SYMBOL(bread_with_size),
	EXPORT_PC_SYMBOL(sync_blks),
	EXPORT_PC_SYMBOL(mark_buf_dirty),
	EXPORT_PC_SYMBOL(buf_getblk),
	EXPORT_PC_SYMBOL(buf_find),

	EXPORT_PC_SYMBOL(iget),
	EXPORT_PC_SYMBOL(ifind),
	EXPORT_PC_SYMBOL(iput),
	EXPORT_PC_SYMBOL(sync_inodes),
	EXPORT_PC_SYMBOL(current_filp),
	EXPORT_PC_SYMBOL(get_dev_filp),
};
static unsigned proc_sem_lock;

static void install_fs_sym()
{
	int num =sizeof(fs_sym_table)/sizeof(struct _export_table_entry);
	return install_dll_table("fs.dll", 1, num, fs_sym_table);
}

enum {
	FS_FORK=1,
	FS_EXEC,
	FS_EXIT
};

int fd_close_exec_set(fs_task_t *dest, int fd)
{
	return (dest->fp_cloexec & (FD_CLOEXEC<<fd));
}

//置位 的描叙子在exec的时候会被close
void fd_setup_close_exec(fs_task_t *dest, int fd)
{
	LOCK_SCHED(proc_sem_lock);
	dest->fp_cloexec |= (FD_CLOEXEC<<fd);
	UNLOCK_SCHED(proc_sem_lock);
}

void fd_clear_close_exec(fs_task_t *dest, int fd)
{
	LOCK_SCHED(proc_sem_lock);
	dest->fp_cloexec &= ~(FD_CLOEXEC<<fd);
	UNLOCK_SCHED(proc_sem_lock);
}

off_t clone_fd(int srcfd, off_t dstfd, int arg)
{
	int i;

	LOCK_SCHED(proc_sem_lock);
	
	fs_task_t *dest  = &proc[dstfd];
	fs_task_t *src = &proc[srcfd];

	UNLOCK_SCHED(proc_sem_lock);
	

	
	switch (arg)
	{
	case FS_FORK: /*fork*/
	assert(current_taskno() == srcfd);

		*dest = *src;

		dup_inode(dest->pwd);
		dup_inode(dest->root);

		//dest->fp_cloexec =dest->fp_cloexec;
		dest->fp_cloexec =(u32_t)-1;

		for (i=0; i<NR_FILES; i++)
		{
			//完全继承父进程
			dest->fp[i] = src->fp[i] ;
			if(dest->fp[i]){	
				//printf("FS_FORK add fd %d\n",i);
				dest->fp[i]->f_count++;
				if (dest->fp[i]->f_inode)
				{
					dest->fp[i]->f_inode->i_count++;
				}
			}
		}		
		
		break;

	case FS_EXEC: /*exec*/
		{
			//printf("FS_EXEC dest->fp_cloexec=%x\n",dest->fp_cloexec);
	for(i = 0; i < NR_FILES; i++)
		if(!dest->fp[i])
		continue;

		if (fd_close_exec_set(dest,i)){
			//printf("FS_EXEC close %d\n",i);
			sys_close(i);
		}
		else{
			printf("fd %d with FD_CLOEXEC attr\n", i);
		}
		break;
		}

	case FS_EXIT: /*exit*/

	//printf("FS_EXIT called\n");
	iput(dest->pwd);
	iput(dest->root);
		for (i=0; i<NR_FILES; i++)
		{
			if(dest->fp[i]){
				sys_close(i);				
			}
		}		
		break;	
	default:
		break;
	}

	return 0;
}



void init_process()
{
	int i;
	fs_task_t *dest=current_filp() ;

	install_fs_sym();

	//printf("init_process is current_taskno %d\n", current_taskno());
	
	for (i=0; i<NR_FPOOLS; i++)
	{
		memset(&FilePool[i], 0, sizeof(file_t));
	}
	
	for (i=0; i<MAX_PROC; i++){
		memset(&proc[i],0,sizeof(fs_task_t));
	}

	/*current inode*/
	
	for (i=0; i<NR_FILES; i++)
		{
			//完全继承父进程
			dest->fp[i] = NULL ;			
		}	
	sys_fs_setup();
}

static void sys_fs_setup()
{
	register_fs(&default_fs);
	select_fs(default_fs.fsname);
}

/*get file*/
void free_fpool(file_t *fp)
{
	assert(fp != NULL);
	fp->f_count--;
}

/*get file*/
file_t *get_empty_filp(void)
{
	int i;

	LOCK_SCHED(proc_sem_lock);
	for (i=0; i<NR_FPOOLS; i++)
	{
		if (FilePool[i].f_count == 0)
		{
			break;
		}
	}
	UNLOCK_SCHED(proc_sem_lock);
	if (i==NR_FPOOLS)
	{
		return (file_t *)0;
	}

	memset(&FilePool[i], 0, sizeof(file_t));
	FilePool[i].f_count++;
	
	thread_waitq_init(&FilePool[i].f_wait);
	return &FilePool[i];
}

file_t *get_file_filp(void)
{
	file_t *fp;

	fp = get_empty_filp();
	
	return fp;
}

file_t  FilePool[NR_FPOOLS];
