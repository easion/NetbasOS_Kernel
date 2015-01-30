
#include <drv/drv.h>
#include <drv/fs.h>
#include "fat.h"
#include <drv/errno.h>
#include <drv/ansi.h>
#include <assert.h>
#include <drv/log.h>

int  msdos_find_file(file_t *fp, char *name, struct msdos_dir *buf_addr);
int msdos_dir_size(mount_t *, u16_t start);

inode_t* msdos_opendir(inode_t* old, unsigned char *name)
{
	int offset;
	struct msdos_dir dos_dir;
	inode_t* child = NULL;
	file_t fp;
	mount_t *sp;

	assert(old!=NULL);

	memset((void *)&dos_dir, 0, sizeof(struct msdos_dir));
	memset((void *)&fp, 0, sizeof(file_t));

	fp.f_mode = O_RDONLY;
	fp.f_inode = old;
	fp.f_pos = 0;

	offset = msdos_find_file(&fp,  name, &dos_dir);
	if(offset<0){
		//syslog(4, "%s(): %s not found\n", __FUNCTION__, name);
		return NULL;
	}


	sp = get_superblock(old->i_dev);
	assert(sp!=NULL);

	switch (sp->m_magic)
	{
	case FAT12_MAGIC:
	case FAT16_MAGIC:
	     child = iget(old->i_dev, dos_dir.first_cluster);
	break;
	case FAT32_MAGIC:
	      child = iget(old->i_dev, FIRSTCLUSTER(dos_dir));
		  //printf("%s():find %s at cluster %d\n ",__FUNCTION__, name, FIRSTCLUSTER(dos_dir));
	break;
	default:
		panic("msdos_opendir");
		break;	
	}

	child->i_size = msdos_dir_size(sp, child->i_number);

	if (MSDOS_ISDIR(dos_dir.attribute)){
		child->i_mode = typemode(S_IFDIR, 0666);
	}
    else{
		child->i_mode =  typemode(S_IFREG, 0777); 
	    child->i_size = dos_dir.file_size;
	}

	child->i_father = old->i_number;
	child->i_seek = offset; 
	return child;
}

inode_t* msdos_get_inode_name(inode_t* old, ino_t ino,unsigned char *name)
{
	int offset;
	struct msdos_dir dos_dir;
	inode_t* child = NULL;
	file_t fp;
	mount_t *sp;

	assert(old!=NULL);

	memset((void *)&dos_dir, 0, sizeof(struct msdos_dir));
	memset((void *)&fp, 0, sizeof(file_t));

	fp.f_mode = O_RDONLY;
	fp.f_inode = old;
	fp.f_pos = 0;

	//offset = msdos_find_file(&fp,  name, &dos_dir);
	//if(offset<0){
		//syslog(4, "%s(): %s not found\n", __FUNCTION__, name);
		return NULL;
	//}


	sp = (old->i_super);
	assert(sp!=NULL);

	switch (sp->m_magic)
	{
	case FAT12_MAGIC:
	case FAT16_MAGIC:
	     child = iget(old->i_dev, dos_dir.first_cluster);
	break;
	case FAT32_MAGIC:
	      child = iget(old->i_dev, FIRSTCLUSTER(dos_dir));
		  //printf("%s():find %s at cluster %d\n ",__FUNCTION__, name, FIRSTCLUSTER(dos_dir));
	break;
	default:
		panic("msdos_opendir");
		break;	
	}

	child->i_size = msdos_dir_size(sp, child->i_number);

	if (MSDOS_ISDIR(dos_dir.attribute)){
		child->i_mode = typemode(S_IFDIR, 0666);
	}
    else{
		child->i_mode = typemode(S_IFREG, 0777) ;
	    child->i_size = dos_dir.file_size;
	}

	child->i_father = old->i_number;
	child->i_seek = offset; 
	return child;
}


int msdos_do_readdir(file_t *fp, vfs_dirent_t *vp, int enable_create, int *off)
{
	fatfs_find_t to_find;
	fatfs_find_t *findbuf = &to_find;
	inode_t *inode = fp->f_inode;
	fat_dir_entry  dir_buf[LFN_FETCH_SLOTS];
	int        start_fpos = 0; /*start first entry*/
	int        num_bytes;
	u16_t       name_utf16[255];

	num_bytes = do_filp_read(fp, (char *)&dir_buf[0], 32);
	if (num_bytes < 0) {
	  syslog(4, "%s() do_filp_read %d zero\n", __FUNCTION__, fp->f_inode->i_number);
	  return num_bytes;
	}
	if(off)*off+=32;

  while (num_bytes > 0)
  {
    if (dir_buf[start_fpos].file_name[0] == 0x00 ){
		//if(enable_create)return 0;
		return -1;
	}

	if(dir_buf[start_fpos].file_name[0] == 0xe5) {
		if(enable_create)return 0;
	}
	else{

      if (dir_buf[start_fpos].attribute != MSDOS_LONGNAME)/*skip all long filename entry */
      {
        findbuf->SfnEntry    = dir_buf[start_fpos];
        fat_expand_name(&dir_buf[start_fpos], findbuf->l_long_name);

        findbuf->LfnEntries = fat_fetch_lfn(dir_buf, start_fpos, name_utf16);
		  vp->l_name_len = 32;

        if (findbuf->LfnEntries){
          fd32_utf16to8(name_utf16, findbuf->l_long_name);
		}
      else{
          //strcpy(findbuf->l_long_name, findbuf->l_short_name); 
	  }
	convdos(vp, &to_find.SfnEntry);
	strcpy(vp->l_long_name, to_find.l_long_name);
       return 0; /*ok backup*/
      }

    }

    if (++start_fpos == LFN_FETCH_SLOTS) 
		start_fpos = 0;

    num_bytes = do_filp_read(fp, (char *)&dir_buf[start_fpos], 32);
    if (num_bytes < 0) return num_bytes;
  if(off)*off+=32;
  } /*end  while */

  //panic("no found\n");


  return -1;
}


//read only
int msdos_readdir(file_t *fp, vfs_dirent_t *vp)
{
	return msdos_do_readdir(fp, vp, false, NULL);
}

