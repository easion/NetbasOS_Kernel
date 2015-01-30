
/************************************************************
  Copyright (C), 2003-2010, Netbas OS Project.
  FileName: 
  Author:        Version :          Date:
  Description:    
  Version:        
  Function List:   
  History:         
      <author>  <time>   <version >   <desc>
      Easion   2010/2/6     1.0     build this moudle  
***********************************************************/


#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>

#include <errno.h>
#include <assert.h>
#include "ramfs.h"

#define K_MALLOC_FLAGS 0

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/


static FileNode_s* rfs_create_node( FileNode_s* psParent, const char* pzName, int nNameLen, int nMode )
{
    FileNode_s*	psNewNode;

    if ( nNameLen < 1 || nNameLen >= RFS_MAX_NAME_LEN ) {
	return( NULL );
    }

	//printf("rfs_create_node %s child %s\n", psParent->fn_zName, pzName);

  
    if ( (psNewNode = kmalloc( sizeof( FileNode_s ), K_MALLOC_FLAGS ) ) ) {
		memset(psNewNode,0,sizeof(FileNode_s));
		memcpy( psNewNode->fn_zName, pzName, nNameLen );
		psNewNode->fn_zName[ nNameLen ] = 0;

		psNewNode->fn_nMode	=	nMode;
		psNewNode->fn_nTime    = get_unix_time() ;

		psNewNode->fn_psNextSibling = psParent->fn_psFirstChild;
		psParent->fn_psFirstChild   = psNewNode;
		psNewNode->fn_psParent	    = psParent;
		psNewNode->fn_nLinkCount    = 1;
			
		psNewNode->fn_nInodeNum	=	(int) psNewNode;

		return( psNewNode );
    }
    return( NULL );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static void rfs_delete_node( RDVolume_s* psVolume, FileNode_s* psNode )
{
    FileNode_s* psParent = psNode->fn_psParent;
    FileNode_s** ppsTmp;

	if ( NULL == psParent )
	{
		printk( "iiik, sombody tried to delete '/' !!!\n" );
		return;
	}
	for ( ppsTmp = &psParent->fn_psFirstChild ; NULL != *ppsTmp ; ppsTmp = &((*ppsTmp)->fn_psNextSibling) ) {
	    if ( *ppsTmp == psNode ) {
		*ppsTmp = psNode->fn_psNextSibling;
		break;
	    }
	}
    psVolume->rv_nFSSize -= psNode->fn_nSize;
    kfree( psNode );
}

static FileNode_s* rfs_find_node( FileNode_s* psParent, const char* pzName, int nNameLen )
{
    FileNode_s*	psNode;

    if ( nNameLen == 1 && '.' == pzName[0] ) {
	return( psParent );
    }
    if ( nNameLen == 2 && '.' == pzName[0] && '.' == pzName[1] ) {
	if ( NULL != psParent->fn_psParent ) {
	    return( psParent->fn_psParent );
	} else {
	    printk( "Error: rfs_find_node() called with .. on root level\n" );
	    return( NULL );
	}
    }
    for ( psNode = psParent->fn_psFirstChild ; NULL != psNode ; psNode = psNode->fn_psNextSibling ) {
	if ( strlen( psNode->fn_zName ) == nNameLen && strncmp( psNode->fn_zName, pzName, nNameLen ) == 0 ) {
	    return( psNode );
	}
    }
    return( NULL );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_lookup( void* pVolume, void* pParent, const char* pzName, int nNameLen, ino_t* pnResInode )
{
    FileNode_s*	psParentNode = pParent;
    FileNode_s*	psNewNode;

	
    *pnResInode = 0;

    if ( nNameLen == 1 && '.' == pzName[0] ) {
	*pnResInode = psParentNode->fn_nInodeNum;
	goto done;
    }
    if ( nNameLen == 2 && '.' == pzName[0] && '.' == pzName[1] ) {
	if ( NULL != psParentNode->fn_psParent ) {
	    *pnResInode = psParentNode->fn_psParent->fn_nInodeNum;
	    goto done;
	} else {
	    printk( "Error: rfs_lookup() called with .. on root level\n" );
	    return( -ENOENT );
	}
    }

	if (nNameLen == 1 && !isprint(pzName[0]))
	{
		printf("go  222\n");
		return( -ENOENT );
	}


    for ( psNewNode = psParentNode->fn_psFirstChild ; NULL != psNewNode ; psNewNode = psNewNode->fn_psNextSibling ) {
		//printf("psNewNode->fn_zName=%s,%p,%s\n ", psNewNode->fn_zName,psParentNode,pzName);

	if ( strlen(psNewNode->fn_zName ) == nNameLen && strncmp( psNewNode->fn_zName, pzName, nNameLen ) == 0 ) {
		
	    *pnResInode = psNewNode->fn_nInodeNum;
	    break;
	}
    }
done:
    if ( 0L != *pnResInode ) {
	return( 0 );
    } else {
	return( -ENOENT );
    }
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_read( void* pVolume, void* pNode, void* pCookie, off_t nPos, void* pBuf, size_t nSize )
{
    RDVolume_s* psVolume = pVolume;
    FileNode_s* psNode = pNode;
    int		nError;

    LOCK( psVolume->rv_hMutex );
    if ( psNode->fn_pBuffer == NULL || nPos >= psNode->fn_nSize ) {
	nError = 0;
	goto done;
    }
    if ( nPos + nSize > psNode->fn_nSize ) {
	nSize = psNode->fn_nSize - nPos;
    }
    memcpy( pBuf, psNode->fn_pBuffer + nPos, nSize );
    nError = nSize;
done:
    UNLOCK( psVolume->rv_hMutex );
    return( nError );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/
#define MINI_BULK_SIZE (80*1024)
static int rfs_write( void* pVolume, void* pNode, void* pCookie, off_t nPos, const void* pBuf, size_t nLen )
{
    RDVolume_s* psVolume = pVolume;
    FileNode_s* psNode = pNode;
    int		nError;
	int bulks=1;
	void* pBuffer ;

    if ( nPos + nLen > RFS_MAX_FILESIZE ) {
	return( -EFBIG );
    }
    if ( nPos < 0 ) {
	return( -EINVAL );
    }
    
    LOCK( psVolume->rv_hMutex );

#ifndef USE_BULK
    if ( nPos + nLen > psNode->fn_nSize || psNode->fn_pBuffer == NULL ) {
	pBuffer = kmalloc( nPos + nLen, K_MALLOC_FLAGS );
	if ( pBuffer == NULL ) {
	    nError = -ENOMEM;
	    goto error;
	}
	if ( psNode->fn_pBuffer != NULL ) {
	    memcpy( pBuffer, psNode->fn_pBuffer, psNode->fn_nSize );
	    kfree( psNode->fn_pBuffer );
	}
	psVolume->rv_nFSSize += nPos + nLen - psNode->fn_nSize;
	psNode->fn_pBuffer = pBuffer;
	psNode->fn_nSize = nPos + nLen;
    }
#else
	if (psNode->fn_pBuffer == NULL)
	{
	pBuffer = kmalloc( MINI_BULK_SIZE, K_MALLOC_FLAGS );
	}
	else if (nPos + nLen > psNode->fn_nSize>psNode->fn_alloc)
	{
		bulks = (nPos + nLen + MINI_BULK_SIZE-1)/MINI_BULK_SIZE;
		pBuffer = kmalloc( MINI_BULK_SIZE, K_MALLOC_FLAGS );
		if ( psNode->fn_pBuffer != NULL && pBuffer) {
	    memcpy( pBuffer, psNode->fn_pBuffer, psNode->fn_nSize );
	    kfree( psNode->fn_pBuffer );
		}
	}
	else{
		goto done;
	}

	printf("ramfs: %s got line%d,%d,%s\n",__FUNCTION__, __LINE__,bulks,psNode->fn_zName);

	if ( pBuffer == NULL ) {
	    nError = -ENOMEM;
	    goto error;
	}
	

	psNode->fn_alloc = MINI_BULK_SIZE*bulks;
	psNode->fn_pBuffer =pBuffer;
done:
#endif
	psNode->fn_nSize = nPos + nLen;
    memcpy( psNode->fn_pBuffer + nPos, pBuf, nLen );
    nError = nLen;
error:
    UNLOCK( psVolume->rv_hMutex );
    return( nError );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/
#include "../fat/fat.h"

int rfs_readdir( void* pVolume, void* pNode, void* pCookie, int nPos,
		 vfs_dirent_t* psFileInfo, size_t nBufSize )
{
    FileNode_s* psParentNode = pNode;
    FileNode_s* psNode;
    int         nCurPos = nPos;

    if ( nCurPos == 0 ) {
	strcpy( psFileInfo->l_long_name, "." );
	psFileInfo->l_name_len = 1;
	psFileInfo->d.l_ino    = psParentNode->fn_nInodeNum;
	return( 1 );
    } else if ( nCurPos == 1 && psParentNode->fn_psParent != NULL ) {
	strcpy( psFileInfo->l_long_name, ".." );
	psFileInfo->l_name_len = 2;
	psFileInfo->d.l_ino    = psParentNode->fn_psParent->fn_nInodeNum;
	return( 1 );
    }
    if ( psParentNode->fn_psParent == NULL ) {
	nCurPos -= 1;
    } else {
	nCurPos -= 2;
    }
    
    for ( psNode = psParentNode->fn_psFirstChild ; NULL != psNode ; psNode = psNode->fn_psNextSibling )
    {
	if ( nCurPos == 0 )
	{
	    strcpy( psFileInfo->l_long_name, psNode->fn_zName );

	    psFileInfo->l_name_len = strlen( psFileInfo->l_long_name );
	    psFileInfo->d.l_ino	   = psNode->fn_nInodeNum;

		psFileInfo->l_atime = time_t2dos(psNode->fn_nTime);
		psFileInfo->l_ctime = time_t2dos(psNode->fn_nTime);
		psFileInfo->l_mtime  =  time_t2dos(psNode->fn_nTime);
		psFileInfo->l_size_high = 0;
		psFileInfo->l_size_low = psNode->fn_nSize;

		if (S_ISDIR( psNode->fn_nMode ))
			psFileInfo->l_attribute = MSDOS_DIR;
		else
			psFileInfo->l_attribute = MSDOS_ARCH;
			
	    return( 1 );
	}
	nCurPos--;
    }
    return( 0 );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_rstat( void* pVolume, void* pNode, struct stat* psStat )
{
    RDVolume_s* psVolume = pVolume;
    FileNode_s*   psNode = pNode;

    psStat->st_ino 	= psNode->fn_nInodeNum;
    psStat->st_dev	= psVolume->rv_nDevNum;
    psStat->st_size 	= psNode->fn_nSize;
    psStat->st_mode	= psNode->fn_nMode;
    psStat->st_nlink	= psNode->fn_nLinkCount;
    psStat->st_atime	= psNode->fn_nTime;
    psStat->st_mtime	= psNode->fn_nTime;
    psStat->st_ctime	= psNode->fn_nTime;
    psStat->st_uid	= 0;
    psStat->st_gid	= 0;

    return( 0 );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_open( void* pVolume, void* pNode, int nMode, void** ppCookie )
{
    return( 0 );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_close( void* pVolume, void* pNode, void* pCookie )
{
    return( 0 );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_read_inode( void* pVolume, ino_t nInodeNum, void** ppNode )
{
    RDVolume_s*	psVolume = pVolume;
    FileNode_s*	psNode;

    switch( nInodeNum )
    {
	case RFS_ROOT:
	    psNode = psVolume->rv_psRootNode;
	    break;
	default:
	    psNode = (FileNode_s*) ((int)nInodeNum);
	    if ( psNode->fn_nInodeNum != (int) psNode ) {
		printk( "rfs_read_inode() invalid inode %Lx\n", nInodeNum );
		psNode = NULL;
	    }
	    break;
    }
	
    if ( NULL != psNode )
    {
	assert( false == psNode->fn_bIsLoaded );
	psNode->fn_bIsLoaded = true;
	*ppNode = psNode;
	return( 0 );
    }
    else
    {
	*ppNode = NULL;
	return( -EINVAL );
    }
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_write_inode( void* pVolume, void* pNode )
{
    RDVolume_s* psVolume = pVolume;
    FileNode_s* psNode   = pNode;
	
    psNode->fn_bIsLoaded	= false;

    if ( 0 == psNode->fn_nLinkCount )
    {
	if ( NULL != psNode->fn_pBuffer ) {
	    kfree( psNode->fn_pBuffer );
	}
	psVolume->rv_nFSSize -= psNode->fn_nSize;
	kfree( psNode );
    }
    return( 0 );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_mkdir( void* pVolume, void* pParent, const char *pzName, int nNameLen, int nPerms )
{
    FileNode_s*	psParentNode = pParent;
    FileNode_s*	psNewNode;

    for ( psNewNode = psParentNode->fn_psFirstChild ; psNewNode != NULL ; psNewNode = psNewNode->fn_psNextSibling ) {
	if ( strncmp( psNewNode->fn_zName, pzName, nNameLen ) == 0 && strlen( psNewNode->fn_zName ) == nNameLen ) {
	    return( -EEXIST );
	}
    }
  
    if ( (psNewNode = rfs_create_node( psParentNode, pzName, nNameLen, S_IFDIR | (nPerms & S_IRWXUGO) )) )
    {
	return( 0 );
    }
    return( -1 );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_rmdir( void* pVolume, void* pParentNode, const char* pzName, int nNameLen )
{
    RDVolume_s*	 psVolume = pVolume;
    FileNode_s*  psParent = pParentNode;
    FileNode_s*	 psNode   = NULL;
    FileNode_s** ppsTmp;
    int		 nError = -ENOENT;
	
    for ( ppsTmp = &psParent->fn_psFirstChild ; NULL != *ppsTmp ; ppsTmp = &((*ppsTmp)->fn_psNextSibling) ) {
	if ( strlen( (*ppsTmp)->fn_zName ) == nNameLen && strncmp( (*ppsTmp)->fn_zName, pzName, nNameLen ) == 0 ) {
	    psNode = *ppsTmp;
			
	    if ( S_ISDIR( psNode->fn_nMode ) == false ) {
		nError = -ENOTDIR;
		psNode = NULL;
		break;
	    }
	    if ( psNode->fn_psFirstChild != NULL ) {
		nError = -ENOTEMPTY;
		psNode = NULL;
		break;
	    }
	    *ppsTmp = psNode->fn_psNextSibling;
	    nError = 0;
	    break;
	}
    }

    if ( NULL != psNode )
    {
	psNode->fn_psParent = NULL;
	psNode->fn_nLinkCount--;

	assert( 0 == psNode->fn_nLinkCount );
		
	if ( false == psNode->fn_bIsLoaded )
	{
	    if ( NULL != psNode->fn_pBuffer ) {
		kfree( psNode->fn_pBuffer );
	    }
	    psVolume->rv_nFSSize -= psNode->fn_nSize;
	    kfree( psNode );
	}
    }
    return( nError );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_unlink( void* pVolume, void* pParentNode, const char* pzName, int nNameLen )
{
    RDVolume_s*	 psVolume = pVolume;
    FileNode_s*  psParent = pParentNode;
    FileNode_s*	 psNode	  = NULL;
    FileNode_s** ppsTmp;
    int		 nError = -ENOENT;
	
    for ( ppsTmp = &psParent->fn_psFirstChild ; NULL != *ppsTmp ; ppsTmp = &((*ppsTmp)->fn_psNextSibling) ) {
	if ( strlen( (*ppsTmp)->fn_zName ) == nNameLen && strncmp( (*ppsTmp)->fn_zName, pzName, nNameLen ) == 0 ) {
	    psNode = *ppsTmp;
	    if ( S_ISDIR( psNode->fn_nMode ) ) {
		nError = -EISDIR;
		psNode = NULL;
		break;
	    }
	    *ppsTmp = psNode->fn_psNextSibling;
	    nError = 0;
	    break;
	}
    }

    if ( NULL != psNode ) {
	psNode->fn_psParent = NULL;
	psNode->fn_nLinkCount--;

	assert( 0 == psNode->fn_nLinkCount );
		
	if ( psNode->fn_bIsLoaded == false ) {
	    if ( NULL != psNode->fn_pBuffer ) {
		kfree( psNode->fn_pBuffer );
	    }
	    psVolume->rv_nFSSize -= psNode->fn_nSize;
	    kfree( psNode );
	}
    }
    return( nError );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_create( void* pVolume, void* pParent, const char* pzName, int nNameLen,
		       int nMode, int nPerms, ino_t* pnInodeNum, void** ppCookie )
{
    RDVolume_s* psVolume = pVolume;
    FileNode_s* psParent = pParent;
    FileNode_s* psNode;
    int		nError = 0;

    LOCK( psVolume->rv_hMutex );

    for ( psNode = psParent->fn_psFirstChild ; psNode != NULL ; psNode = psNode->fn_psNextSibling ) {
	if ( strncmp( psNode->fn_zName, pzName, nNameLen ) == 0 && strlen( psNode->fn_zName ) == nNameLen ) {
	    UNLOCK( psVolume->rv_hMutex );
	    return( -EEXIST );
	}
    }
    
    psNode = rfs_create_node( psParent, pzName, nNameLen, S_IFREG | (nPerms & 0777) );
    if ( NULL != psNode ) {
	*pnInodeNum = psNode->fn_nInodeNum;
	ppCookie = NULL;
    } else {
	nError = -ENOMEM;
    }
    UNLOCK( psVolume->rv_hMutex );
    return( nError );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_symlink( void* pVolume, void* pParentNode, const char* pzName, int nNameLen, const char* pzNewPath )
{
    RDVolume_s* psVolume = pVolume;
    FileNode_s* psParent = pParentNode;
    FileNode_s* psNode;
    int		nError = 0;

    LOCK( psVolume->rv_hMutex );

    for ( psNode = psParent->fn_psFirstChild ; psNode != NULL ; psNode = psNode->fn_psNextSibling ) {
	if ( strncmp( psNode->fn_zName, pzName, nNameLen ) == 0 && strlen( psNode->fn_zName ) == nNameLen ) {
	    UNLOCK( psVolume->rv_hMutex );
	    return( -EEXIST );
	}
    }
    
    psNode = rfs_create_node( psParent, pzName, nNameLen, S_IFLNK | S_IRUGO | S_IWUGO );

    if ( NULL != psNode ) {
	psNode->fn_nSize   = strlen( pzNewPath ) + 1;
	psNode->fn_pBuffer = kmalloc( psNode->fn_nSize , K_MALLOC_FLAGS );

	if ( NULL != psNode->fn_pBuffer ) {
	    strcpy( psNode->fn_pBuffer, pzNewPath );
	    psVolume->rv_nFSSize += psNode->fn_nSize;
	} else {
	    nError = -ENOMEM;
	    rfs_delete_node( psVolume, psNode );
	}
    } else {
	nError = -ENOMEM;
    }
    UNLOCK( psVolume->rv_hMutex );
    return( nError );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_readlink( void* pVolume, void* pNode, char* pzBuf, size_t nBufSize )
{
    FileNode_s* psNode = pNode;

    if ( S_ISLNK( psNode->fn_nMode ) == false ) {
	return( -EINVAL );
    }
    if ( psNode->fn_pBuffer == NULL ) {
	return( 0 );
    }
    if ( nBufSize > psNode->fn_nSize ) {
	nBufSize = psNode->fn_nSize;
    }
    memcpy( pzBuf, psNode->fn_pBuffer, nBufSize );
    return( nBufSize );
}

static int rfs_rename( void* pVolume, void* pOldDir, const char* pzOldName, int nOldNameLen,
		       void* pNewDir, const char* pzNewName, int nNewNameLen, bool bMustBeDir )
{
    RDVolume_s*  psVolume = pVolume;
    FileNode_s*  psOldDir = pOldDir;
    FileNode_s*  psNewDir = pNewDir;
    FileNode_s*	 psNode;
    FileNode_s*  psDstNode;
    int		 nError;
    LOCK( psVolume->rv_hMutex );

    psNode = rfs_find_node( psOldDir, pzOldName, nOldNameLen );
    if ( psNode == NULL ) {
	nError = -ENOENT;
	goto error;
    }
    if ( bMustBeDir && S_ISDIR( psNode->fn_nMode ) == false ) {
	nError = -ENOTDIR;
	goto error;
    }
    psDstNode = rfs_find_node( psNewDir, pzNewName, nNewNameLen );
    if ( psDstNode != NULL ) {
	if ( S_ISDIR( psDstNode->fn_nMode ) ) {
	    nError = -EISDIR;
	    goto error;
	}
	rfs_unlink( pVolume, pNewDir, pzNewName, nNewNameLen );
    }

    if ( psOldDir != psNewDir ) {
	FileNode_s** ppsTmp;
	for ( ppsTmp = &psOldDir->fn_psFirstChild ; NULL != *ppsTmp ; ppsTmp = &((*ppsTmp)->fn_psNextSibling) ) {
	    if ( psNode == *ppsTmp ) {
		*ppsTmp = psNode->fn_psNextSibling;
		break;
	    }
	}
	psNode->fn_psNextSibling = psNewDir->fn_psFirstChild;
	psNewDir->fn_psFirstChild = psNode;
    }
    memcpy( psNode->fn_zName, pzNewName, nNewNameLen );
    psNode->fn_zName[nNewNameLen] = '\0';
    nError = 0;
error:
    UNLOCK( psVolume->rv_hMutex );
    return( nError );
}


/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

static int rfs_mount( dev_t nDevNum, const char* pzDevPath,
		       void** ppVolData, ino_t* pnRootIno )
{
    RDVolume_s* psVolume = kmalloc( sizeof( RDVolume_s ), K_MALLOC_FLAGS );
    FileNode_s*	psRootNode;
    int		nImageFile;
    
    if ( psVolume == NULL ) {
	return( -ENOMEM );
    }
	memset(psVolume,0,sizeof(RDVolume_s));

    psRootNode = kmalloc( sizeof( FileNode_s ), K_MALLOC_FLAGS );
    if ( psRootNode == NULL ) {
	kfree( psVolume );
	return( -ENOMEM );
    }

	memset(psRootNode,0,sizeof(FileNode_s));
    psVolume->rv_hMutex = create_semaphore( "ramfs_mutex", 0 , 1);
    if ( psVolume->rv_hMutex < 0 ) {
	kfree( psVolume );
	kfree( psRootNode );
	return( -ENOMEM );
    }

    psRootNode->fn_nTime    = get_unix_time() ;
    psRootNode->fn_zName[ 0 ] = '\0';
    psRootNode->fn_nLinkCount = 1;

    psRootNode->fn_nMode	= 0777 | S_IFDIR;
    psRootNode->fn_nInodeNum	= RFS_ROOT;

    psRootNode->fn_psNextSibling = NULL;
    psRootNode->fn_psParent	 = NULL;

    psVolume->rv_psRootNode = psRootNode;
    psVolume->rv_nDevNum    = nDevNum;

    *pnRootIno = RFS_ROOT;
    *ppVolData = psVolume;

    if ( pzDevPath != NULL ) {	
    }
    return( 0 );
}

int rfs_rfsstat( void* pVolume, struct stat* psInfo )
{
    RDVolume_s* psVolume = pVolume;
    
    psInfo->st_rdev		= psVolume->rv_nDevNum;
    psInfo->st_ino		= RFS_ROOT;
   // psInfo->st_flags		= 0;
    psInfo->st_blksize	= 512;
    //psInfo->st_io_size		= 65536;
    psInfo->st_blocks	= (psVolume->rv_nFSSize + 511) / 512;
    //psInfo->st_free_blocks	= 0;
    //psInfo->st_free_user_blocks = 0;
    //psInfo->st_total_inodes	= -1;
    //psInfo->st_free_inodes	= -1;
    //strcpy( psInfo->st_volume_name, "ramfs" );
    return( 0 );
}

static void rfs_delete_all_files( RDVolume_s* psVolume, FileNode_s* psRoot )
{
    while ( psRoot->fn_psFirstChild != NULL ) {
	rfs_delete_all_files( psVolume, psRoot->fn_psFirstChild );
    }
    rfs_delete_node( psVolume, psRoot );
}

static int rfs_unmount( void* pVolume )
{
    RDVolume_s* psVolume = pVolume;
    rfs_delete_all_files( psVolume, psVolume->rv_psRootNode );
    destroy_semaphore( psVolume->rv_hMutex );
    kfree( psVolume );
    return( 0 );
}



int ramfs_mount(mount_t *mp, void *_data )
{
	int error;
	ino_t root_ino;
	inode_t *tmp_root_inode;
	char bootblock[512];
	RDVolume_s *vol;
    FileNode_s*	psRootNode;

	error=rfs_mount( mp->m_dev->devno, NULL,
		       &vol, &root_ino );

	if (error)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}

	tmp_root_inode = iget(mp->m_dev->devno, root_ino);
	
	if (!tmp_root_inode)
		goto err1;

	psRootNode = vol->rv_psRootNode;

	tmp_root_inode->i_private_data = psRootNode;

	mp->m_private_data = vol;
	mp->m_root_ino=root_ino;
	mp->m_root=tmp_root_inode;
	mp->m_count ++;
	mp->m_blk_size = 512;
	

	tmp_root_inode->i_father = root_ino;
	tmp_root_inode->i_size = psRootNode->fn_nSize;
	tmp_root_inode->i_ctime = psRootNode->fn_nTime;
	tmp_root_inode->i_atime = psRootNode->fn_nTime;
	tmp_root_inode->i_mtime = psRootNode->fn_nTime;
	tmp_root_inode->i_mode = psRootNode->fn_nMode;

	printf("ramfs_mount =%d,%p\n ", root_ino,psRootNode);


	if(1){
		ino_t pnInodeNum=0;
		void* ppCookie=NULL;

		rfs_mkdir( vol, psRootNode,"pango", 5, 0777 );
		rfs_mkdir( vol, psRootNode,"bin", 3, 0777 );

		error=rfs_create( vol, psRootNode, "hello.txt", strlen("hello.txt"),
				   0666, 0666, &pnInodeNum, &ppCookie );

		printf("rfs_create file node=%d,%d\n",pnInodeNum,error);
		if (!error)
		{
			void *node= rfs_find_node(psRootNode, "hello.txt", strlen("hello.txt"));
			if (node)
			{
		error = rfs_write(vol, node,ppCookie,0,"hello china\n\nrrtt\nyy\nu",20);
		printf("rfs_write file node=%d,%d\n",pnInodeNum,error);
			}
		}

		psRootNode=rfs_find_node(psRootNode, "bin",  3);
		rfs_mkdir( vol, psRootNode,"gtk", 3, 0777 );
	}
	//mp->m_magic = ROMFS_DISK_MAGIC;
	return 0;

err1:
	return error;

}

int ramfs_readdir(file_t* filp, vfs_dirent_t *dirent)
{
	void* pVolume;
	mount_t *mp;
	int err;
	FileNode_s*	pstNode;
	inode_t *inode=filp->f_inode;

	 if (!filp || !dirent)
		 return -1;

	mp=inode->i_super;

	if (!mp)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}

	pstNode=inode->i_private_data;

	if (!pstNode)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}

	pVolume=mp->m_private_data;

	if (!pVolume)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}



	err=rfs_readdir(pVolume,pstNode,NULL,filp->f_pos,dirent,0);
	if (!err)
	{
		//printf("init_ramfs got line%d, filp->f_pos=%d\n",__LINE__,filp->f_pos);
		return -1;
	}
	filp->f_pos++;

	return 0;
}

inode_t* ramfs_opendir(inode_t * inode, char *filepath)
{
	int nNameLen;
	void* pVolume;
	ino_t pnResInode=0;
	void* pParent;
	mount_t *mp;
	inode_t* new;
	int err;
	FileNode_s*	pstNode;

	mp=inode->i_super;
	if (!mp)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return NULL;
	}
	pParent=inode->i_private_data;
	if (!pParent)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return NULL;
	}
	pVolume=mp->m_private_data;
	if (!pVolume)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return NULL;
	}

	nNameLen = strlen(filepath);


	err=rfs_lookup(  pVolume,  pParent, filepath,  nNameLen, &pnResInode );

	if (err!=0)
	{
		//printf("init_ramfs got line%d, %s error\n",__LINE__,filepath);
		return NULL;
	}else{
			
	}


	pstNode=rfs_find_node(pParent, filepath,  nNameLen);
	if (!pstNode)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return NULL;
	}

	//printf("init_ramfs got line%d, %s %p succ\n",__LINE__,filepath,pstNode);

	new= iget(inode->i_dev,pnResInode);
	if (!new)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return NULL;
	}
	new->i_private_data = pstNode;

	new->i_father = inode->i_number;
	new->i_size = pstNode->fn_nSize;
	new->i_ctime = pstNode->fn_nTime;
	new->i_atime = pstNode->fn_nTime;
	new->i_mtime = pstNode->fn_nTime;
	new->i_mode = pstNode->fn_nMode;


	return new;

}

int ramfs_fread(file_t * filp, char * buf, int count)
{
	void* pVolume;
	mount_t *mp;
	int err;
	FileNode_s*	pstNode;
	inode_t *inode=filp->f_inode;


	mp=inode->i_super;


	if (!mp)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return 0;
	}

	pstNode=inode->i_private_data;

	if (!pstNode)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return 0;
	}

	pVolume=mp->m_private_data;

	if (!pVolume)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return 0;
	}

	err=rfs_read( pVolume, pstNode, NULL, filp->f_pos, buf, count );
	if (!err)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
	}
	filp->f_pos+=err;
	return err;
}

int ramfs_write(file_t * filp, char * buf, int count)
{
	void* pVolume;
	mount_t *mp;
	int err;
	FileNode_s*	pstNode;
	inode_t *inode=filp->f_inode;


	mp=inode->i_super;

	if (!mp)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return NULL;
	}

	pstNode=inode->i_private_data;

	if (!pstNode)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return 0;
	}

	pVolume=mp->m_private_data;

	if (!pVolume)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return 0;
	}

	err=rfs_write( pVolume, pstNode, NULL, filp->f_pos, buf, count );
	filp->f_pos+=err;
	
	return err;

}

static int ramfs_mkdir(inode_t* inode, char* filepath,int nPerms)
{
	int nNameLen;
	void* pVolume;
	FileNode_s* pParent;
	mount_t *mp;
	int err;
	//return -1;


	mp=inode->i_super;
	if (!mp)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}
	pParent=inode->i_private_data;
	if (!pParent)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}
	pVolume=mp->m_private_data;
	if (!pVolume)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}

	nNameLen = strlen(filepath);
	err=rfs_mkdir(  pVolume,  pParent, filepath,  nNameLen, 0777 );
	if (err)
	{
	printf("ramfs: %s got line%d %s %p %p error\n",__FUNCTION__, __LINE__,filepath, pVolume,  pParent);
	}
	return err;
}

static int ramfs_create_file(inode_t* inode, char* filepath,int nPerms)
{
	int nNameLen;
	void* pVolume;
	FileNode_s* pParent;
	mount_t *mp;
	int err;
	ino_t ino;


	mp=inode->i_super;
	if (!mp)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}
	pParent=inode->i_private_data;
	if (!pParent)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}
	pVolume=mp->m_private_data;
	if (!pVolume)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}


	nNameLen = strlen(filepath);
	err = rfs_create(  pVolume,  pParent, filepath,  nNameLen,0, nPerms,&ino,NULL );
	printf("%s: %s got line%d err=%d\n",__FUNCTION__,filepath, __LINE__,err);
	return err;
}


int ramfs_unlink2 (inode_t* inode, const char* filepath){
	int nNameLen;
	void* pVolume;
	FileNode_s* pParent;
	mount_t *mp;
	int err;

	//printf("ramfs: %s got line%d %s\n",__FUNCTION__, __LINE__,filepath);

	mp=inode->i_super;
	if (!mp)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}
	pParent=inode->i_private_data;
	if (!pParent)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}
	pVolume=mp->m_private_data;
	if (!pVolume)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}

	nNameLen = strlen(filepath);
	err=rfs_unlink(  pVolume,  pParent, filepath,  nNameLen );
	return err;
}

int ramfs_link2 (inode_t* inode, const char* filepath,const char *pathnew){
	int nNameLen;
	void* pVolume;
	FileNode_s* pParent;
	mount_t *mp;
	int err;


	mp=inode->i_super;
	if (!mp)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}
	pParent=inode->i_private_data;
	if (!pParent)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}
	pVolume=mp->m_private_data;
	if (!pVolume)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}

	nNameLen = strlen(filepath);
	err=rfs_symlink(  pVolume,  pParent, filepath,  nNameLen ,pathnew);
	if (err)
	{
	printf("ramfs: %s got line%d %s,%s error%d\n",__FUNCTION__, __LINE__,filepath,pathnew,err);
	}
	return err;
}


int ramfs_rmdir (inode_t* inode, const char* filepath){
	int nNameLen;
	void* pVolume;
	FileNode_s* pParent;
	mount_t *mp;
	int err;

	mp=inode->i_super;
	if (!mp)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}
	pParent=inode->i_private_data;
	if (!pParent)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}
	pVolume=mp->m_private_data;
	if (!pVolume)
	{
		printf("ramfs: %s got line%d\n",__FUNCTION__, __LINE__);
		return -1;
	}

	nNameLen = strlen(filepath);
	err=rfs_rmdir(  pVolume,  pParent, filepath,  nNameLen );
	return err;
}


/********************************************/

static  fs_dev_ops_t _ramfs=
{
	fs_name: "ramfs",
	fs_copyright:"BSDL",
	fs_author:"Easion",
	fs_bmap: NULL,
	fs_opendir: ramfs_opendir,

	fs_readdir: ramfs_readdir,
	fs_probe:NULL,
	fs_mount:ramfs_mount,
	fs_unmount:rfs_unmount,
	fs_read:ramfs_fread,
	fs_write:ramfs_write,
	fs_mkdir:ramfs_mkdir,
	fs_rmdir:ramfs_rmdir,
	fs_unlink:ramfs_unlink2,
	fs_symlink:ramfs_link2,
	fs_creat:ramfs_create_file,
};


void ramfs_hook()
{
	install_fs(&_ramfs);
}

void remove_ramfs_hook()
{
	deinstall_fs(&_ramfs);
}


