

#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/errno.h>

 

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int do_access (const char *filename, int mode)
{
	int error;
	inode_t *inode = NULL;
	int res, i_mode;
	fs_task_t *fp = current_filp();

	mode &= 0007;

	error = open_namei(filename, O_RDONLY,  &inode);
	if(error != OK)return -ENOENT;	

	i_mode = res = inode->i_mode & 0777;
	iput (inode);

	if (fp->uid == inode->i_uid){
		res >>= 6;
		res >>= 6;
	}

	iput(inode);

	if ((res & 0007 & mode) == mode)
		return 0;

	if ((!fp->uid) && (!(mode & 1) || (i_mode & 0111)))
		return 0;
	return -EACCES;
}

