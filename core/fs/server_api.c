
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------

#include <errno.h>
#include <jicama/process.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>



static fsapi_ops_t *cur_fs_ops=(fsapi_ops_t *)0;
static fsapi_ops_t *mui_fs_ops[4];
/*
**
*/
int vfs_open(const char * filename, int flag,  int mode)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->open){
		res= -1;
	}
	else
		res=cur_fs_ops->open(filename, flag, mode);

	return res;
}
/*
**
*/
int mkdir(const char *f, mode_t mode)
{
	register int res;

	//kprintf("mkdir for %s\n",f);

	if (!cur_fs_ops || !cur_fs_ops->mkdir){
		kprintf("mkdir not register\n");
		res= -1;
	}
	else
		res=cur_fs_ops->mkdir(f,  mode);

	return res;
}

/*
**
*/
int  vfs_stat64(const char * pathname, stat64_t * stat)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->stat64){
		kprintf("stat64 not register\n");
		res= -1;
	}
	else
		res=cur_fs_ops->stat64(pathname,  stat);

	return res;
}

/*
**
*/
int  fs_stat(const char * pathname, stat_t * stat)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->stat){
		kprintf("stat not register\n");
		res= -1;
	}
	else
		res=cur_fs_ops->stat(pathname,  stat);

	return res;
}

/*
**
*/
int  vfs_fstat(int fd, stat_t * stat)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops || !cur_fs_ops->fstat){
		kprintf("fstat not register\n");
		res= -1;
	}
	else
		res=cur_fs_ops->fstat(fd,  stat);

	return res;
}

/*
**
*/
int  vfs_access(const char * pathname, int mode)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->access){
		kprintf("access not register\n");
		res= -1;
	}
	else
		res=cur_fs_ops->access(pathname,  mode);

	return res;
}

/*
**
*/
int  vfs_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->fcntl){
		kprintf("fcntl not register\n");
		res= -1;
	}
	else
		res=cur_fs_ops->fcntl(fd,  cmd, arg);

	return res;
}


/*
**
*/
int chdir(const char *f)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->chdir)
		res= -1;
	else
		res=cur_fs_ops->chdir(f);

	return res;
}

/*
**
*/
int rmdir(const char *f)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->rmdir){
		kprintf("rmdir not register\n");
		res= -1;
	}
	else
		res=cur_fs_ops->rmdir(f);

	return res;
}

/*
**
*/
int chroot(const char *f)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->chroot)
		res= -1;
	else
		res=cur_fs_ops->chroot(f);

	return res;
}

/*
**
*/
int vfs_mount(const char *devname, char *f2, const char *fstype, long mode)
{
	register int res;
	//dev_t d=0x200;

	if (!cur_fs_ops || !cur_fs_ops->mount)
		res= -1;
	else
		res=cur_fs_ops->mount(devname, f2,fstype,mode);

	return res;
}


/*
**
*/
int vfs_unmount(const char *devname)
{
	register int res;
	//dev_t d=0x200;

	if (!cur_fs_ops || !cur_fs_ops->unmount)
		res= -1;
	else
		res=cur_fs_ops->unmount(devname);

	return res;
}
/*
**
*/
int dup(int fd)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->dup) 
		res= -1;
	else
		res=cur_fs_ops->dup(fd);

	return res;
}

/*
**
*/
int pipe(long *fds)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->pipe) 
		res= -1;
	else
		res=cur_fs_ops->pipe(fds);

	return res;
}

/*
**
*/
int dup2(int fd, int fd_new)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->dup) 
		res= -1;
	else
		res=cur_fs_ops->dup2(fd, fd_new);

	return res;
}

/*
**
*/
int rename(const char *n1, const char *n2)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->rename) 
		res= -1;
	else
		res=cur_fs_ops->rename(n1,n2);

	return res;
}

/*
**
*/
int getpwd(char *n1, int len)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->getpwd) 
		res= -1;
	else
		res=cur_fs_ops->getpwd(n1,len);

	return res;
}

/*
**
*/
int vfs_close(int fd)
 {
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->close)
		res= -1;
	else
		res=cur_fs_ops->close(fd);
	return res;
}

/*
**
*/
size_t	 vfs_read(int fd, const void *buf, size_t s, int arg)
{
	register int res;
	
	if (!cur_fs_ops || !cur_fs_ops->read)
		res= -1;
	else
		res=cur_fs_ops->read(fd, buf, s,arg);

	return res;
}

/*
**
*/
int	 vfs_readdir(int fd, const void *buf, size_t s)
{
	register int res;

	//kprintf("readdir called\n");

	if (!cur_fs_ops || !cur_fs_ops->read)
		res= -1;
	else
		res=cur_fs_ops->readdir(fd, buf, s);

	return res;
}

/*
**
*/
int	 vfs_readlink(const char* path, const void *buf, size_t s)
{
	register int res;

	//kprintf("readdir called\n");

	if (!cur_fs_ops || !cur_fs_ops->readlink)
		res= -1;
	else
		res=cur_fs_ops->readlink(path, buf, s);

	return res;
}


/*
**
*/
size_t	 vfs_write(int fd, const void *buf, size_t s, int arg)
{
	int i;
	register int res;
	unsigned char *ptr=(unsigned char *)buf;
	
	if (!cur_fs_ops || !cur_fs_ops->write)
		res= -1;
	else
		res=cur_fs_ops->write(fd, buf, s,  arg);

	return res;
	}

/*
**
*/
int vfs_creat(const char *f, u16_t permiss)
 {
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->create)
		res= -1;
	else
		res=cur_fs_ops->create(f, permiss);

	return res;
 }

int vfs_unlink(const char *f)
 {	
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->unlink){
		kprintf("unlink not register\n");
		res= -1;
	}
	else
		res=cur_fs_ops->unlink(f);

	return res;
 }

int vfs_link(const char *f,const char *fnew)
 {	
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->link){
		kprintf("link not register\n");
		res= -1;
	}
	else
		res=cur_fs_ops->link(f,fnew);

	return res;
 }

/*
**
*/
off_t		lseek(int _fildes, off_t _offset, int _whence)
 {	
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->lseek)
		res= -1;
	else
		res=cur_fs_ops->lseek(_fildes, _offset, _whence);

	return res;
 }

 /*
**
*/
int fs_create_file_filp(void *socketdata,int issock)
 {	
	int res;

	if (!cur_fs_ops->create_fd)
		res= -1;
	else
		res=cur_fs_ops->create_fd(socketdata,issock);

	return res;
 }

void* fs_get_prvi_data(int fd,u32_t *type)
{
	void* res;

	if (!cur_fs_ops || !cur_fs_ops || !cur_fs_ops->get_socket_data)
		res= NULL;
	else
		res=cur_fs_ops->get_socket_data(fd,type);

	return res;
}

int remove_socket_filp(int fd)
{
	int res;

	if (!cur_fs_ops || !cur_fs_ops || !cur_fs_ops->remove_socket)
		res= -1;
	else
		res=cur_fs_ops->remove_socket(fd);

	return res;
}

/*
**
*/
 off_t sendfsinfo(int _fildes, off_t _fildes2, int arg)
{
	register int res;

	if (!cur_fs_ops || !cur_fs_ops->sendfsinfo)
		res= -1;
	else
		res=cur_fs_ops->sendfsinfo(_fildes, _fildes2, arg);

	return res;
};

/*
**
*/
int register_fs( fsapi_ops_t *ops)
{
	int i;

	if(!ops)return -1;
	
	for (i=0; i<4; i++)
	{
		if (!mui_fs_ops[i])
			break;
	}

	if (i==4)return -1;

	mui_fs_ops[i]=ops;
	return 0;
}

/*
**
*/
int unregister_fs(char *name)
{
	int i;

	if(!name)return -1;
	
	for (i=0; i<4; i++)
	{
		if (!mui_fs_ops[i])
			continue;
		if (!strcmp(mui_fs_ops[i]->fsname, name))
			break;
	}

	if (i==4)return -1;	

	mui_fs_ops[i]=NULL;
	return 0;
}

/*
**
*/
int select_fs(char *name)
{
	int i;

	if(!name)return -1;
	
	for (i=0; i<4; i++)
	{
		if (!mui_fs_ops[i])
			continue;
		if (!strcmp(mui_fs_ops[i]->fsname, name))
			break;
	}

	if (i==4)return -1;

	cur_fs_ops=mui_fs_ops[i];
	return 0;
}



