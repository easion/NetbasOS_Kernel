

#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/errno.h>
#include <signal.h>
#include <assert.h>
#include "sys.h"

int read_proc(char *name, char *buf, int len);
int write_proc(char *name, char *buf, int len);


#ifndef FILESYSTEM_PREFIX_LEN
# define FILESYSTEM_PREFIX_LEN(Filename) 0
#endif

#ifndef ISSLASH
# define ISSLASH(C) ((C) == '/')
#endif


/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

char *
basename (char const *name)
{
  char const *base = name + FILESYSTEM_PREFIX_LEN (name);
  int all_slashes = 1;
  char const *p;

  for (p = name; *p; p++)
    {
      if (ISSLASH (*p))
        base = p + 1;
      else
        all_slashes = 0;
    }

  /* If NAME is all slashes, arrange to return `/'.  */
  if (*base == '\0' && ISSLASH (*name) && all_slashes)
    --base;

  /* Make sure the last byte is not a slash.  */
  //assert (all_slashes || !ISSLASH (*(p - 1)));

  return (char *) base;
}


int
dirname_r(const char*  path, char*  buffer, size_t  bufflen)
{
    const char *endp;
    int         result, len;

    /* Empty or NULL string gets treated as "." */
    if (path == NULL || *path == '\0') {
        path = ".";
        len  = 1;
        goto Exit;
    }

    /* Strip trailing slashes */
    endp = path + strlen(path) - 1;
    while (endp > path && *endp == '/')
        endp--;

    /* Find the start of the dir */
    while (endp > path && *endp != '/')
        endp--;

    /* Either the dir is "/" or there are no slashes */
    if (endp == path) {
        path = (*endp == '/') ? "/" : ".";
        len  = 1;
        goto Exit;
    }

    do {
        endp--;
    } while (endp > path && *endp == '/');

    len = endp - path +1;

Exit:
    result = len;
    if (len+1 > MAXPATH) {
        return -1;
    }
    if (buffer == NULL)
        return result;

    if (len > (int)bufflen-1) {
        len    = (int)bufflen-1;
        result = -1;
    }

    if (len >= 0) {
        memcpy( buffer, path, len );
        buffer[len] = 0;
    }
    return result;
}


int
basename_r(const char* path, char*  buffer, size_t  bufflen)
{
    const char *endp, *startp;
    int         len, result;
    char        temp[2];

    /* Empty or NULL string gets treated as "." */
    if (path == NULL || *path == '\0') {
        startp  = ".";
        len     = 1;
        goto Exit;
    }

    /* Strip trailing slashes */
    endp = path + strlen(path) - 1;
    while (endp > path && *endp == '/')
        endp--;

    /* All slashes becomes "/" */
    if (endp == path && *endp == '/') {
        startp = "/";
        len    = 1;
        goto Exit;
    }

    /* Find the start of the base */
    startp = endp;
    while (startp > path && *(startp - 1) != '/')
        startp--;

    len = endp - startp +1;

Exit:
    result = len;
    if (buffer == NULL) {
        return result;
    }
    if (len > (int)bufflen-1) {
        len    = (int)bufflen-1;
        result = -1;
    }

    if (len >= 0) {
        memcpy( buffer, startp, len );
        buffer[len] = 0;
    }
    return result;
}

/*
char*
basename(const char*  path)
{
    static char*  bname = NULL;
    int           ret;

    if (bname == NULL) {
        bname = (char *)malloc(MAXPATH);
        if (bname == NULL)
            return(NULL);
    }
    ret = basename_r(path, bname, MAXPATH);
    return (ret < 0) ? NULL : bname;
}
*/



#include <drv/proc_entry.h>



int dev_dir_open(char *filename, dev_prvi_t*dev,int *len)
{
	dev->data = NULL;

	dev->params[0] = 0;//
	dev->params[1] = 1;//dir
	dev->params[2] = dev_driver_header();//dir
	return 0;
}


int dev_dir_read(dev_prvi_t*dev,vfs_dirent_t *Entry)
{
	driver_ops_t *proc_entry=dev->params[2];

	if (!proc_entry)
	{
		return -1;
	}
	strcpy(Entry->l_long_name,proc_entry->d_name);
	//printf("proc_dir_read %s\n",Entry->l_long_name);
	dev->params[2] = TAILQ_NEXT(proc_entry,next_dev);
	return sizeof(vfs_dirent_t);
}

int proc_dir_open(char *filename, dev_prvi_t*dev,int *len)
{
	dev->data = NULL;

	dev->params[0] = 0;//
	dev->params[1] = 1;//dir
	dev->params[2] = proc_get_header();//dir
	return 0;
}


int proc_dir_read(dev_prvi_t*dev,vfs_dirent_t *Entry)
{
	struct proc_entry *proc_entry=dev->params[2];

	if (!proc_entry)
	{
		return -1;
	}
	strcpy(Entry->l_long_name,proc_entry->name);
	//printf("proc_dir_read %s\n",Entry->l_long_name);
	dev->params[2] = proc_entry->next;
	return sizeof(vfs_dirent_t);
}


int proc_file_open(char *filename,  int mode,dev_prvi_t*dev,int *len)
{
	int err;
	void *ptr = kcalloc(8096);

	char *base=basename(filename);


	if (!*base || IS_PROC_DIR(filename))
	{
	//printf("dir base = %s ---------\n",base);
		return proc_dir_open(filename,dev,len);
	}

	err = read_proc(base, ptr, 8096);
	//err = write_proc(filename, ptr, 8096);
	if (err==-1)
	{
		//printf("proc_open %s error\n",filename);
		kfree(ptr);
		return -1;
	}

	dev->params[1] = 0;//dir
	dev->data = ptr;
	//printf(ptr);
	dev->params[0] = err;
	if(err==0)
		dev->params[0]=8096;
	//read_proc();
	return err;
}




int proc_file_read(dev_prvi_t*dev,int pos,char *buf,int len)
{
	int size = dev->params[0];
	int bytes = size-pos;
	char *ptr = dev->data;

	if (bytes<=0)
	{
		return 0;
	}

	bytes = MIN(bytes,len);
	memcpy(buf,ptr+pos,bytes);
	return bytes;
}

int proc_file_write(dev_prvi_t*dev,char *buf,int pos,int len)
{
	return -1;
}


int proc_file_close(dev_prvi_t*dev)
{
	if(dev->data)
	kfree(dev->data);
	return 0;
}



