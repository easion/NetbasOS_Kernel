
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

/*
**     (R)Jicama OS
**     Path Name Conversion
**     Copyright (C) 2003 DengPingPing
*/


#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/spin.h>
#include <drv/errno.h>
#include <drv/log.h>
#include <assert.h>


mount_t* get_superblock(dev_t dev);

//CREATE_SPINLOCK( path_find_sem );


extern int do_mount(int _fsdev);


int sys_chroot(char *root)
{
	fs_task_t *fp = current_filp();
	//int curdsk = *c - 'A';
	return 0;
}


/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int get_iname(char **path, char *path_out)
{
	 char c;
     int i=0;
	 unsigned char *_iname=*path;

     while( (c = *_iname) == '/') _iname++;
      while( (c = *_iname) == ' ') _iname++;
  
     while( (c = *_iname) != '\0')
	{     
		_iname ++;
		if (i>=PATH_MAX){
			break;
		}
       if(c != '/'|| '\0'){
	    path_out[i++] = c;
	   }
       else break;
     }

     path_out[i]='\0';
	 *path = _iname;
     return i;
}

extern inode_t * iget(const int dev, const int number);


/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

void
truncate (inode_t *inode)
{
  mount_t *mp = get_superblock(inode->i_dev);

  if (!mp)
  {
	  return ;
  }
// 如果不是常规文件或者是目录文件，则返回。
  if (!(S_ISREG (inode->i_mode) || S_ISDIR (inode->i_mode)))
    return;

  vfs_truncate(mp, inode->i_number);
  inode->i_number=0;

  inode->i_size = 0;		// 文件大小置零。
  inode->i_dirt = 1;		// 置节点已修改标志。
}

char *
strncat(char *dst, const char *src, size_t n)
{
  if (n != 0)
  {
    char *d = dst;
    const char *s = src;

    while (*d != 0)
      d++;
    do {
      if ((*d = *s++) == 0)
	break;
      d++;
    } while (--n != 0);
    *d = 0;
  }
  return dst;
}

char * strfind(const char *p, int c)
{
   int c2;

   do {
      c2 = *p++;
      if (c2 == c) {
         return((char *)(p-1));
      }
   } while (c2);

   return NULL;
}


static inline inode_t*  get_root_inode(fs_task_t *fp,const char *dirname, int *pos)
{
	int isroot=0;
	inode_t* tmp;	
	int c=0;
	struct mount_list* list;
	*pos=0;


	if (dirname[0]=='/')
	{
		mount_t *mp;
		char vol[255]="";

		isroot = 1;

		get_mount_dir(vol,dirname,&c);

		list = find_mount_list(vol);

		if (!list||!vol[1])
		{
			//printf("find_mount_list() %s %d failed\n",vol,c);
			goto _local;
		}
		mp = list->m_super;
		if (!mp)
		{
			kprintf("get_root_inode() dir %s  error\n",dirname);
			return NULL;
		}

		*pos=c;

		//kprintf("get_root_inode() %s,%x \n",dirname,mp->m_root->i_dev);
		return mp->m_root;
	}
	
_local:
	//kprintf("from root2 %s %s\n",s,dirname);

	if (isroot){
		tmp= fp->root;
	}
	else{
		tmp= fp->pwd;
	}

	return tmp;
}


int get_child_node_name(inode_t* istart,ino_t ino,char *name)
{
	int err=-1;
#if 1
	file_t file;
	vfs_dirent_t vd;

	memset(file,0,sizeof(file));
	file.f_inode = istart;

	do
	{
		err=vfs_readdir(&file, &vd);
		if (vd.d.l_ino == ino)
		{
			strcpy(name,vd.l_long_name);
			return 0;
		}
	}
	while (err>=0);
	return -1;
#else
	inode_t* temp = NULL;


	temp = vfs_get_inode_name(istart,  ino,name);

	if (temp)
	{
		err=0;
	}
#endif
	 return err;
}


static int do_namei(inode_t* istart,const char* path_name, 
	int flags,  inode_t** new_node)
{
	int b_cur_node=0;
	int   read = 0;
	inode_t* temp = NULL;
	inode_t* inoptr = NULL;
	char *namep=path_name;
	static	 unsigned char _name[PATH_MAX];

	inoptr = istart;

	dup_inode(inoptr);	
	temp = inoptr ;
	

	while((read = get_iname(&namep,_name)) != 0)
	{
		if (read>=PATH_MAX)
		{
			break;
		}

		if (strcmp(_name,".")==0)
		{
			b_cur_node = 1;
			//printf("path_node_str = %s\n", path_node_str);
			//当前目录
			continue;
		}
		else
			b_cur_node = 0;


		temp = vfs_opendir(inoptr,  _name);
		iput(inoptr);

		if(temp == NULL){
			//kprintf("vfs_opendir error dev=%x inode=%d _name=%s\n", inoptr->i_dev,inoptr->i_number,_name);
			break;
		}


		inoptr = temp;

		//memcpy(new_node, temp, sizeof(inode_t));  
		vfs_closedir(temp); //
	} 
	   
	if(b_cur_node==0 && temp == NULL && read != 0){
		//kprintf("open_namei() %s error %d\n",path_name,read);
		//spin_unlock( &path_find_sem );
		//printf("new_node->i_dev = %x\n",new_node->i_dev);
		return -ENAMETOOLONG;
	}

	
	*new_node = temp;
	//istart->i_count --;

	//spin_unlock( &path_find_sem );
	return 0;
}

int open_namei(const char* path_name, 
	int flags,  inode_t** new_node)
{
	fs_task_t *fp = current_filp();
	inode_t* tmp;
	int pos=0;
	int err;
	
	tmp = get_root_inode(fp,path_name,&pos);//path_name;

	if (!tmp)
	{
		kprintf("open_namei() dir %s  error\n",path_name+pos);
		return -1;
	}

		//kprintf("open_namei() dir %s ,pos%d succ\n",path_name+pos,pos);
	err= do_namei(tmp,path_name+pos,flags,new_node);
	return err;
}

int sys_chdir(unsigned char* new_path)
{
	int error;
	inode_t* inode ;
	inode_t *cnode;
	fs_task_t *fp = current_filp();

	if(new_path == NULL)
		return -EINVAL;

	error = open_namei(new_path, O_RDONLY,  &inode);

	if(error != 0)return -ENOENT;

	if(!S_ISDIR(inode->i_mode)){
		iput(inode);
		syslog(4, "%s: Invalid Directory \n", new_path);
		return -ENOTDIR;
	}

	cnode = iget(inode->i_dev, inode->i_number);

	iput(inode);

	if(!cnode ){
		//panic("%s(): iget() null node!\n", __FUNCTION__);
		return -1;
	}

	//printf("chroot to %d\n", cnode->i_number);

	fp->pwd=cnode;
	return 0;
}

int insert_name( char *pbuf, int curlen, char *name, int len )
{
	if ( curlen > 0 )
	{
		memmove( pbuf + len + 1, pbuf, curlen );
	}
	pbuf[0] = '/';
	memcpy( pbuf + 1, name, len );
	return ( curlen + len + 1 );
}

int do_get_pwd(char * buff, size_t size)
{
	ino_t inum;
	int error, len, idx;
	char name[PATH_MAX + 1];
	inode_t * tmp;
	fs_task_t *current = current_filp();
	inode_t *fino ;
	int curpos=0;

	if(!buff)
		return -EFAULT;

	buff[0]=0;

	error = 0;

	tmp = current->pwd;
	assert(tmp);

	if (tmp==current->root)
	{
		//kprintf("do_get_pwd rootdir\n");
		strcpy(buff,"/");
		return 0;
	}
	//tmp->i_count++; // XXX
	//ilock(tmp);

	//buff[size - 1] = '\0';
	//idx = size - 1;

		dup_inode(tmp);
	do{
		if (tmp->i_number == current->root->i_number)
		{
			iput(tmp);
			//printf("root dir\n");
			break;
		}
		inum = tmp->i_number;
		error = do_namei(tmp,"..", O_RDONLY,  &fino);

		if(error != 0){
			printf("do_get_pwd do_namei error\n");
			//return -ENOENT;
			break;
		}

		//if(inum == fino->i_number)
		//	break;

		error = get_child_node_name(fino, tmp->i_number, name); 
		if(error < 0){
			printf("do_get_pwd get_child_node_name error\n");
			break;
		}

		curpos = insert_name( buff, curpos, name, strlen(name) );

		/*len = strlen(name);
		if((idx - (len + 1)) < 0){
			printf("do_get_pwd len error\n");
			error = -ERANGE;
			break;
		}

		idx -= len; 
		memcpy(&buff[idx], name, strlen(name));

		idx--;
		buff[idx] = '/';
		*/

		iput(tmp);

		tmp = fino;
	}while(1);

	iput(fino);

	if(!error){
		int i;

		/*if(idx >= (size - 1)){
			buff[0] = '/';
			buff[1] = '\0';
		}else if(idx > 0){
			for(i = 0; buff[idx]; i++, idx++)
				buff[i] = buff[idx];

			buff[i] = '\0';
		}*/
	}
	else{
	printf("getcwd: err on %s %d\n",buff,error);
	strcpy(buff,"/");
	error=0;
	}

	return error;
}



