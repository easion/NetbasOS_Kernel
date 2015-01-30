
#include <drv/drv.h>
#include <stdarg.h>
#include <drv/fs.h>
#include <drv/unistd.h>
#include <drv/buffer.h>
#include <drv/errno.h>
#include <drv/log.h>
#include "../fat/fat.h"

#include "cdfs.h"

int iso9660_find_in_dir(struct cdfs *cdfs, int dir, char *name, 
int len, buffer_t **dirbuf, struct iso_directory_record **_dirrec);

static int iso9660_fnmatch(struct cdfs *cdfs, char *fn1, int len1, char *fn2, int len2)
{
  if (cdfs->joliet)
  {
    u16_t *wfn2 = (u16_t *) fn2;
    int wlen2 = len2 / 2;
    if (wlen2 > 1 && ntohs(wfn2[wlen2 - 2]) == ';') 
		wlen2 -= 2;
    if (wlen2 > 0 && ntohs(wfn2[wlen2 - 1]) == '.') 
		wlen2 -= 1;
    if (len1 != wlen2) 
		return 0;
    while (len1--) if (*fn1++ != ntohs(*wfn2++)) 
		return 0;
    return 1;
  }
  else
  {
    if (len2 > 1 && fn2[len2 - 2] == ';') 
		len2 -= 2;
    if (len2 > 0 && fn2[len2 - 1] == '.') 
		len2 -= 1;
    if (len1 != len2) 
		return 0;
    while (len1--) if (*fn1++ != *fn2++) 
		return 0;
    return 1;
  }
}



static int iso9660_find_dir(struct cdfs *cdfs, int curdirno, char *name, int len)
{
  char *p;
  int l;
  int dir = 2;
  int parentDirectory = curdirno;

  while (1)
  {
    // Skip path separator
    if (*name == PS1 || *name == PS2) {
      name++;
      len--;
    }

    if (len == 0){
		kprintf("iso9660_find_dir(): parentDirectory \n");
		return parentDirectory;
	}

    // Find next part of name
    p = name;
    l = 0;

    while (l < len && *p != PS1 && *p != PS2) {
      l++;
      p++;
    }

    // Find directory for next name part
    while (dir < cdfs->path_table_records)
    {
      if (cdfs->path_table[dir]->parentDirectory != parentDirectory){
		  kprintf("iso9660_find_dir parentDirectory error \n");
		  return -ENOENT;
	  }
      if (iso9660_fnmatch(cdfs, name, l, cdfs->path_table[dir]->name, cdfs->path_table[dir]->length)){
		  kprintf("iso9660_find_dir() : found %s ok \n", name);
		  break;
	  }
      dir++;
    }

    // If we reached the end of the path table the directory does not exist
    if (dir == cdfs->path_table_records){
		//kprintf("iso9660_find_dir() : dir %s not exist \n", name);
		return -ENOENT;
	}
        
    // If we have parsed the whole name return the directory number
    if (l == len){
		//kprintf("iso9660_find_dir() : found dir at %d \n", dir);
		return dir;
	}

    // Go forward in path table until first entry for directory found
    parentDirectory = dir;

    while (dir < cdfs->path_table_records)
    {
      if (cdfs->path_table[dir]->parentDirectory 
		  == parentDirectory) 
		  break;
      dir++;
    }

    // Prepare for next name part
    name = p;
    len -= l;
  }
}



int iso9660_find_file(struct cdfs *cdfs, char *name, int len, buffer_t **buf, struct iso_directory_record **rec)
{
  int dir;
  int split;
  int n;

  // If root get directory record from volume descriptor
  if (len == 0) {
    struct iso_volume_descriptor *vd;

    if(buf)*buf = bread(cdfs->devptr, cdfs->vdblk);
    if (!*buf) return -EIO;

    vd = (struct iso_volume_descriptor *) (*buf)->b_data;
    *rec = (struct iso_directory_record *) vd->root_directory_record;
    return 0;
  }

  // Split path into directory part and file name part
  split = -1;
  for (n = 0; n < len; n++) if (name[n] == PS1 || name[n] == PS2) split = n;

  // Find directly if file located in root directory
  if (split == -1) 
	  return iso9660_find_in_dir(cdfs, ISO9660ROOT_NO, name, len, buf, rec);

  // Locate directory
  dir = iso9660_find_dir(cdfs,ISO9660ROOT_NO, name, split + 1);
  if (dir < 0) return dir;

  // Find filename in directory
  return iso9660_find_in_dir(cdfs, dir, name + split + 1, len - split - 1, buf, rec);
}

time_t iso9660_isodate(unsigned char *date)
{
  struct tm tm;

  memset(&tm, 0, sizeof tm);

  tm.year = date[0];
  tm.month = date[1] - 1;
  tm.day = date[2];
  tm.hour = date[3];
  tm.min = date[4];
  tm.sec = date[5];
  tm.min += (*(char *) &date[6]) * 15;

  tm.__tm_gmtoff = -1;

  return to_unix_time(&tm);
}

static time_t iso96602dos_isodate(unsigned char *date)
{
  //struct ftime time;
  //struct  fdate fdate;
  time_t t;
  struct tm tm;
  tm.__tm_gmtoff = -1;

 // memset((void *)&time, 0, sizeof time);
 // memset((void *)&fdate, 0, sizeof fdate);

  tm.year = date[0] - 1980 - 20;
  tm.month = date[1] ;
  tm.day = date[2];

  tm.hour = date[3];
  tm.min = date[4];
  tm.sec = date[5];
  tm.min += (*(char *) &date[6]) * 15;

  t = to_unix_time(&tm);

	//t = *(unsigned short*)(&time) 
	//	  + ((*(unsigned short*)(&fdate)) << 16);

  return (t);
}

int iso9660_find_in_dir(struct cdfs *cdfs, int dir, char *name, 
  int len, buffer_t **dirbuf, struct iso_directory_record **_dirrec)
{
  buffer_t *buf;
  char *p;
  struct iso_directory_record *rec;
  int blk;
  int left;
  int reclen;
  int namelen;

  // The first two directory records are . (current) and .. (parentDirectory)
  blk = cdfs->path_table[dir]->extent;
  buf = bread(cdfs->devptr, blk++);  
  if (!buf) 
	  return -EIO;

  // Get length of directory from the first record
  p = buf->b_data;
  rec = (struct iso_directory_record *) p;
  left = isonum_733(rec->size);

  // Find named entry in directory
  while (left > 0)
  {
    // Read next block if all records in current block has been read
    // Directory records never cross block boundaries
    if (p >= buf->b_data + CDROM_SECTOR_SIZE) {		

      if (p > buf->b_data + CDROM_SECTOR_SIZE){
	    buf_release(buf);
		return -EIO;
	  }

 	  buf_release(buf);
      buf = bread(cdfs->devptr, blk++);
      if (!buf) 
		  return -EIO;
      p = buf->b_data;
    }

    // Check for match
    rec = (struct iso_directory_record *) p;
    reclen = isonum_711(rec->length);
    namelen = isonum_711(rec->name_len);
    
    if (reclen > 0)   {

      if (iso9660_fnmatch(cdfs, name, len, (char *) rec->name, namelen))   {
        *_dirrec = rec;
        *dirbuf = buf;
		//buf_release(buf);
        return 0;
      }

      // Skip to next record
      p += reclen;
      left -= reclen;
    }
    else   {
      // Skip to next block
      left = left - (int) ((buf->b_data + CDROM_SECTOR_SIZE) - (unsigned char*)p);
      p = buf->b_data + CDROM_SECTOR_SIZE;
    }
  }

  buf_release(buf);
  return -ENOENT;
}



int iso9660_dir_get(file_t *filp, struct dirent *dirp, int *isdir)
{
  mount_t *mp = get_superblock(filp->f_inode->i_dev);
  struct cdfs *cdfs = mp->m_private_data;
  struct iso_directory_record *rec;
  buffer_t *buf = NULL;
  int namelen;
  int reclen;
  char *name;
  u16_t *wname;
  inode_t * inode= filp->f_inode;
  unsigned long flags;

blkagain:
	if (filp->f_pos >= inode->i_size) 
		return 0;

	// Get directory block
	buf = bread(cdfs->devptr, inode->i_number + (int) filp->f_pos / CDROM_SECTOR_SIZE);
	if (!buf) {
		kprintf("buffer loader inode %d error\n", inode->i_number );
		return -EIO;
	}
  
  // Locate directory record
recagain:
	rec = (struct iso_directory_record *) (buf->b_data + (int) filp->f_pos % CDROM_SECTOR_SIZE);
	reclen = isonum_711(rec->length);
	namelen = isonum_711(rec->name_len);

	// Check for no more records in block
	if (reclen == 0)
	{
		int blkleft = CDROM_SECTOR_SIZE - ((int) filp->f_pos % CDROM_SECTOR_SIZE);
		filp->f_pos += blkleft;
		buf_release(buf);
		goto blkagain;
	}

	// Check for . and .. entries
	if (namelen == 1 && (rec->name[0] == 0 || rec->name[0] == 1))
	{
		filp->f_pos += reclen;
		goto recagain;
	}

	// Get info from directory record
	dirp->ino = isonum_733(rec->extent);
	dirp->reclen = sizeof(struct dirent) - MAXPATH + namelen + 1;
	dirp->date = iso9660_isodate(rec->date);
	dirp->size = isonum_733(rec->size);
	flags = isonum_711(rec->flags);

	if (isdir )
	{
		 if(flags & 2)
			*isdir = 1;
		 else
			*isdir = 0;
	}
	

  if (cdfs->joliet)
  {
    int n;

    namelen /= 2;
    wname = (u16_t *) rec->name;
    if (namelen > 1 && ntohs(wname[namelen - 2]) == ';') namelen -= 2;
    if (namelen > 0 && ntohs(wname[namelen - 1]) == '.') namelen -= 1;

    dirp->namelen = namelen;
    for (n = 0; n < namelen; n++) dirp->name[n] = (char) ntohs(wname[n]);
    dirp->name[namelen] = 0;
  }
  else
  {
    name = (char *) rec->name;
    if (namelen > 1 && name[namelen - 2] == ';') namelen -= 2;
    if (namelen > 0 && name[namelen - 1] == '.') namelen -= 1;

    dirp->namelen = namelen;
    memcpy(dirp->name, name, namelen);
    dirp->name[namelen] = 0;
  }
  
  filp->f_pos +=  reclen;
  buf_release(buf);
  return 1;
}

void lower_fn(char *name)
{
	int i;

   for (i=0;name[i];i++)
   {
      if (!isascii(name[i])) break;
      name[i]=tolower(name[i]);
   };
}

int iso9660_readdir(file_t *fp, vfs_dirent_t *vp)
{
	 struct dirent dir;
	 int isdir = 3;
	 static int t = 0;
	
	 memset((void *)&dir,0,sizeof(dir));

	 if (!fp || !vp)
		 return -1;

	if (iso9660_dir_get(fp, &dir, &isdir) <= 0)
		return -1;

	//vp->l_size_high = 0;
	vp->l_size_high = 0;
	vp->l_size_low = dir.size;
	vp->l_atime = dir.date;
	vp->l_ctime = dir.date;
	vp->l_mtime  =  dir.date; //iso96602dos_isodate(dir.date);
	lower_fn(dir.name);
	strcpy(vp->l_long_name, dir.name);
	//kprintf("file %s\n", dir.name);

	if (isdir)
		vp->l_attribute = MSDOS_DIR;
	else
		vp->l_attribute = MSDOS_ARCH;

	vp->l_name_len = sizeof(struct iso_directory_record);
	vp->d.l_ino = dir.ino;
	return 1;
}

int iso9660_open(inode_t *inode, char *name)
{
  mount_t *mp = get_superblock(inode->i_dev);
  struct cdfs *cdfs = mp->m_private_data;
  struct iso_directory_record *rec;
  time_t date;
  int size;
  int extent;
  int flags;
  int rc;
  buffer_t *buf = NULL;

  // Locate file in file system
  rc = iso9660_find_file(cdfs, name, strlen(name), &buf, &rec);

  if (buf)
  {
	  buf_release(buf);
  }

  if (rc < 0){ 
	  //kprintf("dir %s not found\n", name);
	  return rc;
  }

  flags = isonum_711(rec->flags);
  extent = isonum_733(rec->extent);
  date = iso9660_isodate(rec->date);
  size = isonum_733(rec->size);
 // release_buffer(cdfs->cache, buf);

  if (flags & 2){
		inode->i_mode = typemode(S_IFDIR, 0666);
	}
	else{
		inode->i_mode = typemode(S_IFREG, 0777); 
	}

  // Allocate and initialize file block
  /*cdfile = (struct iso9660_file *) kmalloc(sizeof(struct iso9660_file));
  if (!cdfile) return ENOMEM;
  filp->data = cdfile;*/
  inode->i_number = extent;
  inode->i_ctime = date;
  inode->i_atime = date;
  inode->i_mtime = date;
  inode->i_size = size;
  return 0;
}

inode_t* iso9660_opendir(inode_t * inode, char *nameptr)
{
	 file_t fp;
	 inode_t * inop;
	 struct dirent dir;
	 int found = 0 ;
	 int isdir;
	 char name[256];

	 iso9660_conv_name(name,nameptr);

	fp.f_mode = O_RDONLY;
	fp.f_inode = inode;
	fp.f_pos = 0;

	if (strcmp(name,"..")==0)
	{
		memset(&dir,0,sizeof(dir));
		dir.ino = inode->i_father;
		dir.size = 4096;
		goto done;
	}

	//kprintf("iso9660_opendir: %s\n", name);	

	while (1)
	{
		memset((void*)&dir,0,sizeof(dir));

		if (iso9660_dir_get(&fp, &dir, &isdir) <= 0)
		{
			/*read error*/
			break;
		}

		//printf("dir.name= %s\n", dir.name);

		if (stricmp(dir.name, name) == 0)
		{			
			found = 1;
			/*find ok*/
			break;
		}
	}

	if (!found){
		//printf("iso found  %s error\n",name);
		return NULL;
	}

done:
	inop = iget (inode->i_dev, dir.ino);

	if (!inop){
			printf("iget error\n");

		goto err; 
	}

	inop->i_father = inode->i_number;
	inop->i_size = dir.size;
	inop->i_ctime = dir.date;
	inop->i_atime = dir.date;
	inop->i_mtime = dir.date;

	if (isdir){
		inop->i_mode = typemode(S_IFDIR, 0666) ;
		//kprintf("found: %s size is %d\n", dir.name, dir.size);
	}
	else
		inop->i_mode = typemode(S_IFREG, 0777) ;
	//kprintf("found: %s size is %d\n", dir.name, dir.size);
	return inop;

err:
	 return NULL;
}


char* iso9660_conv_name(char * pnew,  char *name)
{
	char *pname=name;
	char *name_cdfs=pnew;

	//memset(pnew,0,256);

	while (*pname)
	{
		if (*pname=='-')
		{
			*name_cdfs = '_';
		}
		else
		*name_cdfs = *pname;
		name_cdfs++;
		pname++;
	}
	*name_cdfs = 0;
	return pnew;
}

inode_t* iso9660_get_inode_name(inode_t * inode, ino_t ino, char *nameptr)
{
	 file_t fp;
	 inode_t * inop;
	 struct dirent dir;
	 int found = 0 ;
	 int isdir;
	 int cnt=0;

	 char name[256];

	 iso9660_conv_name(name,nameptr);

	fp.f_mode = O_RDONLY;
	fp.f_inode = inode;
	fp.f_pos = 0;

	

	while (1)
	{
		memset((void*)&dir,0,sizeof(dir));

		if (iso9660_dir_get(&fp, &dir, &isdir) <= 0)
		{
			/*read error*/
			break;
		}

		//printf("dir.ino = %d,fp.f_pos=%d\n", dir.ino,fp.f_pos);

		if (dir.ino == ino)
		{			
			found = 1;
			/*find ok*/
			break;
		}
		cnt++;
	}

	if (!found){
		kprintf("iso9660_get_inode_name: %s,ino%d %d not found\n", name,ino,cnt);	
		return NULL;
	}

	strncpy(name,dir.name,PATH_MAX);

	inop = iget (inode->i_dev, dir.ino);

	if (!inop)
		goto err; 

	inop->i_father = inode->i_number;
	inop->i_size = dir.size;
	inop->i_ctime = dir.date;

	if (isdir)
		inop->i_mode = typemode(S_IFDIR, 0666);
	else
		inop->i_mode = typemode(S_IFREG, 0777) ;
	kprintf("iso9660_get_inode_name found: %s size is %d\n", dir.name, dir.size);
	return inop;

err:
	 return NULL;
}


off_t iso9660_bmap(inode_t* inode, int offset, int create)
{
	off_t blk = inode->i_number + offset/CDROM_SECTOR_SIZE;

	if (offset<0)
		return (off_t)-1;

	if ( inode->i_size <= offset )
		return (off_t)-1;

	//kprintf("bmap blk%d\n", blk);
	return blk;
}

