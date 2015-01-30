

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
#include <drv/buffer.h>

int file_open(unsigned char* file_name, int flags, int mode);

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int sys_create(u8_t *path_name, mode_t mode)
 {
	int error = 0;
	inode_t * inoptr=NULL;
	char  _bname[MAXPATH] ;
	char  _dirname[MAXPATH] ;
	int ret;

	if (!path_name)
	{
		return -1;
	}	

	printf("sys_create: %s %d\n",path_name,__LINE__);

	ret = basename_r(path_name, _bname, MAXPATH);
	if (ret<0)
	{
		return -1;
	}

	ret = dirname_r(path_name, _dirname, MAXPATH);
	if (ret<0)
	{
		return -1;
	}

	error = open_namei(_dirname, 0, &inoptr);
	if(error!=OK)
		return -ENOENT;


	error = vfs_create_file(inoptr,_bname,mode);
	iput(inoptr);
	return error;
 }

 int do_mkdir(unsigned char *path_name, int mode)
 {
	int error = 0;
	inode_t * inoptr=NULL;
	char  _bname[MAXPATH] ;
	char  _dirname[MAXPATH] ;
	int ret;

	if (!path_name)
	{
		return -1;
	}	

	ret = basename_r(path_name, _bname, MAXPATH);
	if (ret<0)
	{
		return -1;
	}

	ret = dirname_r(path_name, _dirname, MAXPATH);
	if (ret<0)
	{
		return -1;
	}

	error = open_namei(_dirname, 0, &inoptr);
	if(error!=OK)
		return -ENOENT;


	error = vfs_mkdir(inoptr,_bname,mode);
	iput(inoptr);
	return error;
 }

 int do_unlink(const char *path_name)
{
	int error = 0;
	inode_t * inoptr=NULL;
	char  _bname[MAXPATH] ;
	char  _dirname[MAXPATH] ;
	int ret;

	if (!path_name)
	{
		return -1;
	}	

	ret = basename_r(path_name, _bname, MAXPATH);
	if (ret<0)
	{
		return -1;
	}

	ret = dirname_r(path_name, _dirname, MAXPATH);
	if (ret<0)
	{
		return -1;
	}

	error = open_namei(_dirname, 0, &inoptr);
	if(error!=OK)
		return -ENOENT;


	error = vfs_unlink(inoptr,_bname);
	iput(inoptr);
	return error;
}

 int do_link(const char *path_name,const char *pathnew)
{
	int error = 0;
	inode_t * inoptr=NULL;
	char  _bname[MAXPATH] ;
	char  _bnew[MAXPATH] ;
	char  _dirname[MAXPATH] ;
	int ret;


	if (!path_name)
	{
		return -1;
	}	

	ret = basename_r(path_name, _bname, MAXPATH);
	if (ret<0)
	{
		return -1;
	}

	ret = basename_r(pathnew, _bnew, MAXPATH);
	if (ret<0)
	{
		return -1;
	}

	ret = dirname_r(path_name, _dirname, MAXPATH);
	if (ret<0)
	{
		return -1;
	}

	error = open_namei(_dirname, 0, &inoptr);
	if(error!=OK)
		return -ENOENT;


	error = vfs_link(inoptr,_bname,pathnew);
	iput(inoptr);
	return error;
}



int do_rename(unsigned char *path, char *path2)
{
	return -1;
}

 int do_rmdir(const char *path_name)
{
	int error = 0;
	inode_t * inoptr=NULL;
	char  _bname[MAXPATH] ;
	char  _dirname[MAXPATH] ;
	int ret;

	if (!path_name)
	{
		return -1;
	}	

	ret = basename_r(path_name, _bname, MAXPATH);
	if (ret<0)
	{
		return -1;
	}

	ret = dirname_r(path_name, _dirname, MAXPATH);
	if (ret<0)
	{
		return -1;
	}

	error = open_namei(_dirname, 0, &inoptr);
	if(error!=OK)
		return -ENOENT;


	error = vfs_rmdir(inoptr,_bname);
	iput(inoptr);
	return error;
}

int sys_readlink(const char * path, char * buff, size_t buf_size)
{
	int error = 0;
	off_t off;
	inode_t * inoptr;

	//checkname(path);

	//printf("sys_readlink = %s\n",path);

	error = open_namei(path, 0, &inoptr);
	if(error!=OK)
		return -ENOENT;

	if(!S_ISLNK(inoptr->i_mode)){
		iput(inoptr);
		return -EPERM;
	}

	//if(!admit(inoptr, I_RB)){
	//	iput(inoptr);
	//	return -EACCES;
	//}

	//dup_inode(inoptr);
	//iput(inoptr); // XXX: unlock first

	off = vfs_readlink(inoptr,buff,buf_size);

	iput(inoptr);

	return off;
}

