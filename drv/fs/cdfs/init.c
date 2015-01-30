

#include <drv/drv.h>
#include <stdarg.h>
#include <drv/fs.h>
#include "../fat/fat.h"
#include <drv/unistd.h>
#include <drv/buffer.h>
#include "cdfs.h"
#include <drv/log.h>

static int iso9660_read_path_table(struct cdfs *cdfs, struct iso_volume_descriptor *vd);
static inline int _strncmp (const char *s1, const char *s2, unsigned long len);
int iso9660_bmap(inode_t* inode, int offset, int create);

inode_t* iso9660_opendir(inode_t * inode, char *name);
inode_t* iso9660_get_inode_name(inode_t * inode, ino_t ino, char *name);

int iso9660_readdir(file_t *fp, vfs_dirent_t *p);
static int iso9660_mount(mount_t *mp, void* arg);
static int iso9660_probe( mount_t *mp);
static bool try_mount_cdfs(mount_t *mp);

static int rootdirsz = CDROM_SECTOR_SIZE;
static int rootdirblk = 1;

static int cdfs_release(file_t * fp)
{
	return 0;
}


static const fs_dev_ops_t iso9660_fs=
{
	fs_name: "iso9660fs",
	fs_bmap: iso9660_bmap,
	fs_opendir: iso9660_opendir,
	fs_get_inode_name: iso9660_get_inode_name,

	fs_readdir: iso9660_readdir,
	fs_probe:iso9660_probe,
	fs_mount:iso9660_mount,
	fs_releasefile:cdfs_release,
};
#define ISO_VD_PRIMARY 1
#define ISO_VD_SUPPLEMENTAL 2

static int iso9660_probe(mount_t *mp)
{
	int i;
	iso_volume_descriptor* vd;
	buffer_t *bp;
	struct cdfs __cdfs;
	struct cdfs *cdfs = &__cdfs;
	inode_t *rnd;
	struct iso_directory_record *rec=NULL;
	int size,date;
	int extent;
	int flags;
	int rc;


	if(!mp){
		printk("fsInitialize() mp %x null!\n",mp->m_dev);
		show_superblock();
		 return -1;
	 }

	memset(cdfs,0,sizeof(struct cdfs));

	cdfs->devptr = mp->m_dev;
	cdfs->blks = dev_ioctl(cdfs->devptr, 1,0,NULL,1);

	
    for (i = 16; i < 100; i++)
    {	
      int type;
      unsigned char *esc;

		bp = bread_with_size(cdfs->devptr, i, 2048);
        /* read */

        if (bp == NULL)
        {
			kprintf("iso9660_probe load falied: no bytes read\n");
            //kfree( vd, sizeof( iso_volume_vd));
            return -1;
        }

		//memcpy(vd, bp->b_b_data, CDROM_SECTOR_SIZE);
		vd =  (void *)bp->b_data;
		if (vd == NULL)
		{
			kprintf("iso9660_probe load falied: bread error\n");
			return -1;
		}

		type = isonum_711(vd->type);
		esc = vd->escape_sequences;

		

		if (_strncmp(vd->id, "CD001", 5) != 0)
		{
			buf_release(bp);
		   printk("iso9660_probe no iso_volume_descriptor found\n");
		  return -1;
		}

		if ( type == ISO_VD_PRIMARY){
		  /* read primary vd */
		  //kprintf("ISO_VD_PRIMARY block is %d\n", i);
		   cdfs->vdblk = i;
		   rec = vd->root_directory_record;
		}
		else if (type == ISO_VD_SUPPLEMENTAL && 
				 esc[0] == 0x25 && esc[1] == 0x2F && 
			 (esc[2] == 0x40 || esc[2] == 0x43 || esc[2] == 0x45))
		{
		  cdfs->vdblk = i;
		  //cdfs->joliet = 1;
		  cdfs->joliet = 0;
		  //printk("iso9660_probe we found ISO_VD_SUPPLEMENTAL at blk%d\n", i);
		}

		//buf_release(bp);

        /* end */
        if (type == ISO_END_VOLUME_DESCRIPTOR)
        {
            break;
        }
    }

    /* invalid */
    if (i == 100 || (cdfs->vdblk == 0) )
    {
		kprintf("iso9660_probe load atapi falied: primaryVolumeDescriptor not Found\n");
		kfree( vd);
        return -1;
    }

	mp->m_private_data = kcalloc(sizeof(struct cdfs));
	if(mp->m_private_data)
	  memcpy(mp->m_private_data,&__cdfs,sizeof(__cdfs));
	return 0;
}


static inline int _strncmp (const char *s1, const char *s2, unsigned long len)
  {
    int i = 0;

    if (!s1 || !s2)
	    return 1;  

    while (!(s1[i]==0 && s2[i]==0) && i<len)
      {
	if (s1[i] < s2[i])	return -1;
	if (s1[i] > s2[i])	return 1;
	i++;
      }

    return 0;
}



static bool try_mount_cdfs(mount_t *mp)
{
    int  i;
	iso_volume_descriptor* vd;
	buffer_t *bp;
	struct cdfs *cdfs = mp->m_private_data;
	inode_t *rnd;
	struct iso_directory_record *rec=NULL;
	int size,date;
	int extent;
	int flags;
	int rc;

	if(!mp){
		printk("fsInitialize() mp %x null!\n",mp->m_dev);
		show_superblock();
		 return false;
	 }

	memset(cdfs,0,sizeof(struct cdfs));	

	cdfs->devptr = mp->m_dev;
	cdfs->blks = dev_ioctl(cdfs->devptr, 1,0,NULL,1);

	
    for (i = 16; i < 100; i++)
    {	
      int type;
      unsigned char *esc;

		bp = bread(cdfs->devptr, i);
        /* read */

        if (bp == NULL)
        {
			kprintf("load falied: no bytes read on dev%x\n",cdfs->devptr->devno);
            //kfree( vd, sizeof( iso_volume_vd));
            return false;
        }

		//memcpy(vd, bp->b_b_data, CDROM_SECTOR_SIZE);
		vd =  (void *)bp->b_data;
		if (!vd)
		{
			kprintf("load falied: bread error\n");
			return false;
		}

		type = isonum_711(vd->type);
		esc = vd->escape_sequences;

    if (_strncmp(vd->id, "CD001", 5) != 0)
    {
		buf_release(bp);
       printk("no iso_volume_descriptor found\n");
      return false;
    }

		if ( type == ISO_VD_PRIMARY){
		  /* read primary vd */
		  //kprintf("ISO_VD_PRIMARY block is %d\n", i);
		   cdfs->vdblk = i;
		   rec = vd->root_directory_record;
		}
		else if (type == ISO_VD_SUPPLEMENTAL && 
				 esc[0] == 0x25 && esc[1] == 0x2F && 
			 (esc[2] == 0x40 || esc[2] == 0x43 || esc[2] == 0x45))
		{
		  cdfs->vdblk = i;
		  //cdfs->joliet = 1;
		  cdfs->joliet = 0;
		 // printk("we found ISO_VD_SUPPLEMENTAL at blk%d\n", i);
		}

		buf_release(bp);

        /* end */
        if (type == ISO_END_VOLUME_DESCRIPTOR)
        {
            break;
        }
    }

    /* invalid */
    if (i == 100 || (cdfs->vdblk == 0) )
    {
		//kprintf("load atapi falied: primaryVolumeDescriptor not Found\n");
		kfree( vd);
        return false;
    }



	// Initialize filesystem from selected volume descriptor and read path table
	bp  = bread(cdfs->devptr, cdfs->vdblk);
	if (!bp) return false;

	vd = (struct iso_volume_descriptor *) bp->b_data;

	cdfs->volblks = isonum_733(vd->volume_space_size);

	i = iso9660_read_path_table(cdfs, vd);
	if (i < 0){
		buf_release(bp);
		return false;
	}

	flags = isonum_711(rec->flags);
	extent = isonum_733(rec->extent);
	date = iso9660_isodate(rec->date);
	rootdirsz = isonum_733(rec->size);
	//kprintf("root at %x %d, size is%d\n", mp->m_dev->devno,extent,size);

	kfree( vd);
	rnd = iget(mp->m_dev->devno, extent );

	rnd->i_size = rootdirsz;
	rnd->i_mode = typemode(S_IFDIR, 0666);
	rnd->i_father = extent;

	mp->m_root_ino = extent;
	buf_release(bp);

	if (rnd==NULL)
	{
		kprintf("%s rnd==NULL\n",__FUNCTION__);
		return false;
	}

	mp->m_root =  rnd;
	mp->m_count ++;

	//kprintf("mount on iso9660 fs OK\n");

	return true;
}

void cdfs_hook()
{
	install_fs(&iso9660_fs);
}


static int iso9660_read_path_table(struct cdfs *cdfs, struct iso_volume_descriptor *vd)
{
  buffer_t *buf;
  unsigned char *pt;
  int ptblk;
  int ptlen;
  int ptpos;
  int n;

  // Determine size and location of path table and allocate buffer
  ptlen = isonum_733(vd->path_table_size);
  ptblk = isonum_731(vd->type_l_path_table);
  cdfs->path_table_buffer = kmalloc(ptlen,0);
  if (!cdfs->path_table_buffer) return -2;

  // Read L path table into buffer
  ptpos = 0;
  while (ptpos < ptlen)
  {
    buf = bread(cdfs->devptr, ptblk++);
    if (!buf) return -1;

    if (ptlen - ptpos > CDROM_SECTOR_SIZE)
    {
      memcpy(cdfs->path_table_buffer + ptpos, buf->b_data, CDROM_SECTOR_SIZE);
      ptpos += CDROM_SECTOR_SIZE;
    }
    else
    {
      memcpy(cdfs->path_table_buffer + ptpos, buf->b_data, ptlen - ptpos);
      ptpos = ptlen;
    }

	buf_release(buf);

    //release_buffer(cdfs->devptr, buf);
  }

  // Determine number of records in pathtable
  // Path table records are indexed from 1 (first entry not used)
  pt = cdfs->path_table_buffer;
  n = 1;
  while (pt < cdfs->path_table_buffer + ptlen)
  {
    struct iso_pathtable_record *pathrec = (struct iso_pathtable_record *) pt;
    int namelen = pathrec->length;
    int reclen = sizeof(struct iso_pathtable_record) + namelen + (namelen & 1);

    n++;
    pt += reclen;
  }

  cdfs->path_table_records = n;

  // Allocate path table
  cdfs->path_table = (struct iso_pathtable_record **)
	  kmalloc(cdfs->path_table_records * sizeof(struct iso_pathtable_record **),0);
  if (!cdfs->path_table) return -2;
  cdfs->path_table[0] = NULL;

  // Setup pointers into path table buffer
  pt = cdfs->path_table_buffer;
  for (n = 1; n < cdfs->path_table_records; n++)
  {
    struct iso_pathtable_record *pathrec = (struct iso_pathtable_record *) pt;
    int namelen = pathrec->length;
    int reclen = sizeof(struct iso_pathtable_record) + namelen + (namelen & 1);

    cdfs->path_table[n] = pathrec;
    pt += reclen;
  }

  return 0;
}

static int iso9660_mount(mount_t *mp, void* arg)
{
 	unsigned cur_flag;

	//kprintf("iso9660_mount called\n");

	//save_eflags(&cur_flag);	
	/* initialize ISO9660 FS */
    if (try_mount_cdfs(mp) == false)
    {
		kprintf("iso9660_mount failed\n");
		//restore_eflags(cur_flag);	
        return -1;
    }

	mp->m_magic = ISO9660_MAGIC;
	mp->m_blk_size = 2048;
	//kprintf("iso9660_mount succ\n");
	return 0;
}

  