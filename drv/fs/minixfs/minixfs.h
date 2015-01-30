

#ifndef __ROMFS_H__
#define __ROMFS_H__
#define RWX_MODES       0000777	/* mode bits for RWX only */

typedef unsigned long	zone_t;
typedef unsigned short	zone1_t;

typedef char		nlink_t;

typedef unsigned short bitchunk_t; /* collection of bits in a bitmap */
 typedef unsigned long  bit_t;      /* bit number in a bit map */


//typedef short		uid_t;
//typedef char		gid_t; /* XXX: 'short' in newlib */

//typedef unsigned short	ino_t;
//typedef unsigned short	mode_t; /* XXX: 'int' in newlib */ 

//typedef long		off_t;
//typedef long		time_t;
#define V2_BLOCK_SIZE		1024


/* Tables sizes */
#define V1_NR_DZONES       7	/* # direct zone numbers in a V1 inode */
#define V1_NR_TZONES       9	/* total # zone numbers in a V1 inode */
#define V2_NR_DZONES       7	/* # direct zone numbers in a V2 inode */
#define V2_NR_TZONES      10	/* total # zone numbers in a V2 inode */

#define NR_FILPS         128	/* # slots in filp table */
#define NR_INODES         64	/* # slots in "in core" inode table */
#define NR_SUPERS          8	/* # slots in super block table */
#define NR_LOCKS           8	/* # slots in the file locking table */

/* The type of sizeof may be (unsigned) long.  Use the following macro for
 * taking the sizes of small objects so that there are no surprises like
 * (small) long constants being passed to routines expecting an int.
 */
#define usizeof(t) ((unsigned) sizeof(t))

/* File system types. */
#define SUPER_MAGIC   0x137F	/* magic number contained in super-block */
#define SUPER_REV     0x7F13	/* magic # when 68000 disk read on PC or vv */
#define SUPER_V2      0x2468	/* magic # for V2 file systems */
#define SUPER_V2_REV  0x6824	/* V2 magic written on PC, read on 68K or vv */

//#define V1		   1	/* version number of V1 file systems */ 
//#define V2		   2	/* version number of V2 file systems */ 

/* Miscellaneous constants */
#define SU_UID 	 ((uid_t) 0)	/* super_user's uid_t */
#define SYS_UID  ((uid_t) 0)	/* uid_t for processes MM and INIT */
#define SYS_GID  ((gid_t) 0)	/* gid_t for processes MM and INIT */
#define NORMAL	           0	/* forces get_block to do disk read */
#define NO_READ            1	/* prevents get_block from doing disk read */
#define PREFETCH           2	/* tells get_block not to read or mark dev */

#define XPIPE  (-NR_TASKS-1)	/* used in fp_task when susp'd on pipe */
#define XLOCK  (-NR_TASKS-2)	/* used in fp_task when susp'd on lock */
#define XPOPEN (-NR_TASKS-3)	/* used in fp_task when susp'd on pipe open */

#define NO_BIT   ((bit_t) 0)	/* returned by alloc_bit() to signal failure */

#define DUP_MASK        0100	/* mask to distinguish dup2 from dup */

#define LOOK_UP            0	/* tells search_dir to lookup string */
#define ENTER              1	/* tells search_dir to make dir entry */
#define DELETE             2	/* tells search_dir to delete entry */
#define IS_EMPTY           3	/* tells search_dir to ret. OK or ENOTEMPTY */  

#define CLEAN              0	/* disk and memory copies identical */
#define DIRTY              1	/* disk and memory copies differ */
#define ATIME            002	/* set if atime field needs updating */
#define CTIME            004	/* set if ctime field needs updating */
#define MTIME            010	/* set if mtime field needs updating */

#define BYTE_SWAP          0	/* tells conv2/conv4 to swap bytes */
#define DONT_SWAP          1	/* tells conv2/conv4 not to swap bytes */

#define END_OF_FILE   (-104)	/* eof detected */

//#define DEV_RAM	((dev_t) 0x100)	/* device number of /dev/ram */

#define ROOT_INODE         1	/* inode number for root directory */
#define BOOT_BLOCK  ((block_t) 0)	/* block number of boot block */
#define SUPER_BLOCK ((block_t) 1)	/* block number of super block */

#define DIR_ENTRY_SIZE       usizeof (mfs_direct_t)  /* # bytes/dir entry   */
#define NR_DIR_ENTRIES   (V2_BLOCK_SIZE/DIR_ENTRY_SIZE)  /* # dir entries/blk   */
#define SUPER_SIZE      usizeof (mfs_sb_t)  /* super_block size    */
#define PIPE_SIZE          (V1_NR_DZONES*V2_BLOCK_SIZE)  /* pipe size in bytes  */
#define BITMAP_CHUNKS (V2_BLOCK_SIZE/usizeof (bitchunk_t))/* # map chunks/blk   */

/* Derived sizes pertaining to the V1 file system. */
#define V1_ZONE_NUM_SIZE           usizeof (zone1_t)  /* # bytes in V1 zone  */
#define V1_INODE_SIZE             usizeof (d1_inode)  /* bytes in V1 dsk ino */
#define V1_INDIRECTS   (V2_BLOCK_SIZE/V1_ZONE_NUM_SIZE)  /* # zones/indir block */
#define V1_INODES_PER_BLOCK (V2_BLOCK_SIZE/V1_INODE_SIZE)/* # V1 dsk inodes/blk */

/* Derived sizes pertaining to the V2 file system. */
#define V2_ZONE_NUM_SIZE            usizeof (zone_t)  /* # bytes in V2 zone  */
#define V2_INODE_SIZE             usizeof (mfs2_inode_t)  /* bytes in V2 dsk ino */
#define V2_INDIRECTS   (V2_BLOCK_SIZE/V2_ZONE_NUM_SIZE)  /* # zones/indir block */
#define V2_INODES_PER_BLOCK (V2_BLOCK_SIZE/V2_INODE_SIZE)/* # V2 dsk inodes/blk */


#define INODES_PER_BLOCK V2_INODES_PER_BLOCK

#define INODE_BLOCK        0				 /* inode block */
#define DIRECTORY_BLOCK    1				 /* directory block */
#define INDIRECT_BLOCK     2				 /* pointer block */
#define MAP_BLOCK          3				 /* bit map */
#define ZUPER_BLOCK       (4 + WRITE_IMMED + ONE_SHOT)	 /* super block */
#define FULL_DATA_BLOCK    5		 	 	 /* data, fully used */
#define PARTIAL_DATA_BLOCK 6 				 /* data, partly used*/

typedef unsigned long block_t;

#define NOZONE		-1
#define NOBLOCK		-1


/* Declaration of the V2 inode as it is on the disk (not in core). */
typedef struct {		/* V2.x disk inode */
  mode_t i_mode;		/* file type, protection, etc. */
  u16_t i_nlinks;		/* how many links to this file. HACK! */
  uid_t i_uid;			/* user id of the file's owner. */
  u16_t i_gid;			/* group number HACK! */
  off_t i_size;		/* current file size in bytes */
  time_t i_atime;		/* when was file data last accessed */
  time_t i_mtime;		/* when was file data last changed */
  time_t i_ctime;		/* when was inode data last changed */
  zone_t i_zone[V2_NR_TZONES];	/* block nums for direct, ind, and dbl ind */
} mfs2_inode_t;

typedef struct {		/* V2.x disk inode */
  //mode_t i_mode;		/* file type, protection, etc. */
  u16_t i_nlinks;		/* how many links to this file. HACK! */
 // uid_t i_uid;			/* user id of the file's owner. */
  u16_t i_gid;			/* group number HACK! */
  //off_t i_size;		/* current file size in bytes */
 // time_t i_atime;		/* when was file data last accessed */
  //time_t i_mtime;		/* when was file data last changed */
  //time_t i_ctime;		/* when was inode data last changed */
  zone_t i_zone[V2_NR_TZONES];	/* block nums for direct, ind, and dbl ind */
  int i_ndzones;		/* # direct zones (Vx_NR_DZONES) */
  int i_nindirs;		/* # indirect zones per indirect block */

} mfs2_inode_mem_t;

#define BLOCK_TO_SECTOR(n) (n*2)


typedef struct  {
  ino_t s_ninodes;		/* # usable inodes on the minor device */
  zone1_t  s_nzones;		/* total device size, including bit maps etc */
  short s_imap_blocks;		/* # of blocks used by inode bit map */
  short s_zmap_blocks;		/* # of blocks used by zone bit map */
  zone1_t s_firstdatazone;	/* number of first data zone */
  short s_log_zone_size;	/* log2 of blocks/zone */
  off_t s_max_size;		/* maximum file size on this device */
  short s_magic;		/* magic number to recognize super-blocks */
  short s_pad;			/* try to avoid compiler-dependent padding */
  zone_t s_zones;		/* number of zones (replaces s_nzones in V2) */

#if 0
  /* The following items are only used when the super_block is in memory. */
  struct inode *s_isup;		/* inode for root dir of mounted file sys */
  struct inode *s_imount;	/* inode mounted on */
  unsigned s_inodes_per_block;	/* precalculated from magic number */
  dev_t s_dev;			/* whose super block is this? */
  int s_rd_only;		/* set to 1 iff file sys mounted read only */
  int s_native;			/* set to 1 iff not byte swapped file system */
  int s_version;		/* file system version, zero means bad magic */
#endif
  int s_ndzones;		/* # direct zones in an inode */
  int s_nindirs;		/* # indirect zones per indirect block */
  bit_t s_isearch;		/* inodes below this bit number are in use */
  bit_t s_zsearch;		/* all zones below this bit number are in use*/
} mfs_sb_t;

#define NIL_SUPER (mfs_sb_t *) 0
#define IMAP		0	/* operating on the inode bit map */
#define ZMAP		1	/* operating on the zone bit map */
#define NAME_MAX	30	/* for minix filesystem */
typedef struct {
	ino_t d_ino;
	char d_name[NAME_MAX];
}mfs_direct_t;
#define BLOCKSIZ	0x400
	
//netbas utils

int minixfs_probe(const mount_t *mp );
int minixfs_mount(mount_t *mp, void *_data );
int minixfs_unmount(mount_t *mp, void *pVolume );
int minixfs_fread(file_t * filp, char * buf, int count);
int minixfs_fwrite(file_t * filp, char * buf, int count);
inode_t* minixfs_opendir(inode_t * inode, char *name);
int minixfs_readdir(file_t* filp, vfs_dirent_t *dirent);
int minixfs_readlink(inode_t * inoptr, char *buff, size_t buf_size);

void free_zone(mount_t*dev, zone_t numb);
zone_t alloc_zone(mount_t *dev, zone_t z);
int write_map(inode_t * inoptr, off_t position, zone_t new_zone);
void clear_zone(inode_t * inoptr, off_t pos, int flag);
block_t read_map(inode_t *inoptr , off_t position);
inode_t *alloc_inode(mount_t *dev,mode_t  bits);
void mfs_truncate(inode_t *inoptr);
buffer_t *new_block(inode_t *inode, off_t position);
bit_t alloc_bit(mount_t *mp, int map, bit_t origin);
void free_bit(mount_t *mp, int map, bit_t bit_returned);

#define DBGOUT kprintf
extern char dot1[2];   /* dot1 (&dot1[0]) and dot2 (&dot2[0]) have a special */
extern char dot2[3];   /* meaning to search_dir: no access permission check. */

#endif


