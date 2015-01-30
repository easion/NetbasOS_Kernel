#ifndef EXT2FS_H
#define EXT2FS_H

#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>

#define EXT2_MAGIC	 0xEF53   /////////当前版本魔数
#define	BAD_INO		 1	/////////错误块节点
#define EXT2_ROOT		 2	/////////根节点
#define ACL_INO		 3	   /////////////// ACL 节点
#define EXT2_SIZE		1024	/////////////第一个保留节点
#define EXT2_NAME_LEN 255 //文件名长度

#define EXT2_DIR 16877 ////0x41ed
#define EXT2_FILE 33188    ////ox81a4
#define IN_DISK 1

struct ext2_entry {  //目录项
	unsigned long	inode;			/////////// 节点号Inode number 
	unsigned short	rec_len;		/////////// 入口长度Directory entry length 
	unsigned short	name_len;		///////////文件名长度 Name length 
	char	name[255];	///////////目录名 File name 
};

struct ext2_entry2 {
	unsigned long	inode;			/////////// 节点号Inode number 
	unsigned short	rec_len;		/////////// 入口长度Directory entry length 
	unsigned char	name_len;		///////////目录名长度 Name length 
	unsigned char	file_type;
	char	name[255];	///////////目录名 File name 
};

struct ext2_super 
{
	unsigned long	s_inodes_count;		/////////// Inodes count 
	unsigned long	s_blocks_count;		/////////// Blocks count 
	unsigned long	s_r_blocks_count;	/////////// Reserved blocks count 
	unsigned long	s_free_blocks_count;	/////////// Free blocks count 
	unsigned long	s_free_inodes_count;	/////////// Free inodes count 
	unsigned long	s_first_data_block;	/////////// First Data Block 
	unsigned long	s_log_block_size;	/////////// Block size 
	signed long	s_log_frag_size;	/////////// Fragment size 
	unsigned long	s_blocks_per_group;	/////////// # Blocks per group 
	unsigned long	s_frags_per_group;	/////////// # Fragments per group 
	unsigned long	s_inodes_per_group;	/////////// # Inodes per group 

	unsigned long	s_mtime;		/////////// Mount time 
	unsigned long	s_wtime;		/////////// Write time 
	unsigned short	s_mnt_count;		/////////// Mount count 
	signed short	s_max_mnt_count;	/////////// Maximal mount count 
	unsigned short	s_magic;		/////////// Magic signature 
	unsigned short	s_state;		/////////// File system state 
	unsigned short	s_errors;		/////////// Behaviour when detecting errors 
	unsigned short	s_pad;
	unsigned long	s_lastcheck;		/////////// time of last check 
	unsigned long	s_checkinterval;	/////////// max. time between checks 
	unsigned long	s_creator_os;		/////////// OS 
	unsigned long	s_rev_level;		/////////// Revision level 
	unsigned short	s_def_resuid;		/////////// Default uid for reserved blocks 
	unsigned short	s_def_resgid;		/////////// Default gid for reserved blocks 
	unsigned long	s_reserved[235];	/////////// Padding to the end of the block 
};

struct ext2_group
{
	unsigned long	bg_block_bitmap;		/////////// Blocks bitmap block 
	unsigned long	bg_inode_bitmap;		/////////// Inodes bitmap block 
	unsigned long	bg_inode_table;		/////////// Inodes table block 
	unsigned short	bg_free_blocks_count;	/////////// Free blocks count 
	unsigned short	bg_free_inodes_count;	/////////// Free inodes count 
	unsigned short	bg_used_dirs_count;	/////////// Directories count 
	unsigned short	bg_pad;
	unsigned long	bg_reserved[3];
};

struct ext2_inode {
	unsigned short	i_mode;		/////////// 文件模式File mode 
	unsigned short	i_uid;		/////////// Owner用户 Uid 
	unsigned long	i_size;		/////////// 大小Size in bytes 
	unsigned long	i_atime;	/////////// 存取时间Access time 
	unsigned long	i_ctime;	/////////// 创建时间Creation time 
	unsigned long	i_mtime;	/////////// 修改时间Modification time 
	unsigned long	i_dtime;	///////////删除时间 Deletion Time 
	unsigned short	i_gid;		/////////// Group组 Id 
	unsigned short	i_links_count;	/////////// 连接数Links count 
	unsigned long	i_blocks;	/////////// 块数Blocks count 
	unsigned long	i_flags;	/////////// 文件标志File flags 
	union {
		struct {
			unsigned long  l_i_reserved1;
		} linux1;
		struct {
			unsigned long  h_i_translator;
		} hurd1;
		struct {
			unsigned long  m_i_reserved1;
		} masix1;
	} osd1;				/////////// OS系统决定 dependent 1 
	unsigned long	i_block[15];///////////指向块 Pointers to blocks 
	unsigned long	i_version;	/////////// 版本File version (for NFS) 
	unsigned long	i_file_acl;	/////////// File ACL 
	unsigned long	i_dir_acl;	/////////// Directory ACL 
	unsigned long	i_faddr;	/////////// 碎片地址Fragment address 
	union {
		struct {
			unsigned char	l_i_frag;	/////////// 碎片数Fragment number 
			unsigned char	l_i_fsize;	/////////// 碎片大小Fragment size 
			unsigned short	i_pad1;
			unsigned long	l_i_reserved2[2];
		} linux2;  //LINUX系统
		struct {
			unsigned char	h_i_frag;	/////////// 碎片数Fragment number 
			unsigned char	h_i_fsize;	/////////// 碎片大小Fragment size 
			unsigned short	h_i_mode_high;
			unsigned short	h_i_uid_high;
			unsigned short	h_i_gid_high;
			unsigned long	h_i_author;
		} hurd2; //HURD系统
		struct {
			unsigned char	m_i_frag;	/////////// Fragment number 
			unsigned char	m_i_fsize;	/////////// Fragment size 
			unsigned short	m_pad1;
			unsigned long	m_i_reserved2[2];
		} masix2;
	} osd2;				/////////// OS dependent 2 
};

struct ext2_info {	
	unsigned int group_nr;
	unsigned char group_desc[3][1024];
	unsigned long   gd_first_block;        ////the groud descriptors start block is 1st block or 2en block
	unsigned long	block_size;	       ///// sector size
	unsigned long	s_block_per_group;  	/////////// Number of inode table blocks per group 	
	unsigned long s_desc_per_block;
	unsigned long s_inodes_per_block;
	unsigned long s_inodes_per_group;

	unsigned short loaded_imaps;
	unsigned short loaded_bmaps;
	unsigned long imap_number[8];   ////最大的载入组数为8
	unsigned long bmap_number[8];  ///块位图
};

extern struct ext2_info sp_info;
extern inode_t* ext2_file2node(inode_t* current, unsigned char* file);
extern inode_t* ext2_opendir(inode_t* prev, unsigned char* name);
extern inode_t* ext2_root;
extern inode_t *ext2_iget(const int, const int inode_nr);

extern int ext2_dir(inode_t *node);

#endif

