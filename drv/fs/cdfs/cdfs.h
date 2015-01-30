/*!
    \file  File.h
    \brief File

    Copyright (c) 2004 HigePon
    WITHOUT ANY WARRANTY

    \author  HigePon
    \version $Revision: 1.2 $
    \date   create:2004/11/06 update:$Date: 2005/07/17 13:27:24 $
*/

#ifndef _ISO9660_FILE_
#define _ISO9660_FILE_

#define ISO_POSITION(from, to) (to - from + 1)
#define ISO9660ROOT_NO 1

enum
{
	ISO_PRIMARY_VOLUME_DESCRIPTOR = 1,
	ISO_END_VOLUME_DESCRIPTOR     = 255
};

enum
{
	CDROM_SECTOR_SIZE = 2048,
	NO_ERROR = 0,
	READ_ERROR,
	MEMORY_ALLOCATE_ERROR,
	VOLUME_DESCRIPTOR_NOT_FOUND,
	DIRECTORY_NOT_FOUND,
};

/*typedef struct iso_volume_descriptor
{
    unsigned char type;
    char id  [ISO_POSITION(2, 6)];
    unsigned char version;
    char data[ISO_POSITION(8, 2048)];
}iso_volume_descriptor;
*/
typedef struct iso_volume_descriptor
{
    char type                  [ISO_POSITION (  1,   1)];
    char id                    [ISO_POSITION (  2,   6)];
    char version               [ISO_POSITION (  7,   7)];
    char unused1               [ISO_POSITION (  8,   8)];
    char system_id             [ISO_POSITION (  9,  40)];
    char volume_id             [ISO_POSITION ( 41,  72)];
    char unused2               [ISO_POSITION ( 73,  80)];
    char volume_space_size     [ISO_POSITION ( 81,  88)];
    unsigned char escape_sequences               [ISO_POSITION ( 89, 120)];
    char volume_set_size       [ISO_POSITION (121, 124)];
    char volume_sequence_number[ISO_POSITION (125, 128)];

	unsigned char logical_block_size	[ISO_POSITION(129, 132)]; // 723
	unsigned char path_table_size		[ISO_POSITION(133, 140)]; // 733
	unsigned char type_l_path_table	[ISO_POSITION(141, 144)]; // 731
	unsigned char opt_type_l_path_table	[ISO_POSITION(145, 148)]; // 731
	unsigned char type_m_path_table	[ISO_POSITION(149, 152)]; // 732
	unsigned char opt_type_m_path_table	[ISO_POSITION(153, 156)]; // 7

    /*unsigned short logical_block_size_l;
    unsigned short logical_block_size_b;
    unsigned long path_table_size_l;
    unsigned long path_table_size_b;
    unsigned long type_l_path_table;
    unsigned long opt_type_l_path_table;
    unsigned long type_m_path_table;
    unsigned long opt_type_m_path_table;*/
    char root_directory_record [ISO_POSITION (157, 190)];
    char volume_set_id         [ISO_POSITION (191, 318)];
    char publisher_id          [ISO_POSITION (319, 446)];
    char preparer_id           [ISO_POSITION (447, 574)];
    char application_id        [ISO_POSITION (575, 702)];
    char copyright_file_id     [ISO_POSITION (703, 739)];
    char abstract_file_id      [ISO_POSITION (740, 776)];
    char bibliographic_file_id [ISO_POSITION (777, 813)];
    char creation_date         [ISO_POSITION (814, 830)];
    char modification_date     [ISO_POSITION (831, 847)];
    char expiration_date       [ISO_POSITION (848, 864)];
    char effective_date        [ISO_POSITION (865, 881)];
    char file_structure_version[ISO_POSITION (882, 882)];
    char unused4               [ISO_POSITION (883, 883)];
    char application_data      [ISO_POSITION (884, 1395)];
    char unused5               [ISO_POSITION (1396, 2048)];
}iso_volume_descriptor;

/*----------------------------------------------------------------------
    ISOPathTableEntry
----------------------------------------------------------------------*/
#pragma pack(2)
typedef struct iso_pathtable_record
{
    unsigned char length;
    unsigned char ext_attr_length;
    unsigned long extent;
    unsigned short parentDirectory;
    char name[0];
} ISOPathTableEntry;

#pragma pack(0)

/*----------------------------------------------------------------------
    ISODirectoryEntry
----------------------------------------------------------------------*/
#pragma pack(1)
typedef struct ISODirectoryEntry
{
    unsigned char length;
    unsigned char ext_attr_length;
    unsigned long extent_l;
    unsigned long extent_b;
    unsigned long size_l;
    unsigned long size_b;
    char date[ISO_POSITION (19, 25)];
    unsigned existence      : 1;
    unsigned directory      : 1;
    unsigned associetedFile : 1;
    unsigned record         : 1;
    unsigned protection     : 1;
    unsigned reserved       : 2;
    unsigned lastRecord     : 1;
    unsigned char file_unit_size;
    unsigned char interleave;
    unsigned short volume_sequence_number_l;
    unsigned short volume_sequence_number_b;
    unsigned char name_len;
    char name[0];
};
#pragma pack(0)

/*----------------------------------------------------------------------
    ISO9660Attribute
----------------------------------------------------------------------*/
typedef struct ISO9660Attribute
{
    unsigned long id;
    unsigned long parentID;
    unsigned long extent;
    unsigned long size;
};

struct iso_directory_record 
{
  unsigned char length			[ISO_POSITION( 1,  1)]; // 711
  unsigned char ext_attr_length		[ISO_POSITION( 2,  2)]; // 711
  unsigned char extent			[ISO_POSITION( 3, 10)]; // 733
  unsigned char size			[ISO_POSITION(11, 18)]; // 733
  unsigned char date			[ISO_POSITION(19, 25)]; // 7 by 711
  unsigned char flags			[ISO_POSITION(26, 26)];
  unsigned char file_unit_size		[ISO_POSITION(27, 27)]; // 711
  unsigned char interleave		[ISO_POSITION(28, 28)]; // 711
  unsigned char volume_sequence_number	[ISO_POSITION(29, 32)]; // 723
  unsigned char name_len		[ISO_POSITION(33, 33)]; // 711
  unsigned char name			[0];
};


static inline  int isonum_711(unsigned char *p)
{
  return p[0];
}

static inline  int isonum_712(unsigned char *p)
{
  return (char) p[0];
}

static inline  int isonum_721(unsigned char *p)
{
  return p[0] | ((char) p[1] << 8);
}

static inline  int isonum_722(unsigned char *p)
{
  return ((char) p[0] << 8) | p[1];
}

static inline  int isonum_723(unsigned char *p)
{
  return isonum_721(p);
}

static inline  int isonum_731(unsigned char *p)
{
  return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

static inline  int isonum_732(unsigned char *p)
{
  return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

static inline  int isonum_733(unsigned char *p)
{
  return isonum_731(p);
}


static inline  unsigned short htons(unsigned short n)
{
  return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
}

static inline  unsigned short ntohs(unsigned short n)
{
  return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
}

static inline  unsigned long htonl(unsigned long n)
{
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}

static inline  unsigned long ntohl(unsigned long n)
{
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}


#define PS1                     '/'    // Primary path separator
#define PS2                     '\\'   // Alternate path separator


struct cdfs
{
  dev_prvi_t* devptr;
  int blks;
  int volblks;
  int vdblk;
  int joliet;
  //buffer_tpool *cache;
  unsigned char *path_table_buffer;
  struct iso_pathtable_record **path_table;
  int path_table_records;
};



#endif
