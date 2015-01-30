/*
	Copyright 1999, Be Incorporated.   All Rights Reserved.
	This file may be used under the terms of the Be Sample Code License.
*/

#ifndef _DOSFS_ITER_H_
#define _DOSFS_ITER_H_
#include <drv/drv.h>
#include <string.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>
#include <drv/errno.h>
#include <net/inet.h>



typedef enum fsattr_type
{
    ATTR_TYPE_NONE,     /*! untyped "raw" data */
    ATTR_TYPE_INT32,    /*! 32-bit integer value */
    ATTR_TYPE_INT64,    /*! 64-bit integer value (also used for time-values) */
    ATTR_TYPE_FLOAT,    /*! 32-bit floating point value */
    ATTR_TYPE_DOUBLE,   /*! 64-bit floating point value */
    ATTR_TYPE_STRING,   /*! UTF8 string */
    ATTR_TYPE_COUNT     /*! not a valid type, just gives the number of valid types */
} fsattr_type;

#define B_FS_IS_READONLY                0x00000001
#define B_FS_IS_REMOVABLE               0x00000002
#define B_FS_IS_PERSISTENT              0x00000004
#define B_FS_IS_SHARED                  0x00000008
#define B_FS_HAS_MIME                   0x00010000
#define B_FS_HAS_ATTR                   0x00020000
#define B_FS_HAS_QUERY                  0x00040000

typedef long long int64;
typedef long  int32;
typedef short  int16;
typedef char  int8;
typedef unsigned int  uint;
typedef unsigned char uchar;
typedef unsigned long long uint64;
typedef unsigned long  uint32;
typedef unsigned short  uint16;
typedef unsigned char  uint8;

typedef  int status_t;
typedef  int sem_id;
typedef uint64 bigtime_t;

typedef struct attr_info
{
    off_t ai_size;
    int	  ai_type;
} attr_info_s;


  /* Flags returned in the fi_flags field of fs_info */
#define	FS_IS_READONLY	 0x00000001	/* Set if mounted readonly or resides on a RO meadia */
#define	FS_IS_REMOVABLE	 0x00000002	/* Set if mounted on a removable media */
#define	FS_IS_PERSISTENT 0x00000004	/* Set if data written to the FS is preserved across reboots */
#define	FS_IS_SHARED	 0x00000008	/* Set if the FS is shared across multiple machines (Network FS) */
#define FS_IS_BLOCKBASED 0x00000010	/* Set if the FS use a regular blockdevice
					 * (or loopback from a single file) to store its data.
					 */
#define FS_CAN_MOUNT	 0x00000020	/* Set by probe() if the FS can mount the given device */
#define	FS_HAS_MIME	 0x00010000
#define	FS_HAS_ATTR	 0x00020000
#define	FS_HAS_QUERY	 0x00040000


#define WSTAT_MODE   0x0001
#define	WSTAT_UID    0x0002
#define	WSTAT_GID    0x0004
#define	WSTAT_SIZE   0x0008
#define	WSTAT_ATIME  0x0010
#define	WSTAT_MTIME  0x0020
#define	WSTAT_CTIME  0x0040

#define	WFSSTAT_NAME 0x0001


enum
{
    NWEVENT_CREATED = 1,
    NWEVENT_DELETED,
    NWEVENT_MOVED,
    NWEVENT_STAT_CHANGED,
    NWEVENT_ATTR_WRITTEN,
    NWEVENT_ATTR_DELETED,
    NWEVENT_FS_MOUNTED,
    NWEVENT_FS_UNMOUNTED
};

typedef struct device_geometry
{
  uint64 sector_count;
  uint64 cylinder_count;
  uint32 sectors_per_track;
  uint32 head_count;
  uint32 bytes_per_sector;
  char	read_only;
  char	removable;
} device_geometry;



struct _nspace;

/* csi keeps track of current cluster and sector info */
struct csi
{
	struct _nspace *vol;
	uint32	cluster;
	uint32	sector;
};

int init_csi(struct _nspace *vol, uint32 cluster, uint32 sector, struct csi *csi);
int iter_csi(struct csi *csi, int sectors);
uint8 *csi_get_block(struct csi *csi);
status_t csi_release_block(struct csi *csi);
status_t csi_mark_block_dirty(struct csi *csi);
status_t csi_read_block(struct csi *csi, uint8 *buffer);
status_t csi_write_block(struct csi *csi, uint8 *buffer);

/* directory entry iterator */
#define DIRI_MAGIC '!duM'
struct diri
{
	uint32	magic;
	struct csi csi;
	bool dirty;
	uint32 starting_cluster;
	uint32 current_index;
	uint8 *current_block;
};

uint8 *diri_init(struct _nspace *vol, uint32 cluster, uint32 index, struct diri *diri);
int diri_free(struct diri *diri);
uint8 *diri_current_entry(struct diri *diri);
uint8 *diri_next_entry(struct diri *diri);
uint8 *diri_rewind(struct diri *diri);
void diri_mark_dirty(struct diri *diri);

int check_diri_magic(struct diri *t, char *funcname);

#endif
