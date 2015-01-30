

#ifndef __RAMFS_H__
#define __RAMFS_H__

#define	LOCK		lock_semaphore
#define	UNLOCK	unlock_semaphore



#define S_IRWXUGO	(S_IRWXU|S_IRWXG|S_IRWXO)
#define S_IALLUGO	(S_ISUID|S_ISGID|S_ISVTX|S_IRWXUGO)
#define S_IRUGO		(S_IRUSR|S_IRGRP|S_IROTH)
#define S_IWUGO		(S_IWUSR|S_IWGRP|S_IWOTH)
#define S_IXUGO		(S_IXUSR|S_IXGRP|S_IXOTH)

#define UTIME_NOW	((1l << 30) - 1l)
#define UTIME_OMIT	((1l << 30) - 2l)


typedef	struct	FileNode	FileNode_s;
typedef	struct	SuperInfo	SuperInfo_s;

#define	RFS_MAX_NAME_LEN	64
#define RFS_MAX_FILESIZE	(1024*1024*2)

enum
{
  RFS_ROOT = 1,
};

//#define USE_BULK 1
struct	FileNode
{
    FileNode_s*	fn_psNextHash;
    FileNode_s*	fn_psNextSibling;
    FileNode_s*	fn_psParent;
    FileNode_s*	fn_psFirstChild;
    int		fn_nInodeNum;
    int		fn_nMode;
    int		fn_nSize;
#ifdef USE_BULK
    int		fn_alloc;
#endif
    int		fn_nTime;
    int		fn_nLinkCount;
    bool	fn_bIsLoaded;	// true between read_inode() and write_inode()
    char	fn_zName[ RFS_MAX_NAME_LEN ];
    char*	fn_pBuffer;
};

typedef struct
{
    dev_t	rv_nDevNum;
    void*	rv_hMutex;
    FileNode_s*	rv_psRootNode;
    int		rv_nFSSize;
    int		rv_nFSSizeLimited;
} RDVolume_s;

typedef struct
{
    int	nPos;
} RFSCookie_s;

#define RAMFS_DEVNO 0X9906

#endif

