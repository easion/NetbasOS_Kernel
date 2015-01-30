

#ifndef __ROMFS_H__
#define __ROMFS_H__

/* Header definitions from Linux ROMFS documentation; all integer quantities are
   expressed in big-endian notation. Unfortunately the ROMFS guys were being
   clever and made this header a variable length depending on the size of
   the volume name *groan*. Its size will be a multiple of 16 bytes though. */
typedef struct {
	char	magic[8];		/* Should be "-rom1fs-" */
	u32_t	full_size;		/* Full size of the file system */
	u32_t	checksum;		/* Checksum */
	char	volume_name[16];	/* Volume name (zero-terminated) */
} romdisk_hdr_t;

/* File header info; note that this header plus filename must be a multiple of
   16 bytes, and the following file data must also be a multiple of 16 bytes. */
typedef struct {
	u32_t	next_header;		/* Offset of next header */
	u32_t	spec_info;		/* Spec info */
	u32_t	size;			/* Data size */
	u32_t	checksum;		/* File checksum */
	char	filename[16];		/* File name (zero-terminated) */
} romdisk_file_t;


/* Util function to reverse the byte order of a u32_t */
static inline u32_t ntohl_32(const void *data) {
	const u8_t *d = (const u8_t*)data;
	return (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | (d[3] << 0);
}

enum{
	ROMFS_DIR_TYPE=1,
	ROMFS_REG_TYPE=2,
	ROMFS_SYM_TYPE=3,
};


#define ROMFS_DISK_MAGIC "-rom1fs-"

	
//netbas utils

int romfs_probe(const mount_t *mp );
int romfs_mount(mount_t *mp, void *_data );
int romfs_unmount(mount_t *mp, void *pVolume );
int romfs_fread(file_t * filp, char * buf, int count);
int romfs_fwrite(file_t * filp, char * buf, int count);
inode_t* romfs_opendir(inode_t * inode, char *name);
int romfs_readdir(file_t* filp, vfs_dirent_t *dirent);

#define DBGOUT //kprintf


#endif


