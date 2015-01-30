

#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>
#include "romfs.h"
#define ROMFSBUF_SZ 1024

/********************************************/

static  fs_dev_ops_t _romfs=
{
	fs_name: "romfs",
	fs_copyright:"BSDL",
	fs_author:"Easion",
	fs_bmap: NULL,
	fs_opendir: romfs_opendir,

	fs_readdir: romfs_readdir,
	fs_probe:romfs_probe,
	fs_mount:romfs_mount,
	fs_unmount:romfs_unmount,
	fs_read:romfs_fread,
	fs_write:NULL,
	fs_mkdir:NULL,
};


void romfs_hook()
{
	install_fs(&_romfs);
}

void remove_romfs_hook()
{
	deinstall_fs(&_romfs);
}


static romdisk_hdr_t *romdisk_hdr = NULL;
static u32_t full_size;



int romfs_probe(const mount_t *mp )
{
	char bootblock[512];
	int blocksize=512;
	int cnt;

	cnt = dev_read( mp->m_dev, 0, ( void * )bootblock, blocksize );

	if (cnt<=0)return -1;

		/* Check and print some info about it */
	romdisk_hdr = (romdisk_hdr_t *)bootblock;
	if (strncmp(bootblock, ROMFS_DISK_MAGIC, 8)) {
		DBGOUT( "Rom disk image is not a ROMFS image %s on fd %x\r\n", bootblock,mp->m_dev->devno);
		return -1;
	}

	//CHECK CRC

	return 0;
}

int romfs_mount(mount_t *mp, void *_data )
{
	int error;
	inode_t *tmp_root_inode;
	char bootblock[512];
	int blocksize=512;
	int cnt;
	u32_t romdisk_files = 0;
	int i,ni;
	romdisk_file_t	*fhdr;

	cnt = dev_read( mp->m_dev, 0, ( void * )bootblock, blocksize );

	if (cnt<=0)return -1;

	/* Check and print some info about it */
	romdisk_hdr = (romdisk_hdr_t *)bootblock;
	if (strncmp(bootblock, ROMFS_DISK_MAGIC, 8)) {
		DBGOUT( "Rom disk image  is not a ROMFS image\r\n");
		return -1;
	}

	full_size = ntohl_32(&romdisk_hdr->full_size);

	DBGOUT( "ROMFS image recognized. Full size is 0x%x bytes\r\n",
		ntohl_32(&romdisk_hdr->full_size));
	DBGOUT( "  Checksum is 0x%x\r\n",ntohl_32(&romdisk_hdr->checksum));
	DBGOUT( "  Volume ID is ``%s''\r\n",romdisk_hdr->volume_name);

	romdisk_files = sizeof(romdisk_hdr_t)
		+ (strlen(romdisk_hdr->volume_name) / 16) * 16;
	DBGOUT( "  File entries begin at offset 0x%x\r\n",
		romdisk_files);

	tmp_root_inode = iget(mp->m_dev->devno, romdisk_files);
	
	if (!tmp_root_inode)
		goto err1;

	mp->m_root_ino=romdisk_files;
	mp->m_root=tmp_root_inode;
	mp->m_count ++;
	mp->m_magic = ROMFS_DISK_MAGIC;
	return 0;

err1:
	return error;
}

long romfs2dostype(int type)
{
	int typevalue = type & 0x03;
	long dostype=0;
#define MSDOS_DIR     16 // Ä¿Â¼ 
#define MSDOS_ARCH    32 // ´æµµÎÄ¼þ 

	switch (typevalue)
	{
	case ROMFS_DIR_TYPE:
		dostype=MSDOS_DIR;
		break;
	case ROMFS_REG_TYPE:
		dostype=MSDOS_ARCH;
		break;
	case ROMFS_SYM_TYPE:
		//dostype=
		break;
	default:
		break;	
	}
	return dostype;
}

long romfs2unixtype(int type)
{
	int typevalue = type & 0x03;
	long unixtype=0;

	switch (typevalue)
	{
	case ROMFS_DIR_TYPE:
		unixtype=S_IFDIR;
		break;
	case ROMFS_REG_TYPE:
		unixtype=S_IFREG;
		break;
	case ROMFS_SYM_TYPE:
		unixtype=S_IFLNK;
		break;
	default:
		break;	
	}
	return unixtype;
}

int romfs_readdir(file_t* filp, vfs_dirent_t *dirent)
{
	inode_t *s_ino=NULL;
	inode_t *dir = filp->f_inode;
	u32_t		i, ni, type;
	romdisk_file_t	*fhdr;
	char tmpbuf[ROMFSBUF_SZ];
	int blk,blk_offset;
	int retval;
	u32_t offset = filp->f_inode->i_number;
	int fd = filp->f_inode->i_dev;
	int pl = filp->f_pos;
	int pos=0;
		
	assert(dir !=NULL);
	memset(dirent, 0, sizeof(vfs_dirent_t));

	if (pl==-1)
		return -1;

	i = offset;
	do {
		blk = i / 512;
		blk_offset = i % 512;
		retval = dev_read(get_dev_desc(fd), blk,tmpbuf,ROMFSBUF_SZ);

		if (retval<=0)
			return 0;

		/* Locate the entry, next pointer, and type info */
		fhdr = (romdisk_file_t *)(tmpbuf + blk_offset);
		ni = ntohl_32(&fhdr->next_header);
		type = ni & 0x0f;
		ni = ni & 0xfffffff0;

		if (ni > full_size)return 0;		

		if(pos==pl){
			strncpy(dirent->l_long_name, fhdr->filename,sizeof(dirent->l_long_name));
			dirent->l_attribute   = romfs2dostype(type);
			dirent->l_ctime  = 0;
			dirent->l_atime  = 0;
			dirent->l_mtime  = 0;
			dirent->l_size_high = 0;
			dirent->l_size_low = ntohl_32(&fhdr->size);
			*(unsigned long*)&dirent->l_res[0] = 0;
			*(unsigned long*)&dirent->l_res[4] = 0;
			filp->f_pos++;
			return 0;
		}
		DBGOUT("romfs_readdir() find %s\n", fhdr->filename);
		i = ni;
		if(i==0)
			filp->f_pos=-1;
		pos ++;

	} while (i != 0);
	return 0;
}

int romfs_unmount(mount_t *m, void *pVolume )
{
	return 0;
}
int romfs_fread(file_t * filp, char * buf, int count)
{
	char tmpbuf[512];
	int blk,blk_offset;
	int retval;
	int bytes;
	u32_t ino = filp->f_inode->i_number;
	int fd = filp->f_inode->i_dev;
	int n=0;


	while (count > 0)
	{
		int i = ino + filp->f_pos;

		blk = i / 512;
		blk_offset = i % 512;

		/* Locate the entry, next pointer, and type info */
		retval = dev_read(get_dev_desc(fd), blk,tmpbuf,512);

		if (retval<=0)
			break;

		bytes = 512-blk_offset;
		if(bytes>count)bytes = count;

		/* Copy out the requested amount */
		memcpy(buf+n, tmpbuf + blk_offset, bytes);
		//DBGOUT("romfs_fread at %x with buf %s\n",i,buf);

		filp->f_pos += bytes;
		n+=bytes;
		count -= bytes;
	}


	return n;
}


/* Mutex for file handles */

/* Given a filename and a starting romdisk directory listing (byte offset),
   search for the entry in the directory and return the byte offset to its
   entry. */
static u32_t romdisk_find_object(const char *filename, int *filetype, inode_t*inode) {
	u32_t		i, ni, type;
	romdisk_file_t	*fhdr;
	char tmpbuf[ROMFSBUF_SZ];
	int blk,blk_offset;
	int retval;
	long offset = inode->i_number;
	int fd = inode->i_dev;

	DBGOUT("romdisk_find_object() try find %s at 0x%x\n", filename, offset);

	i = offset;
	do {
		blk = i / 512;
		blk_offset = i % 512;
		retval = dev_read(get_dev_desc(fd), blk,tmpbuf,ROMFSBUF_SZ);

		if (retval<=0)
			break;

		/* Locate the entry, next pointer, and type info */
		fhdr = (romdisk_file_t *)(tmpbuf + blk_offset);
		ni = ntohl_32(&fhdr->next_header);
		type = ni & 0x0f;
		ni = ni & 0xfffffff0;
		if (ni > full_size)return 0;		

		//DBGOUT("next_header at %x current %s target=%s\n",ni,fhdr->filename,filename);


		/* Check filename */
		if (!strnicmp(fhdr->filename, filename)) {
			if(filetype)*filetype = type;
			return i;
		}
		
		i = ni;
	} while (i != 0);

	/* Didn't find it */
	return 0;
}

inode_t* romfs_opendir(inode_t * inode, char *filepath)
{
	int file_type=0;
	inode_t* child_vnode = NULL;
	mount_t *sp;
	u32_t c_ino;
	u32_t		target_ino;
	romdisk_file_t	*fhdr;
	char tmpbuf[ROMFSBUF_SZ];
	int blk,blk_offset;	
	int retval;
	int fd = inode->i_dev;

	/* No blank filenames */
	if (!filepath[0])
		return 0;

	/* Look for the file */
	target_ino = romdisk_find_object(filepath,  &file_type, inode);
	if (target_ino == 0){
		DBGOUT("romfs_opendir() file %s not found\n",filepath);
		return 0;	
	}

	/* Match: return this index */
	blk = target_ino / 512;
	blk_offset = target_ino % 512;
	retval = dev_read(get_dev_desc(fd), blk,tmpbuf,ROMFSBUF_SZ);

	/* Locate the entry, next pointer, and type info */
	fhdr = (romdisk_file_t *)(tmpbuf + blk_offset);

	file_type=ntohl_32(&fhdr->next_header)&0xf;

	char *data=tmpbuf+blk_offset+sizeof(romdisk_file_t) + (strlen(fhdr->filename)/16)*16;
	DBGOUT("spec at %s addr %x - size %d data=%s\n",fhdr->filename,target_ino,ntohl_32(&fhdr->size),data);

	c_ino = target_ino + sizeof(romdisk_file_t) + (strlen(fhdr->filename)/16)*16;

	assert(inode !=NULL);

	child_vnode = iget(inode->i_dev, c_ino);
	child_vnode->i_size = ntohl_32(&fhdr->size);
	child_vnode->i_mode =romfs2unixtype(file_type);

	child_vnode->i_father = inode->i_number;
	DBGOUT("find child at %x\n", c_ino);
	return child_vnode;
err:
	return NULL;
}

