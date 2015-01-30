

#ifndef UTIL_H_
#define UTIL_H_

typedef struct
{
    u16_t nStructSize;
    u16_t nFlags;
    u32_t nCylinders;
    u32_t nHeads;
    u32_t nSectors;
    u64_t nTotSectors;
    u16_t nBytesPerSector;
} drive_param_t;

typedef struct device_geometry
{
  u64_t sector_count;
  u64_t cylinder_count;
  u32_t sectors_per_track;
  u32_t head_count;
  u32_t bytes_per_sector;
  bool	read_only;
  bool	removable;
} device_geometry;

typedef struct
{
    off_t	p_nStart;	/* Offset in bytes */
    off_t	p_nSize;	/* Size in bytes   */
    int		p_nType;	/* Type as found in the partition table	*/
    int		p_nStatus;	/* Status as found in partition table (bit 7=active) */
} Partition_s;
#define	BDD_ROOT_INODE	0x124131

typedef struct biosdisk_drv biosdisk_drv_t;
struct biosdisk_drv
{
    biosdisk_drv_t*	bi_psFirstPartition;
    biosdisk_drv_t*	bi_psNext;
    int		d_DeviceHandle;
    char	bi_zName[16];
    //atomic_t	d_OpenCount;
    int		d_DriveNum;	/* The bios drive number (0x80-0xff) */
    int		d_NodeHandle;
    int		d_PartitionType;
    int		d_SectorSize;
    int		d_Sectors;
    int		d_Cylinders;
    int		d_Heads;
    off_t	d_Start;
    off_t	d_Size;
    bool	d_bRemovable;
    bool	d_bLockable;
    bool	d_bHasChangeLine;
    bool	d_bCSHAddressing;
    bool	d_bTruncateToCyl;
};


/* Address packet to specify disk positions in extended INT13h services */
typedef struct
{
    u8_t   size;       /* Size of this structure: 10h or 18h                  */
    u8_t   reserved;   /* 0                                                   */
    u16_t   num_blocks; /* Max 007Fh for Phoenix EDD                           */
    u32_t  buf_addr;   /* Seg:off pointer to transfer buffer                  */
    u64_t  start;      /* Starting absolute block number                      */
    u64_t  flat_addr;  /* (EDD-3.0, optional) 64-bit flat address of transfer */
                       /* buffer; used if u32_t at 04h is FFFFh:FFFFh         */
}
__attribute__ ((packed)) AddressPacket;

#define BCF_EXTENDED_DISK_ACCESS	0x01 // (0x42-0x44,0x47,0x48)
#define BCF_REMOVABLE_DRIVE_FUNCTIONS	0x02 // (0x45,0x46,0x48,0x49, INT 15/0x52)
#define BCF_ENHANCED_DDRIVE_FUNCTIONS	0x04 // (0x48,0x4e)

// Information flags returned by GET DRIVE PARAMETERS (0x48)

#define DIF_DMA_BND_ERRORS_HANDLED	0x01 // DMA boundary errors handled transparantly.
#define DIF_CSH_INFO_VALID		0x02 // CHS information is valid.
#define DIF_REMOVABLE			0x04 // Removable drive.
#define DIF_WRITE_VERIFY_SUPPORTED	0x08 // Write with verify supported.
#define DIF_HAS_CHANGE_LINE		0x10 // Drive has change-line support. (Removable only)
#define DIF_CAN_LOCK			0x20 // Drive can be locked. (Removable only)
#define DIF_CHS_MAXED_OUT		0x40 // CHS info set to maximum supported values, not current media.

#define BDD_BUFFER_SIZE 4096

extern char*	g_cmdbuf;
extern char*	g_databuf;
extern char*	g_rawdata; // Same as g_databuf but is not aligned to 16 byte boundary
extern krnl_timer_t g_floppy_timer;
void floppy_off_timer( void* pData );
typedef s32_t S64_t;

typedef struct
{
	u8_t p_nStatus;
	u8_t p_nFirstHead;
	u16_t p_nFirstCyl;
	u8_t p_nType;
	u8_t p_nLastHead;
	u16_t p_nLastCyl;
	u32_t p_nStartLBA;
	u32_t p_nSize;
} PartitionRecord_s;
typedef size_t disk_read_op( void* pCookie, off_t n_off, void* pbuf, size_t nSize );

#endif /* UTIL_H_ */
