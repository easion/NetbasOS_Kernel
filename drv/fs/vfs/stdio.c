/*
**     (R)Jicama OS
**     File Control Block
**     Copyright (C) 2003 DengPingPing
*/

#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <drv/errno.h>
#include <assert.h>


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

file_t *fdopen(int fd);
int do_write( file_t * filp, char * buf, int count);
int read_pipe(file_t * filp, char * buf, int count);
int write_pipe(file_t * filp, char * buf, int count);


/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int fseek(file_t *fp, off_t offset, int where)
{
	int fsz=fp->f_inode->i_size;
	int pos=fp->f_pos;
	if(!fp)return -EINVAL;

	assert(fp!=NULL);

	switch (where)
	{
	case SEEK_SET:
		    if (offset < 0)  return -EINVAL;
	      pos = offset;
	break;
	case SEEK_CUR:
		    if (pos + offset < 0)return -EINVAL;
			pos += offset;
			break;	
	case SEEK_END:
		 if ((pos = fsz + offset) < 0)	return -EINVAL;
	       pos = fsz+ offset;
			break;
		default:
		break;
	}

	if(pos<0)return -1;
	fp->f_pos=pos;
	return pos;
}

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int fsize(int fd, int size)
 {
	file_t *fp=fdopen(fd);

	if(!fp)
		return -1;
	
	fp->f_inode->i_size = size;
	//do_fsize(fd);
	return OK;
}

bool feof(file_t *f)
{
	assert(f!=NULL);
	return f->f_pos <= f->f_inode->i_size ?FALSE:TRUE;
}


/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int do_filp_read(file_t * filp, char * buf, int count)
{
	int real_reads=0;
	int left,char_counts;
	off_t blk_no=0;
	buffer_t * bh;
	inode_t *inode=filp->f_inode;

	assert(filp!=NULL);
	assert(inode!=NULL);

	left=MIN((inode->i_size-filp->f_pos), count) ;

	/*从块设备读取文件内容*/
	if ((left)<=0)
		return 0;

	while (left) {

		if(filp->f_pos > inode->i_size)
			return 0;

		blk_no = vfs_bmap(inode, filp->f_pos, 0);

		if (blk_no != (off_t)-1) {
			dev_prvi_t*dp = get_dev_desc(inode->i_dev);
			bh = bread(dp, blk_no);
			if (!(bh)){
				real_reads = -1;
				kprintf("do_filp_read: bread dev %x %x @%d error\n", dp->devno,inode->i_dev,blk_no );
				break;
			}
			else{
				//printf("do_filp_read: %x idev=%x ino=%d succ\n", dp->devno,inode->i_dev,blk_no);
			}
		} 
		else{
			bh = NULL;
		}

		blk_no = filp->f_pos % (bh->b_size);
		char_counts = MIN( dev_block_size(inode->i_dev) - blk_no , left );
		filp->f_pos += char_counts;
		left -= char_counts;
		real_reads += char_counts;

		if (bh) {
			memcpy(buf, &bh->b_data[blk_no], char_counts);
			buf_release(bh);
		} else {
			break;
		}
		buf += char_counts;
	}

	inode->i_atime = startup_ticks();
	return real_reads;
}



int fwrite( file_t * filp, char * buf, int count)
{
	off_t pos;
	int block,c;
	buffer_t * bh;
	char * p;
	int i=0;
	inode_t * inode = filp->f_inode;

	assert(filp != NULL);

	if (filp->f_mode & O_APPEND)
		pos = inode->i_size;
	else	
		pos = filp->f_pos;

	while (i<count) {
		if (!(block = vfs_bmap(filp->f_inode, pos, 1))) /*error here!*/
			break;

		bh = bread(get_dev_desc(inode->i_dev),block);

		if (!bh)
			break;

		c = pos % dev_block_size(inode->i_dev);
		p = bh->b_data + c;

		mark_buf_dirty(bh);
		c = dev_block_size(inode->i_dev)-c;

		if (c > count-i)
			c = count-i;
		pos += c;

		if (pos > inode->i_size) {
			inode->i_size = pos;
			inode->i_dirt = 1;
		}

		i += c;
		memcpy(p, buf, c);

		buf_release(bh);
		buf += c;
	}
	//inode->i_mtime = CURRENT_TIME;
	if (!(filp->f_mode & O_APPEND)) {
		filp->f_pos = pos;
	}

	vfs_write_inode(inode);
	return (i?i:-1);
}

static int file_write(file_t * filp, char * buf, int count)
{	
	int n = -1,e = 0;
	inode_t* inode ;

	if (!filp)
		return -1;

	inode = filp->f_inode;
	if (!inode)
		return -1;

	if (S_ISFIFO(inode->i_mode))
	{
		n = write_pipe(filp, buf, count);	
	}
	else if (S_ISSOCK(inode->i_mode))
	{
		n = send(filp->f_iob,buf,count,0);
	}
	else if (S_ISBLK(inode->i_mode))
	{
		n= dev_write(filp->f_iob,filp->f_pos,buf,count);
		if(n>0)
			filp->f_pos+=n;
	}
	else if (S_ISWHT(inode->i_mode)){
	}	
	else if (S_ISREG(inode->i_mode )){
		n=vfs_write(filp, buf, count,&e);
		if (!e)
		{
		n = fwrite(filp, buf, count);
		}
	}
	else{
	   n = -1;
	   printf("file_write:error file type %x\n", inode->i_mode);
	}

	return n;
}



int file_read(file_t * filp, char * buf, int count)
{
	int ncount, n;
	int exist = 0;
	inode_t* inode = filp->f_inode;
	
	if (!filp){
		kprintf("file_read():filp is free slot!\n");
		return -1;
	}

	if (S_ISFIFO(filp->f_inode->i_mode)){
		return read_pipe(filp, buf, count);	
	}
	else if ( S_ISSOCK(filp->f_inode->i_mode))
	{
		return recv(filp->f_iob,buf,count,0);
	}
	else if (S_ISCHR(filp->f_inode->i_mode)||S_ISBLK(filp->f_inode->i_mode))
	{
		n= dev_read(filp->f_iob,filp->f_pos,buf,count);
		if(n>0)
			filp->f_pos+=n;
		return n;
	}
	else if (S_ISWHT(inode->i_mode)){
		n= proc_file_read(filp->f_iob,filp->f_pos,buf,count);
		if(n>0)
			filp->f_pos += n;
		return n;
	}	

	ncount = vfs_read(filp, buf, count, &exist);

	if (exist)
	{
		return ncount;
	}
	else{
		//kprintf("vfs read failed %d\n",ncount);
		ncount = do_filp_read(filp, buf, count);
	}
	return ncount;
}

int sys_write(int fd, const char * buf, int count, int arg)
{
	int ret = -2;
	file_t * filp=NULL;

	if (fd>=NR_FILES || fd<0){
		return -EINVAL;//
	}

	filp = current_filp()->fp[fd];

	if (!filp)
	{
		printf("file_write fd = %d,%d empty,tid=%d\n",fd,count,current_thread_id(NULL));
		return -1;
	}

	ret = file_write(filp, buf, count);
	if (ret<0)
	{
	  printf("file_write fd = %d error\n",fd);
	}

	return ret;
}

int sys_read(int fd, char * buf, int count, int arg)
{
	int ret;
	file_t * filp=NULL;

	if (fd>=NR_FILES || fd<0)
		return -EINVAL;

	filp=current_filp()->fp[fd];

	if (!filp)
	{
		kprintf("sys_read: fd %d empty\n",fd);
		return -1;
	}

	//printf("sys_read %d %d********\n",fd,count);

	ret = file_read(filp, buf, count);

	return ret;	
}

int sys_lseek(int fd, off_t pos, int where)
{
	file_t * filp = current_filp()->fp[fd];

	if (fd >= NR_FILES || fd<0){
		return -EINVAL;//
	}
	if (!filp)
		return -1;

	if (!S_ISREG(filp->f_inode->i_mode)&&!S_ISDIR(filp->f_inode->i_mode))
		return -1;	

	return fseek(filp, pos, where);
}


long ftell(register file_t *fp)
{
	return fp->f_pos;
}

int sys_readdir(unsigned int fd, void *buf, int arg)
{
	file_t * filp;
	vfs_dirent_t *Entry = (vfs_dirent_t *)buf;	

	if (fd>=NR_FILES || fd<0){
		return -EINVAL;//
	}

	filp = current_filp()->fp[fd];
	
	if (!filp){
		//printk("not used!\n");
		return -1;
	}

	inode_t* inode = filp->f_inode;

	if (S_ISWHT(inode->i_mode)){
		return proc_dir_read(filp->f_iob,Entry);
	}
	else if (S_ISCHR(inode->i_mode) || S_ISBLK(inode->i_mode)){
		return dev_dir_read(filp->f_iob,Entry);
	}
	else if (vfs_readdir(filp, Entry) < 0)
	{
		return -1;
	}

	return 0;
}


