/*
 *  The Syllable kernel
 *  Simple SCSI layer
 *  Contains some linux kernel code
 *  Copyright (C) 2003 Arno Klenke
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of version 2 of the GNU
 *  General Public License as published by the Free Software
 *  Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef _ATHEOS_SCSI_H_
#define _ATHEOS_SCSI_H_


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

enum direction
{
	NO_DATA = 0,
	WRITE,	/* TO device */
	READ	/* FROM device */
};

/* Return values from atapi_check_sense() */
enum check_sense
{
	SENSE_OK,
	SENSE_RETRY,
	SENSE_FATAL
};

enum
{
    IOCTL_GET_DEVICE_GEOMETRY = 1,
    IOCTL_REREAD_PTABLE,
    IOCTL_GET_DEVICE_PATH  ,
	IOCTL_GET_DEVICE_HANDLE ,
	
	IOCTL_GET_APPSERVER_DRIVER ,
	IOCTL_GET_USERSPACE_DRIVER ,
	
    IOCTL_USER = 100000000
};

typedef struct
{
    off_t	p_nStart;	/* Offset in bytes */
    off_t	p_nSize;	/* Size in bytes   */
    int		p_nType;	/* Type as found in the partition table	*/
    int		p_nStatus;	/* Status as found in partition table (bit 7=active) */
} Partition_s;

typedef struct device_geometry
{
  uint64 sector_count;
  uint64 cylinder_count;
  uint32 sectors_per_track;
  uint32 head_count;
  uint32 bytes_per_sector;
  bool	read_only;
  bool	removable;
} device_geometry;

/*
 *  SCSI command sets
 */

#define SCSI_UNKNOWN    0
#define SCSI_1          1
#define SCSI_1_CCS      2
#define SCSI_2          3
#define SCSI_3          4

/*
 * SCSI opcodes
 */

#define SCSI_TEST_UNIT_READY       0x00
#define SCSI_REZERO_UNIT           0x01
#define SCSI_REQUEST_SENSE         0x03
#define SCSI_FORMAT_UNIT           0x04
#define SCSI_READ_BLOCK_LIMITS     0x05
#define SCSI_REASSIGN_BLOCKS       0x07
#define SCSI_READ_6                0x08
#define SCSI_WRITE_6               0x0a
#define SCSI_SEEK_6                0x0b
#define SCSI_READ_REVERSE          0x0f
#define SCSI_WRITE_FILEMARKS       0x10
#define SCSI_SPACE                 0x11
#define SCSI_INQUIRY               0x12
#define SCSI_RECOVER_BUFFERED_DATA 0x14
#define SCSI_MODE_SELECT           0x15
#define SCSI_RESERVE               0x16
#define SCSI_RELEASE               0x17
#define SCSI_COPY                  0x18
#define SCSI_ERASE                 0x19
#define SCSI_MODE_SENSE            0x1a
#define SCSI_START_STOP            0x1b
#define SCSI_RECEIVE_DIAGNOSTIC    0x1c
#define SCSI_SEND_DIAGNOSTIC       0x1d
#define SCSI_ALLOW_MEDIUM_REMOVAL  0x1e

#define SCSI_SET_WINDOW            0x24
#define SCSI_READ_CAPACITY         0x25
#define SCSI_READ_10               0x28
#define SCSI_WRITE_10              0x2a
#define SCSI_SEEK_10               0x2b
#define SCSI_WRITE_VERIFY          0x2e
#define SCSI_VERIFY                0x2f
#define SCSI_SEARCH_HIGH           0x30
#define SCSI_SEARCH_EQUAL          0x31
#define SCSI_SEARCH_LOW            0x32
#define SCSI_SET_LIMITS            0x33
#define SCSI_PRE_FETCH             0x34
#define SCSI_READ_POSITION         0x34
#define SCSI_SYNCHRONIZE_CACHE     0x35
#define SCSI_LOCK_UNLOCK_CACHE     0x36
#define SCSI_READ_DEFECT_DATA      0x37
#define SCSI_MEDIUM_SCAN           0x38
#define SCSI_COMPARE               0x39
#define SCSI_COPY_VERIFY           0x3a
#define SCSI_WRITE_BUFFER          0x3b
#define SCSI_READ_BUFFER           0x3c
#define SCSI_UPDATE_BLOCK          0x3d
#define SCSI_READ_LONG             0x3e
#define SCSI_WRITE_LONG            0x3f
#define SCSI_CHANGE_DEFINITION     0x40
#define SCSI_WRITE_SAME            0x41
#define SCSI_READ_TOC              0x43
#define SCSI_LOG_SELECT            0x4c
#define SCSI_LOG_SENSE             0x4d
#define SCSI_MODE_SELECT_10        0x55
#define SCSI_RESERVE_10            0x56
#define SCSI_RELEASE_10            0x57
#define SCSI_MODE_SENSE_10         0x5a
#define SCSI_PERSISTENT_RESERVE_IN 0x5e
#define SCSI_PERSISTENT_RESERVE_OUT 0x5f
#define SCSI_MOVE_MEDIUM           0xa5
#define SCSI_READ_12               0xa8
#define SCSI_WRITE_12              0xaa
#define SCSI_WRITE_VERIFY_12       0xae
#define SCSI_SEARCH_HIGH_12        0xb0
#define SCSI_SEARCH_EQUAL_12       0xb1
#define SCSI_SEARCH_LOW_12         0xb2
#define SCSI_READ_ELEMENT_STATUS   0xb8
#define SCSI_SEND_VOLUME_TAG       0xb6
#define SCSI_READ_CD               0xbe
#define SCSI_WRITE_LONG_2          0xea

/*
 * Sense keys
 */

#define SCSI_NO_SENSE            0x00
#define SCSI_RECOVERED_ERROR     0x01
#define SCSI_NOT_READY           0x02
#define SCSI_MEDIUM_ERROR        0x03
#define SCSI_HARDWARE_ERROR      0x04
#define SCSI_ILLEGAL_REQUEST     0x05
#define SCSI_UNIT_ATTENTION      0x06
#define SCSI_DATA_PROTECT        0x07
#define SCSI_BLANK_CHECK         0x08
#define SCSI_VENDOR_SPECIFIC     0x09
#define SCSI_COPY_ABORTED        0x0a
#define SCSI_ABORTED_COMMAND     0x0b
#define SCSI_VOLUME_OVERFLOW     0x0d
#define SCSI_MISCOMPARE          0x0e

/*
 * Additional sense codes
 */

#define SCSI_NO_ASC_DATA					0x00
#define SCSI_LOGICAL_UNIT_NOT_READY			0x04
	#define SCSI_NOT_REPORTABLE				0x00
	#define SCSI_BECOMING_READY				0x01
	#define SCSI_MUST_INITIALIZE			0x02
	#define SCSI_MANUAL_INTERVENTION		0x03
	#define SCSI_FORMAT_IN_PROGRESS			0x04
	#define SCSI_REBUILD_IN_PROGRESS		0x05
	#define SCSI_RECALC_IN_PROGRESS			0x06
	#define SCSI_OP_IN_PROGRESS				0x07
	#define SCSI_LONG_WRITE_IN_PROGRESS		0x08
	#define SCSI_SELF_TEST_IN_PROGRESS		0x09
	#define SCSI_ASSYM_ACCESS_STATE_TRANS	0x0a
	#define SCSI_TARGET_PORT_STANDBY		0x0b
	#define SCSI_TARGET_PORT_UNAVAILABLE	0x0c
	#define SCSI_AUX_MEM_UNAVAILABLE		0x10
	#define SCSI_NOTIFY_REQUIRED			0x11
#define SCSI_NOT_RESPONDING					0x05
#define SCSI_MEDIUM							0x3a
	#define SCSI_MEDIUM_NOT_PRESENT			0x00
	#define SCSI_MEDIUM_TRAY_CLOSED			0x01
	#define	SCSI_MEDIUM_TRAY_OPEN			0x02
	#define	SCSI_MEDIUM_LOADABLE			0x03

/*
 * Transfer directions
 */
 
#define SCSI_DATA_UNKNOWN       0
#define SCSI_DATA_WRITE         1
#define SCSI_DATA_READ          2
#define SCSI_DATA_NONE          3

/*
 * Device types
 */

#define SCSI_TYPE_DISK           0x00
#define SCSI_TYPE_TAPE           0x01
#define SCSI_TYPE_PRINTER        0x02
#define SCSI_TYPE_PROCESSOR      0x03	/* HP scanners use this */
#define SCSI_TYPE_WORM           0x04	/* Treated as ROM by our system */
#define SCSI_TYPE_ROM            0x05
#define SCSI_TYPE_SCANNER        0x06
#define SCSI_TYPE_MOD            0x07	/* Magneto-optical disk - 
					 * - treated as TYPE_DISK */
#define SCSI_TYPE_MEDIUM_CHANGER 0x08
#define SCSI_TYPE_COMM           0x09	/* Communications device */
#define SCSI_TYPE_ENCLOSURE      0x0d	/* Enclosure Services Device */
#define SCSI_TYPE_NO_LUN         0x7f


/*
 * Status codes
 */

#define SCSI_GOOD                 0x00
#define SCSI_CHECK_CONDITION      0x01
#define SCSI_CONDITION_GOOD       0x02
#define SCSI_BUSY                 0x04
#define SCSI_INTERMEDIATE_GOOD    0x08
#define SCSI_INTERMEDIATE_C_GOOD  0x0a
#define SCSI_RESERVATION_CONFLICT 0x0c
#define SCSI_COMMAND_TERMINATED   0x11
#define SCSI_QUEUE_FULL           0x14

#define SCSI_STATUS_MASK          0x3e


/*
 * Message codes
 */

#define SCSI_COMMAND_COMPLETE    0x00
#define SCSI_EXTENDED_MESSAGE    0x01
#define     SCSI_EXTENDED_MODIFY_DATA_POINTER    0x00
#define     SCSI_EXTENDED_SDTR                   0x01
#define     SCSI_EXTENDED_EXTENDED_IDENTIFY      0x02	/* SCSI-I only */
#define     SCSI_EXTENDED_WDTR                   0x03
#define SCSI_SAVE_POINTERS       0x02
#define SCSI_RESTORE_POINTERS    0x03
#define SCSI_DISCONNECT          0x04
#define SCSI_INITIATOR_ERROR     0x05
#define SCSI_ABORT               0x06
#define SCSI_MESSAGE_REJECT      0x07
#define SCSI_NOP                 0x08
#define SCSI_MSG_PARITY_ERROR    0x09
#define SCSI_LINKED_CMD_COMPLETE 0x0a
#define SCSI_LINKED_FLG_CMD_COMPLETE 0x0b
#define SCSI_BUS_DEVICE_RESET    0x0c

#define SCSI_INITIATE_RECOVERY   0x0f	/* SCSI-II only */
#define SCSI_RELEASE_RECOVERY    0x10	/* SCSI-II only */

#define SCSI_SIMPLE_QUEUE_TAG    0x20
#define SCSI_HEAD_OF_QUEUE_TAG   0x21
#define SCSI_ORDERED_QUEUE_TAG   0x22


/*
 * Host codes
 */

#define SCSI_OK					0x00
#define SCSI_ERROR  			0x01

#define SCSI_NEEDS_RETRY     	0x2001
#define SCSI_SUCCESS         	0x2002
#define SCSI_FAILED          	0x2003
#define SCSI_QUEUED          	0x2004
#define SCSI_SOFT_ERROR      	0x2005
#define SCSI_ADD_TO_MLQUEUE  	0x2006

/*
 * Status macros
 */

#define SCSI_STATUS(result) (((result) >> 1) & 0x1f)
#define SCSI_STATUS_MESSAGE(result)    (((result) >> 8) & 0xff)
#define SCSI_STATUS_HOST(result)   (((result) >> 16) & 0xff)
#define SCSI_STATUS_DRIVER(result) (((result) >> 24) & 0xff)


/*****************************************/

#define MKCDIOCTL(x)	IOCTL_USER+x

/* Audio control */
#define CD_PLAY_MSF			MKCDIOCTL(0x1001)	/* Play Audio MSF */
#define CD_PLAY_LBA			MKCDIOCTL(0x1002)	/* Play Audio LBA */
#define CD_PLAY_TRACK		MKCDIOCTL(0x1003)	/* Play Audio Track */
#define CD_PAUSE			MKCDIOCTL(0x1004)	/* Pause Audio */ 
#define CD_RESUME			MKCDIOCTL(0x1005)	/* Resume paused Audio */
#define CD_STOP				MKCDIOCTL(0x1006)	/* Stop Audio playback */
#define CD_GET_TIME			MKCDIOCTL(0x1007)	/* Get current playback time & position */

/* Drive controls */
#define CD_EJECT			MKCDIOCTL(0x2001)	/* Open / Close the CD-ROM tray */

/* TOC/Track management */
#define CD_READ_TOC			MKCDIOCTL(0x3001)	/* Read the Table of Contents */
#define CD_READ_TOC_ENTRY	MKCDIOCTL(0x3002)	/* Read an entry of the Table of Contents */

/* CD-DA */
#define CD_READ_CDDA		MKCDIOCTL(0x4001)	/* Read a single CD-DA block */

/* Direct packet interface (E.g. cdrecord) */
#define CD_PACKET_COMMAND	MKCDIOCTL(0x5001)	/* Send a raw packet command to the drive */

/* Disc specific */

/* The leadout track is always 0xAA, regardless of # of tracks on disc */
#define	CDROM_LEADOUT		0xAA

/* Audio discs */
#define CD_FRAMESIZE	2048	/* Sector size for Mode 1 & 2 discs */
#define CD_SECS              60	/* seconds per minute */
#define CD_FRAMES            75	/* frames per second */
#define CD_MSF_OFFSET       150 /* MSF numbering offset of first frame */

#define CD_CDDA_FRAMESIZE	2352

static inline void lba_to_msf(int lba, uint8 *m, uint8 *s, uint8 *f)
{
	lba += CD_MSF_OFFSET;
	lba &= 0xffffff;  /* negative lbas use only 24 bits */
	*m = lba / (CD_SECS * CD_FRAMES);
	lba %= (CD_SECS * CD_FRAMES);
	*s = lba / CD_FRAMES;
	*f = lba % CD_FRAMES;
}


struct cdrom_packet_cmd
{
	uint8 nCommand[12];
	int nCommandLength;
	uint8 *pnData;
	int nDataLength;
	uint8 *pnSense;
	int nSenseLength;
	unsigned int nDirection;
	unsigned int nFlags;
	uint8 nSense;
	uint8 nError;
};

typedef struct cdrom_packet_cmd cdrom_packet_cmd_s;

/* CD-ROM Table Of Contents and Audio Track structures */
struct cdrom_toc_header
{
	unsigned short toc_length;
	uint8 first_track;
	uint8 last_track;
};

typedef struct cdrom_toc_header cdrom_toc_header_s;

/* Structure of a MSF cdrom address. */
struct cdrom_msf
{
	uint8 reserved;
	uint8 minute;
	uint8 second;
	uint8 frame;
};

typedef struct cdrom_msf cdrom_msf_s;

struct cdrom_toc_entry
{
	uint8 reserved1;
	uint8 control : 4;
	uint8 adr     : 4;
	uint8 track;
	uint8 reserved2;
	union
	{
		unsigned lba;
		cdrom_msf_s msf;
	} addr;
};

typedef struct cdrom_toc_entry cdrom_toc_entry_s;
#define MAX_TRACKS 99
struct cdrom_toc
{
	int    last_session_lba;
	int    xa_flag;
	unsigned long capacity;
	cdrom_toc_header_s hdr;
	cdrom_toc_entry_s  ent[MAX_TRACKS+1];		/* One extra track for the leadout. */
};

typedef struct cdrom_toc cdrom_toc_s;

struct iovec
{
  void*	 iov_base;
  size_t iov_len;
};

/*
 * Sense data
 */

typedef struct
{
	uint8 error_code	: 7;
	uint8 valid			: 1;
	uint8 segment_number;
	uint8 sense_key		: 4;
	uint8 reserved2		: 1;
	uint8 ili			: 1;
	uint8 reserved1		: 2;
	uint8 information[4];
	uint8 add_sense_len;
	uint8 command_info[4];
	uint8 asc;
	uint8 ascq;
	uint8 fruc;
	uint8 sks[3];
	uint8 asb[46];
} SCSI_sense_s;

/* 
 * SCSI controller
 */

typedef struct SCSI_cmd_t SCSI_cmd;

struct SCSI_host_t
{
	void *pPrivate;
	int ( *get_device_id ) ( void );
	char *( *get_name ) ( void );
	int ( *get_max_channel ) ( void );
	int ( *get_max_device ) ( void );
	int ( *queue_command ) ( SCSI_cmd * psCommand );
};

typedef struct SCSI_host_t SCSI_host_s;

struct SCSI_device_t;

/* 
 * SCSI device
 */

struct SCSI_device_t
{
	struct SCSI_device_t *psNext;
	struct SCSI_device_t *psRawDevice;
	struct SCSI_device_t *psFirstPartition;
	struct SCSI_host_t *psHost;

	sem_id hLock;

	int nID;
	int nNodeHandle;
	int nDeviceHandle;
	atomic_t nOpenCount;
	char zName[255];

	int nChannel;
	int nDevice;
	int nLun;

	int nType;
	int nSCSILevel;
	bool bRemovable;

	char zVendor[8];
	char zModel[16];
	char zRev[4];

	uint8 *pDataBuffer;

	uint32 nSectorSize;
	uint32 nSectors;

	int nPartitionType;
	off_t nStart;
	off_t nSize;

	/* For CD-ROM drives */
	bool bMediaChanged;
	bool bTocValid;
	cdrom_toc_s	sToc;

};

typedef struct SCSI_device_t SCSI_device_s;

/*
 * SCSI command
 */

#define SCSI_CMD_SIZE 16
#define SCSI_SENSE_SIZE 64

struct SCSI_cmd_t
{
	struct SCSI_host_t *psHost;

	int nChannel;
	int nDevice;
	int nLun;

	int nResult;

	int nDirection;
	unsigned char nCmd[SCSI_CMD_SIZE];
	int nCmdLen;
	union
	{
		unsigned char nSense[SCSI_SENSE_SIZE];
		SCSI_sense_s sSense;
	} s;

	uint8 *pRequestBuffer;
	unsigned nRequestSize;
};

typedef struct SCSI_cmd_t SCSI_cmd_s;

/*
 * SCSI bus
 */
 
#define SCSI_BUS_NAME "scsi"
#define SCSI_BUS_VERSION 1

typedef struct
{
	unsigned char 	(*get_command_size)( int nOpcode );
	void 			(*scan_host)( SCSI_host_s * psHost );
	void 			(*remove_host)( SCSI_host_s * psHost );
} SCSI_bus_s;

extern const unsigned char nSCSICmdSize[8];

extern SCSI_device_s *g_ScsiDisk ;
extern SCSI_device_s *g_ScsiCdrom ;


#endif



