/**
 *
 * Copyright (c) 2006 Jarek Pelczar <jpelczar@gmail.com>
 * Copyright (c) 2001-2005 Anton Altaparmakov
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __unixutc__NTFS_H__
#define __unixutc__NTFS_H__

#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/errno.h>
#include <linux/module.h>
//#include <assert.h>
#define MS_RDONLY	 1	/* Mount read-only */
#define NTFS_FD(vol)		((vol)->u.fd)

typedef long long int64;
typedef long  int32;
typedef short  int16;
typedef char  int8;
typedef unsigned int  uint;

typedef unsigned long long uint64;
typedef unsigned long  uint32;
typedef unsigned short  uint16;
typedef unsigned char  uint8;
typedef unsigned short  kdev_t;
typedef void *sem_id;
typedef  long bigtime_t;

typedef uint8 wchar_t;
typedef uint8 ntfs_u8 ;
typedef uint16 ntfs_u16 ;
typedef uint32 ntfs_u32 ;
typedef int32 ntfs_s32 ;
typedef uint64 ntfs_u64 ;
typedef uint64 ntfs_time64_t;
typedef int64 ntfs_s64 ;
typedef off_t ntfs_cluster_t;
typedef size_t ntfs_size_t;
typedef struct _ntfs_inode ntfs_inode;

static inline void lock_kernel (void)
{
}

static inline void unlock_kernel (void)
{
}

#define BUG() panic("bug @ %s\n", __FUNCTION__)

#define kerndbg(N, format, argc...) kprintf(format, ##argc)

#define ntfs_malloc(a)	kmalloc(a,0)
#define ntfs_free(a)	kfree(a)

#define ntfs_vmalloc(a)	kmalloc(a,0)
#define ntfs_vfree(a)	kfree(a)

#define ntfs_memcpy(a,b,c) memcpy(a,b,c)
#define ntfs_memmove(a,b,c) memmove(a,b,c)
#define ntfs_error(x...) 	kerndbg( KERN_WARNING, "ntfs error : " x)
#if 0
#define ntfs_debug(format, argc...)  kprintf(format, ##argc)
#else
#define ntfs_debug(x...)	//kerndbg( KERN_DEBUG, "ntfs debug : " x)
#endif
#define ntfs_bzero(a,b)		memset(a,0,b)

#define CPU_TO_LE16(a) ((uint16_t)(a))
#define CPU_TO_LE32(a) ((uint32_t)(a))
#define CPU_TO_LE64(a) ((uint64_t)(a))

#define LE16_TO_CPU(a) ((uint16_t)(a))
#define LE32_TO_CPU(a) ((uint32_t)(a))
#define LE64_TO_CPU(a) ((uint64_t)(a))

#define NTFS_GETU8(p)      (*(uint8*)(p))
#define NTFS_GETU16(p)     ((uint16)LE16_TO_CPU(*(uint16*)(p)))
#define NTFS_GETU24(p)     ((uint32)NTFS_GETU16(p) | \
			   ((uint32)NTFS_GETU8(((char*)(p)) + 2) << 16))
#define NTFS_GETU32(p)     ((uint32)LE32_TO_CPU(*(uint32*)(p)))
#define NTFS_GETU40(p)     ((uint64)NTFS_GETU32(p) | \
			   (((uint64)NTFS_GETU8(((char*)(p)) + 4)) << 32))
#define NTFS_GETU48(p)     ((uint64)NTFS_GETU32(p) | \
			   (((uint64)NTFS_GETU16(((char*)(p)) + 4)) << 32))
#define NTFS_GETU56(p)     ((uint64)NTFS_GETU32(p) | \
			   (((uint64)NTFS_GETU24(((char*)(p)) + 4)) << 32))
#define NTFS_GETU64(p)     ((uint64)LE64_TO_CPU(*(uint64*)(p)))

 /* Macros writing unsigned integers */
#define NTFS_PUTU8(p,v)      ((*(uint8*)(p)) = (v))
#define NTFS_PUTU16(p,v)     ((*(uint16*)(p)) = CPU_TO_LE16(v))
#define NTFS_PUTU24(p,v)     NTFS_PUTU16(p, (v) & 0xFFFF);\
                             NTFS_PUTU8(((char*)(p)) + 2, (v) >> 16)
#define NTFS_PUTU32(p,v)     ((*(uint32*)(p)) = CPU_TO_LE32(v))
#define NTFS_PUTU64(p,v)     ((*(uint64*)(p)) = CPU_TO_LE64(v))

 /* Macros reading signed integers */
#define NTFS_GETS8(p)        ((*(int8*)(p)))
#define NTFS_GETS16(p)       ((int16)LE16_TO_CPU(*(short*)(p)))
#define NTFS_GETS24(p)       (NTFS_GETU24(p) < 0x800000 ? \
					(int)NTFS_GETU24(p) : \
					(int)(NTFS_GETU24(p) - 0x1000000))
#define NTFS_GETS32(p)       ((int32)LE32_TO_CPU(*(int*)(p)))
#define NTFS_GETS40(p)       (((int64)NTFS_GETU32(p)) | \
			     (((int64)NTFS_GETS8(((char*)(p)) + 4)) << 32))
#define NTFS_GETS48(p)       (((int64)NTFS_GETU32(p)) | \
			     (((int64)NTFS_GETS16(((char*)(p)) + 4)) << 32))
#define NTFS_GETS56(p)       (((int64)NTFS_GETU32(p)) | \
			     (((int64)NTFS_GETS24(((char*)(p)) + 4)) << 32))
#define NTFS_GETS64(p)	     ((int64)NTFS_GETU64(p))

#define NTFS_PUTS8(p,v)      NTFS_PUTU8(p,v)
#define NTFS_PUTS16(p,v)     NTFS_PUTU16(p,v)
#define NTFS_PUTS24(p,v)     NTFS_PUTU24(p,v)
#define NTFS_PUTS32(p,v)     NTFS_PUTU32(p,v)
#define NTFS_PUTS64(p,v)     NTFS_PUTU64(p,v)


#define IS_MAGIC(a,b)		(*(int*)(a) == *(int*)(b))
#define IS_MFT_RECORD(a)	IS_MAGIC((a),"FILE")
#define IS_INDEX_RECORD(a)	IS_MAGIC((a),"INDX")

/* 'NTFS' in little endian */
#define NTFS_SUPER_MAGIC	0x5346544E

#define NTFS_AFLAG_RO           1
#define NTFS_AFLAG_HIDDEN       2
#define NTFS_AFLAG_SYSTEM       4
#define NTFS_AFLAG_ARCHIVE      20
#define NTFS_AFLAG_COMPRESSED   0x800
#define NTFS_AFLAG_DIR          0x10000000

#include <i386/bitops.h>

#define NTFS_PUTINUM(p,i)    NTFS_PUTU64(p, i->i_ino); \
                             NTFS_PUTU16(((char*)p) + 6, i->sequence_number)

typedef enum
{
	FILE_Mft = 0,
	FILE_MftMirr = 1,
	FILE_LogFile = 2,
	FILE_Volume = 3,
	FILE_AttrDef = 4,
	FILE_root = 5,
	FILE_BitMap = 6,
	FILE_Boot = 7,
	FILE_BadClus = 8,
	FILE_Secure = 9,
	FILE_UpCase = 10,
	FILE_Extend = 11,
	FILE_Reserved12 = 12,
	FILE_Reserved13 = 13,
	FILE_Reserved14 = 14,
	FILE_Reserved15 = 15,
} NTFS_SYSTEM_FILES;

typedef struct
{
	//kdev_t id;
	dev_prvi_t* fd;
	uid_t uid;
	gid_t gid;
	mode_t umask;
	int nls_map;
	uint ngt;
	int mft_zone_multiplier;
	off_t mft_zone_pos;
	off_t mft_zone_start;
	off_t mft_zone_end;
	off_t data1_zone_pos;
	off_t data2_zone_pos;
	off_t mft_data_pos;
	off_t partition_bias;
	uint32 at_standard_information;
	uint32 at_attribute_list;
	uint32 at_file_name;
	uint32 at_volume_version;
	uint32 at_security_descriptor;
	uint32 at_volume_name;
	uint32 at_volume_information;
	uint32 at_data;
	uint32 at_index_root;
	uint32 at_index_allocation;
	uint32 at_bitmap;
	uint32 at_symlink;	/* aka SYMBOLIC_LINK or REPARSE_POINT */
	int sector_size;
	int cluster_size;
	int cluster_size_bits;
	int mft_clusters_per_record;
	int mft_record_size;
	int mft_record_size_bits;
	int index_clusters_per_record;
	int index_record_size;
	int index_record_size_bits;
	off_t nr_clusters;
	off_t mft_lcn;
	off_t mft_mirr_lcn;
	unsigned char *mft;
	unsigned short *upcase;
	unsigned int upcase_length;
	ntfs_inode *mft_ino;
	ntfs_inode *mftmirr;
	ntfs_inode *bitmap;
	uint8 ino_flags;
	sem_id lock;
	char volumeLabel[64];
	long s_flags; //easion add
} ntfs_volume;

#define NTFS_SECTOR_BITS 9
#define NTFS_SECTOR_SIZE HD_SECTOR_SIZE

/*
 * Attribute flags (16-bit).
 */
typedef enum
{
	ATTR_IS_COMPRESSED = ( 0x0001 ),
	ATTR_COMPRESSION_MASK = ( 0x00ff ),
	/* Compression method mask. Also,
	 * first illegal value. */
	ATTR_IS_ENCRYPTED = ( 0x4000 ),
	ATTR_IS_SPARSE = ( 0x8000 ),
} __attribute__ ( ( __packed__ ) ) ATTR_FLAGS;

/*
 * The two zones from which to allocate clusters.
 */
typedef enum
{
	MFT_ZONE,
	DATA_ZONE
} NTFS_CLUSTER_ALLOCATION_ZONES;



struct _ntfs_inode
{
	ino_t i_ino;
	off_t i_size;
	mode_t i_mode;
	ntfs_volume *vol;
	uint16 sequence_number;
	uint8 *attr;
	int attr_count;
	struct ntfs_attribute *attrs;
	int record_count;
	int *records;
	uid_t i_uid;
	gid_t i_gid;
	bigtime_t atime, ctime, mtime;
	unsigned long i_number;
	union
	{
		struct
		{
			int recordsize;
			int clusters_per_record;
		} index;
	} u;
};

/* Which files should be returned from a director listing. */
#define ngt_dos   1		/* only short names, no system files */
#define ngt_nt    2		/* only long names, all-uppercase becomes 
						   all-lowercase, no system files */
#define ngt_posix 3		/* all names except system files */
#define ngt_full  4		/* all entries */

typedef struct
{
	off_t lcn;
	off_t len;
} ntfs_runlist;

typedef struct ntfs_io
{
	int do_read;
	void ( *fn_put ) ( struct ntfs_io * dest, void *buf, size_t );
	void ( *fn_get ) ( void *buf, struct ntfs_io * src, size_t len );
	void *param;
	unsigned long size;
} ntfs_io;

typedef struct ntfs_attribute
{
	int type;
	uint16 *name;
	int namelen;
	int attrno;
	int64 size, allocated, initialized, compsize;
	ATTR_FLAGS flags;
	uint8 resident, indexed;
	int cengine;
	union
	{
		void *data;	/* if resident */
		struct
		{
			ntfs_runlist *runlist;
			unsigned long len;
		} r;
	} d;
} ntfs_attribute;

//#include <attr.h>
//#include <unistr.h>

/* dir.h */
#define ITERATE_SPLIT_DONE      1

enum ntfs_iterate_e
{
	BY_POSITION,
	BY_NAME,
	DIR_INSERT
};

typedef struct ntfs_iterate_s
{
	enum ntfs_iterate_e type;
	ntfs_inode *dir;
	union
	{
		off_t pos;
		int flags;
	} u;
	char *result;		/* pointer to entry if found */
	uint16 *name;
	int namelen;
	int block;		/* current index record */
	int newblock;		/* index record created in a split */
	char *new_entry;
	int new_entry_size;
} ntfs_iterate_s;

static __inline__ int ntfs_test_and_set_bit( unsigned char *byte, const int bit )
{
	unsigned char *ptr = byte +( bit >> 3 );
	int b = 1 << ( bit & 7 );
	int oldbit = *ptr & b ? 1 : 0;

	*ptr |= b;
	return oldbit;
}


struct ntfs_filldir
{
	uint32 pos;
	ntfs_inode *dir;
	int type;
	uint32 ph, pl;
	char *name;
	int namelen;
	int ret_code;
	int count;
	struct vfs_dirent_t *dirent;
	int (*filldir)(vfs_dirent_t *cookie,
	const char *name,  int namelen,vfs_dirent_t* );
};

int ntfs_node_makesure(ntfs_volume *vol, inode_t *dir);


int ntfs_encodeuni(ntfs_volume *vol, ntfs_u16 *in, int in_len, char **out,
		int *out_len);
int ntfs_decodeuni(ntfs_volume *vol, char *in, int in_len, ntfs_u16 **out,
		int *out_len);
void ntfs_put( ntfs_io * dest, void *src, size_t n );
void ntfs_get( void *dest, ntfs_io * src, size_t n );
void *ntfs_calloc( int size );
void ntfs_ascii2uni( short int *to, char *from, int len );
int ntfs_uni_strncmp( short int *a, short int *b, int n );
int ntfs_ua_strncmp( short int *a, char *b, int n );
bigtime_t ntfs_ntutc2unixutc( bigtime_t ntutc );
bigtime_t ntfs_unixutc2ntutc( bigtime_t t );
void ntfs_indexname( char *buf, int type );
int ntfs_readdir2(file_t* filp, vfs_dirent_t *dirent);

ntfs_attribute *ntfs_find_attr( ntfs_inode * ino, int type, char *name );
int ntfs_init_inode( ntfs_inode * ino, ntfs_volume * vol, ino_t innum );
int ntfs_allocate_attr_number( ntfs_inode * ino, int *result );
char *ntfs_get_attr( ntfs_inode * ino, int attr, char *name );
int64 ntfs_get_attr_size( ntfs_inode * ino, int type, char *name );
int ntfs_attr_is_resident( ntfs_inode * ino, int type, char *name );
int ntfs_decompress_run( unsigned char **data, int *length, off_t *cluster, int *ctype );
int ntfs_readwrite_attr( ntfs_inode * ino, ntfs_attribute * attr, int64 offset, ntfs_io * dest );
int ntfs_read_attr( ntfs_inode * ino, int type, char *name, int64 offset, ntfs_io * buf );
int ntfs_write_attr( ntfs_inode * ino, int type, char *name, int64 offset, ntfs_io * buf );
int splice_runlists( ntfs_runlist ** rl1, int *r1len, const ntfs_runlist * rl2, int r2len );
void ntfs_clear_inode( ntfs_inode * ino );

int ntfs_read_mft_record( ntfs_volume * vol, int mftno, char *buf );
int ntfs_getput_clusters( ntfs_volume * vol, int cluster, size_t start_offs, ntfs_io * buf );
bigtime_t ntfs_now( void );

int ntfs_init_attrdef( ntfs_inode * attrdef );
int ntfs_get_version( ntfs_inode * volume );
int ntfs_load_special_files( ntfs_volume * vol );
int ntfs_allocate_attr_number( ntfs_inode * ino, int *result );
int ntfs_get_volumesize( ntfs_volume * vol, int64 *vol_size );
int ntfs_get_free_cluster_count( ntfs_inode * bitmap );

int ntfs_allocate_clusters( ntfs_volume * vol, off_t *location, off_t *count, ntfs_runlist ** rl, int *rl_len, const NTFS_CLUSTER_ALLOCATION_ZONES zone );
int ntfs_deallocate_cluster_run( const ntfs_volume * vol, const off_t lcn, const off_t len );
int ntfs_deallocate_clusters( const ntfs_volume * vol, const ntfs_runlist * rl, const int rl_len );
int ntfs_read_inode( void *_vol, ino_t inid, void **_node );

int ntfs_fixup_record( char *record, char *magic, int size );
int ntfs_check_mft_record( ntfs_volume * vol, char *record );
int ntfs_release_volume( ntfs_volume * vol );
int ntfs_insert_fixups( unsigned char *rec, int rec_size );

int ntfs_update_inode( ntfs_inode * ino );
void ntfs_decompress( unsigned char *dest, unsigned char *src, size_t l );

int ntfs_split_indexroot( ntfs_inode * ino );
int ntfs_getdir_byname( ntfs_iterate_s * walk );
int ntfs_getdir_unsorted( ntfs_inode * ino, uint32 *p_high, uint32 *p_low, int ( *cb ) ( uint8 *, void * ), void *param );

#define ntfs_where()  //printk("%s(): line %d ok\n", __FUNCTION__, __LINE__)
#define ntfs_epoint()  printk("Error: at  file %s %s(): line %d.\n", __FILE__, __FUNCTION__, __LINE__)



//netbas utils

int ntfs_probe(const mount_t *mp );
int ntfs_mount(mount_t *mp, void *_data );
int ntfs_unmount(mount_t *mp, void *pVolume );
int ntfs_fread(file_t * filp, char * buf, int count);
int ntfs_fwrite(file_t * filp, char * buf, int count);
inode_t* ntfs_opendir(inode_t * inode, char *name);
int _linux_ntfs_mkdir(inode_t* n, char* dname);

#endif



