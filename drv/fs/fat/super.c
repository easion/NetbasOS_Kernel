/*
**     (R)Jicama OS
**     Microsoft File Allocation Table
**     Copyright (C) 2003 DengPingPing
*/

#include <drv/drv.h>
#include <drv/fs.h>
#include <assert.h>
#include <drv/log.h>
#include "fat.h"

static	unsigned char tmp_super_buf[HD_SECTOR_SIZE*2];
void msdos_outinfo(struct msdos_info *info);
int msdos_load_fat12(const int dev,int secs);
int msdos_load_fat32(const int dev);
#define MAJOR(dev_nr) (((unsigned)(dev_nr))>>8)

int msdos_dir_size(mount_t *mp, u16_t start);
int fat_bmap(inode_t* inode, int offset, int create);
int fat32_dirsz(u32_t start);
inode_t* msdos_opendir(inode_t* old, unsigned char *name);
inode_t* msdos_get_inode_name(inode_t* old, ino_t ino,unsigned char *name);
int msdos_readdir(file_t *fp, vfs_dirent_t *vp);

static char volume_label [12];
static char fstype [9];
#define FAT_CVT_U16(bytarr) (* (u16_t*)(bytarr))
static int fat_probe(const mount_t *mp );
int fat_mount(mount_t *mp, void *arg);
int fat_write_node(inode_t * inode);
int fat_create (inode_t* n, unsigned char* file);
int fat_unlink (inode_t* n, unsigned char* file);
int fat_mkdir(inode_t* n, char* dname);
int fat_truncate(mount_t *mp, unsigned long cluster );
struct msdos_info *msdos_dev2info(dev_t dev);

int fat_table_sync(mount_t*mp)
{
	switch (mp->m_magic)
	{
	case FAT12_MAGIC:
		//Í¬²½fat±í
		fat12_sync(mp);
		break;
	case FAT16_MAGIC:
		break;
	case FAT32_MAGIC:
		break;	
	default:
		panic("bad magic at hd0\n");
	break;
	}
}

int fat_release(file_t * fp)
{
	mount_t*mp = get_superblock(fp->f_inode->i_dev);


	if (O_WRONLY & fp->f_mode || fp->f_mode & O_RDWR)
	{
		printf("fat_release sync with mode 0x%x\n", fp->f_mode);
		fat_dir_update(fp);
		fat_table_sync(mp);
	}
	return 0;
}

static  fs_dev_ops_t _fatfs=
{
	fs_name: "fatfs",
	fs_bmap: fat_bmap,
	fs_probe:fat_probe,
	fs_mount:fat_mount,
	fs_opendir: msdos_opendir,
	fs_get_inode_name: msdos_get_inode_name,

	fs_readdir: msdos_readdir,
	fs_write_inode:fat_write_node,

	fs_creat:fat_create,
	fs_unlink:fat_unlink,
	fs_mkdir:fat_mkdir,
	fs_truncate:fat_truncate,
	fs_releasefile:fat_release,
};

static int fat_probe(const mount_t *mp )
{
	int ret;
	struct msdos_super* msp=NULL;
	struct fat_1x *fat1x=NULL;
	struct fat_32 *fat3x=NULL;
	dev_prvi_t* dev = mp->m_dev;
	struct msdos_info fat_info;

	//kprintf("fat_probe called\n");

	assert(mp!=NULL);

	memset(tmp_super_buf,0,HD_SECTOR_SIZE);
	memset(&fat_info,0,sizeof(struct msdos_info));
	int readsize =  dev_read(dev, 0, ( void * )tmp_super_buf, HD_SECTOR_SIZE );


	 if(readsize <0){
		//printf("dev_read() at 0x%x failed()!%x\n",mp->m_dev, readsize);
		 return -1;
	 }

    if (((tmp_super_buf[0x1fe] != 0x55) ||(tmp_super_buf[0x1ff] != 0xaa))
		&& (tmp_super_buf[0x15] == 0xf8))
		return -1;

   if (!memcmp(tmp_super_buf+3, "NTFS    ", 8) 
		|| !memcmp(tmp_super_buf+3, "HPFS    ", 8)) {
		printk("%s() %s, not FAT filesystem!\n",__FUNCTION__, tmp_super_buf+3);
		return -1;
    }

	msp= (struct msdos_super*)tmp_super_buf;
	fat1x = (struct fat_1x*)&tmp_super_buf[sizeof(struct msdos_super)];
	fat3x = (struct fat_32*)&tmp_super_buf[sizeof(struct msdos_super)];

	if (!msp->cluster_size)
	{
		return -1;
	}

    if ((tmp_super_buf[0x15] != 0xF0) && (tmp_super_buf[0x15] < 0xf8)) {
		printk("dosfs error: invalid media descriptor byte (%x)\n",tmp_super_buf[0x15]);
		return -1;
    }

	if(memcmp(fat1x->fstype,"FAT12   ",  8) == 0
		|| memcmp(fat1x->fstype, "FAT16   ", 8)==0){
		//printf("fat1x->fstype = %s\n",fat1x->fstype);
		return 0;
	}	
	else if(msp->fat16_length==0){
		printf("msp->fat16_length=0\n");
		return 0;
	}
	else{
		//printf("other\n");
		return -1;
	}

	return -1;
}

struct msdos_info *msdos_dev2info(dev_t dev)
{
	struct msdos_info *fat_info_ptr = NULL;

	mount_t *mp = get_superblock(dev);

	if (!mp)
	{
		return NULL;
	}
	return mp->m_private_data;
}

inline int memcmp(const void *left_p, const void *right_p, size_t count)
{
	const unsigned char *left = (const unsigned char *)left_p;
	const unsigned char *right = (const unsigned char *)right_p;

	for(; count != 0; count--)
	{
		if(*left != *right)
			return *left -  *right;
		left++;
		right++;
	}
	return 0;
}


int fat_mount(mount_t *mp, void *arg)
{
	inode_t *tmp_root_inode;
	struct msdos_super* msp;
	struct fat_1x *fat1x;
	struct fat_32 *fat3x;
	struct msdos_info *fat_info_ptr;

	assert(mp!=NULL);

	 fat_info_ptr = kmalloc(sizeof(struct msdos_info),0);
	 if (fat_info_ptr == NULL)
	 {
		 printf("error fat_info_ptr\n");
		 return -1;
	 }

	mp->m_private_data = fat_info_ptr;
	memset(tmp_super_buf,0,HD_SECTOR_SIZE);
	memset(fat_info_ptr,0,sizeof(struct msdos_info));
	int readsize = dev_read( mp->m_dev, 0, ( void * )tmp_super_buf, HD_SECTOR_SIZE );

	 if(readsize <0){
		syslog(4,"dev_read() at 0x%x failed()!%x\n",mp->m_dev, readsize);
		 return -1;
	 }

	msp=fat_info_ptr->dos_sp = (struct msdos_super*)tmp_super_buf;
	fat1x = (struct fat_1x*)&tmp_super_buf[sizeof(struct msdos_super)];
	fat3x = (struct fat_32*)&tmp_super_buf[sizeof(struct msdos_super)];

	if (!msp->cluster_size)
	{
		panic("bad msp->cluster_size");
		return -1;
	}
	
	fat_info_ptr->blk_size = msp->cluster_size*HD_SECTOR_SIZE;   //every block size.
	//printf("cluster size:%d\n",msp->cluster_size);
	//printf("oem:%s\n",msp->oem);
	fat_info_ptr->cluster_size = msp->cluster_size;  //cluster size
	
	
	//printf("total sectors:%d\n",msp->total_sectors);	
	//printf("Sectors:%d\n", msp->sectors[0]+(msp->sectors[1]<<8));

	fat_info_ptr->sector_size = FAT_CVT_U16(msp->sector_size);
	fat_info_ptr->dir_entries = FAT_CVT_U16(msp->dir_entries);

    /* for fat16 and fat32*/
	///Fat Base: Begin Sectors add reserved
	//printf("dir entries:%d\n",fat_info_ptr->dir_entries);


  	fat_info_ptr->fat_base =  (msp->reserved );  //Value is sector
	//printf("reserved:%d - base %d msp->hidden %d\n",	msp->reserved, fat_info_ptr->fat_base, msp->hidden);
	  

     if (msp->fat16_length != 0)
	{
		fat_info_ptr->fat_size = msp->fat16_length;	
		fat_info_ptr->fat_root =  msp->reserved+ (msp->fats*msp->fat16_length);  //Value is sector

		memcpy(volume_label, fat1x->volume_label,11);

		//printf("fstype:%s\n",fat1x->fstype);
		//printf("volume_id:%x\n",fat1x->volume_id);
		if(memcmp(fat1x->fstype,"FAT12   ",  8) == 0){
			fat_info_ptr->fat_entries = msp->fat16_length*HD_SECTOR_SIZE*3/(sizeof(unsigned short));
			mp->m_magic = FAT12_MAGIC;
			fat_info_ptr->blk_base = fat_info_ptr->fat_root + (fat_info_ptr->dir_entries*32)/HD_SECTOR_SIZE;
		}
		else if(memcmp(fat1x->fstype, "FAT16   ", 8) == 0){
			fat_info_ptr->fat_entries = msp->fat16_length*HD_SECTOR_SIZE/(sizeof(unsigned short));
			mp->m_magic = FAT16_MAGIC;
			fat_info_ptr->blk_base = fat_info_ptr->fat_root +  (fat_info_ptr->dir_entries*32)/HD_SECTOR_SIZE;
		}
		else{
			panic("bad fstype");
		}

		syslog(4, "fat_info_ptr->fat_entries  is %x msp->fat16_length:%d root at %d\n",
			fat_info_ptr->fat_entries, msp->fat16_length, fat_info_ptr->fat_root);
		
		tmp_root_inode=iget(mp->m_dev->devno, FAT12_ROOT);
		mp->m_root_ino = FAT12_ROOT;
	 }
	  else{
		//printf("volume_label32:%s\n",fat3x->volume_label);
		fat_info_ptr->fat_size = fat3x->fat_size;
		fat_info_ptr->fat_entries = ((int)fat3x->fat_size*HD_SECTOR_SIZE) / sizeof(unsigned long);
		
		fat_info_ptr->fat_root =  fat3x->fat32_root_cluster;  //But this value is cluster.
		fat_info_ptr->blk_base = msp->reserved + ((int)msp->fats * (int)fat3x->fat_size);
		mp->m_root_ino=fat_info_ptr->fat_root ;
		
		mp->m_magic = FAT32_MAGIC;
		tmp_root_inode=iget(mp->m_dev->devno, fat_info_ptr->fat_root);
		//printf("fat32 root:%d fatlen %d -%d\n",	fat_info_ptr->fat_root,(int)msp->fats , (int)fat3x->fat_size);
		memcpy(volume_label, fat3x->volume_label,11);
		}


	volume_label[11] = 0;
	printf("volume Label %s\n", volume_label);


	if(mp->m_magic == FAT12_MAGIC){			
			//printf("try loading fat12 system fat table\n");
			if(msdos_load_fat12(mp->m_dev,fat_info_ptr->fat_size))
				return -1;
	}
	else if(mp->m_magic == FAT16_MAGIC){
		syslog(4,"loading fat16 system ok\n");
	}else if(mp->m_magic == FAT32_MAGIC){
		syslog(4, "loading fat32 system ok\n");
	}

	if (tmp_root_inode==NULL){
		panic("tmp_root_inode==NULL");
	}

	mp->m_root =tmp_root_inode;
	mp->m_count ++;
	tmp_root_inode->i_size = msdos_dir_size(mp, fat_info_ptr->fat_root);
	mp->m_blk_size = 512;
	//panic("mount fat done");
	return OK;
}

void fat_hook()
{
	install_fs(&_fatfs);
}

void msdos_outinfo(struct msdos_info *info)
{
	printf(" fat_info_ptr->fat_entries:%d\n", info->fat_entries);
	printf ("Sector per cluster:%u \n", info->cluster_size );
	
	printf("Dir Entries:%i\n",  info->dir_entries);
	
	printf("Fat Length:%u\n",	 info->fat_size); 
	printf("Root sector:%d\nBlock begin:%u\n",
		info->fat_root,info->blk_base);
 }
