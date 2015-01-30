

#ifndef fat_dll_H
#define fat_dll_H

#define FAT12_MAX    4078 // 12 位 FAT的最大串号 
#define FREE_FAT12 0x0


typedef enum { FAT12FS, FAT16FS, FAT32FS, EXT2FS } tFatType;

#define BAD_FAT12    0xff7
#define BAD_FAT16 0xfff7
#define BAD_FAT32     0x0ffffff7

#define EOF_FAT12   0xff8		// standard EOF 
#define EOF_FAT16   0xfff8
#define EOF_FAT32   0x0ffffff8
#define         FILE_NOT_FOUND  0xffffffff
#define FAT12_ROOT	 1

#define  FAT12_EOC(EntryValue)  (EntryValue >= 0x0FF8)
#define  FAT16_EOC(EntryValue)  (EntryValue >= 0xFFF8)
#define  FAT32_EOC(EntryValue)  (EntryValue == 0x0FFFFFF8)

#define  FAT12_UNUSED(EntryValue)  (EntryValue == 0x0000)
#define  FAT16_UNUSED(EntryValue)  (EntryValue == 0x0000)
#define  FAT32_UNUSED(EntryValue)  (EntryValue == 0x0000000)

typedef struct ftime{
  unsigned sec:5;
  unsigned min:6;
  unsigned hour:5;
}__attribute__((packed)) DOS_TIME;

typedef struct  fdate{
  unsigned day_of_month:5;
  unsigned month:4;
  unsigned year:7; /* since 1980 */
}__attribute__((packed)) DOS_DATE;


/* FAT 32-byte Long File Name Directory Entry structure */
typedef struct
{
  u8_t Order;        /* Sequence number for slot        */
  u16_t Name0_4[5];   /* First 5 Unicode characters      */
  u8_t l_attribute;         /* Attributes, always 0x0F         */
  u8_t l_res;     /* Reserved, always 0x00           */
  u8_t checksum_value;     /* checksum_value of 8.3 name            */
  u16_t Name5_10[6];  /* 6 more Unicode characters       */
  u16_t first_cluster;      /* First cluster number, must be 0 */
  u16_t Name11_12[2]; /* Last 2 Unicode characters       */
}
__attribute__ ((packed)) lfn_entry_t;

typedef struct msdos_dir
{
	u8_t file_name[8];
	u8_t ext_name[3];	// name and extension 
	u8_t	attribute;		// attribute bits 

	u8_t    lcase;		// Case for base and extension 
	u8_t	ctime_ms;	// Creation time, milliseconds 
	DOS_TIME	ctime;		// Creation time 
	DOS_DATE	cdate;		// Creation date 
	DOS_DATE	adate;		// Last access date 
	u16_t   first_high;	// High 16 bits of cluster in FAT32 

	DOS_TIME	time;
	DOS_DATE date;
	u16_t first_cluster;// first cluster 
	u32_t	file_size;		// file size (in u8_ts) 
}__attribute__((packed)) fat_dir_entry;

#define FIRSTCLUSTER(D) (((u32_t) D.first_high << 16) + (u32_t) D.first_cluster)

struct msdos_super 
{
	signed char	jmp_boot[3];	// Boot strap short or near jump 
	signed char	oem[8];	// Name - can be used to special casepartition manager volumes 
	u8_t	sector_size[2];	// u8_ts per logical sector 
	u8_t	cluster_size;	// sectors/cluster 
	u16_t	reserved;	// reserved sectors 
	//u8_t	reserved[2];	// reserved sectors 
	u8_t	fats;		// number of FATs 
	u8_t	dir_entries[2];	// root directory entries 
	u8_t	sectors[2];	// number of sectors 
	u8_t	media_descriptor;		// media code (unused) 
	u16_t	fat16_length;	// sectors/FAT 
	u16_t	sec_per_track;	// sectors per track 
	u16_t	heads;		// number of heads 
	u32_t	hidden;		// hidden sectors (unused) 
	u32_t	total_sectors;	// number of sectors (if sectors == 0) 
}__attribute__((packed));

struct fat_1x /* specific part for fat 12/16 */
{
	unsigned char drive_number;
	unsigned char reserved;
	unsigned char boot_signature;
	unsigned long volume_id;
	unsigned char volume_label[ 11 ];
	unsigned char fstype[ 8 ];
}  __attribute__((packed));

struct fat_32
{
	unsigned long fat_size;
	unsigned short reserved:8;
	unsigned short mirroring:1;
	unsigned short reserved2:3;
	unsigned short active_fat:4;
	unsigned short filesystem_version;
	unsigned long fat32_root_cluster;
	unsigned short filesystem_info;
	unsigned short backup_boot_record;
	unsigned char reserved3[ 12 ];
	unsigned char drive_number;
	unsigned char reserved4;
	unsigned char boot_signature;
	unsigned long volume_id;
	char volume_label[ 11 ];
	char filesyste_type[ 8 ];
}  __attribute__((packed));

struct msdos_info
{
	 u16_t sector_size;
	 u8_t cluster_size;
	 u32_t blk_size;
	 u32_t fat_size;
	 u16_t dir_entries;
	 u32_t fat_entries;

	u32_t fat_base;
	u32_t fat_root;
     
	// u32_t fs_base;
	 u32_t blk_base ;  //////数据区开始

	struct msdos_super *dos_sp;
	dev_t devno;
	struct msdos_info *next;

}__attribute__((packed));


#define FAT_START_BLK 1

#define MSDOS_READONLY      1  // 只读
#define MSDOS_HIDDEN  2  // 隐藏文件 
#define MSDOS_SYS_FILE     4  // 系统文件 
#define MSDOS_VOLUME  8  // 卷标 
#define MSDOS_LONGNAME     (MSDOS_READONLY |  MSDOS_HIDDEN | MSDOS_SYS_FILE | MSDOS_VOLUME)//  
#define MSDOS_DIR     16 // 目录 
#define MSDOS_ARCH    32 // 存档文件 

#define MSDOS_ISVOLUME(attribute)		(((attribute) &0x20) && ((attribute) & 0x08))
#define MSDOS_ISDIR(attribute)		(((attribute) &0x10) && !((attribute) & 0x08))
#define MSDOS_ISREG(attribute)		(!((attribute) & 0x08) && !((attribute) &0x10))

#define LFN_FETCH_SLOTS 21
#define LFN_END_FLAGS 0x40


/* Finddata block for the internal "fat_do_readdir" function (fat_readdir) */
typedef struct
{
  struct msdos_dir  SfnEntry;
  u32_t     EntryOffset;
  int       LfnEntries;
  char      l_long_name[260];
  //char      l_short_name[14];
}fatfs_find_t;



void convdos(vfs_dirent_t* Entry, fat_dir_entry * fatdir);
void dosconv(vfs_dirent_t* Entry, fat_dir_entry * fatdir);

bool fat_next_useable(mount_t *mp, u32_t *next, u32_t actual);

#endif


