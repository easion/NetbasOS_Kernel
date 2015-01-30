#include <drv/drv.h>
#include <drv/spin.h>
//#include <drv/cpplib.h>
#include <drv/timer.h>
#include <drv/ia.h>
#include <drv/vm86.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "util.h"

CREATE_SPINLOCK( floppy_sem );
#define MAX_BIOSDISK_MAJOR 5
int biosdisk_open(const char *f, int mode,dev_prvi_t*);
int biosdisk_close(dev_prvi_t*  file);
int biosdisk_drv_read(dev_prvi_t* minor, off_t  pos, void * buf,size_t count);
int biosdisk_drv_write(dev_prvi_t* minor, off_t  pos, const void * buf,size_t count);
int biosdisk_drv_ioctl( dev_prvi_t* handler, u32_t nCommand, void* pArgs,int len, bool bFromKernel );
static  driver_ops_t kops;

int cmos_read (int pos)
{
    outb (0x70, pos);
    asm ("jmp 1f\n1:");
    return inb (0x71);
}

static void init_driver()
{

	memset(&kops,0,sizeof(kops));
	kops.d_name = "bhd";
	kops.d_author = "Easion";
	kops.d_version = "version 0.1";
	kops.d_index = -1;
	kops.d_kver = CURRENT_KERNEL_VERSION;
	kops.open = biosdisk_open;
	kops.close = biosdisk_close;
	kops.read = biosdisk_drv_read;
	kops.write = biosdisk_drv_write;
	kops.ioctl = biosdisk_drv_ioctl;
	kops.d_data = NULL;

	kernel_driver_register(dev);
}




int biosdisk_open(const char *f, int mode,dev_prvi_t*fp)
{
	
	return 0;
}

int biosdisk_close(dev_prvi_t*  file)
{
	return 0;
}



int init_bdd();


int dll_main(char **argv)
{
	int i,err;
	biosdisk_drv_t *d ;
	char buf[512];

	init_driver();
	irq_init();
	init_bdd();	
	
	kprintf("biosdisk_stdread ...\n");
	d = get_biosdisk_args(0x200);
	for (i=0; i<3; i++)
	{
		err = biosdisk_stdread(d,0,1,buf);
		if (err == 0)
		{
			break;
		}
		//biosdisk_test();
	}
	kprintf("biosdisk_stdread %s ok\n", &buf[0x36]);
	return 0;
}


int dll_destroy()
{
	//deinit();
	remove_timer(&g_floppy_timer);
	return 0;
}

int dll_version()
{
	printk("JICAMA OS fdc VERSION 0.01!\n");
	return 0;
}




int biosdisk_drv_read( dev_prvi_t* handler,  off_t nPos, void* pBuf, size_t nLen )
{
	int count;
    biosdisk_drv_t*  Disk  = get_biosdisk_args(handler);

	//kprintf("disk read %x pos %d len %d...\n", handler,nPos,nLen);
	count = biosdisk_read(Disk, nPos,pBuf,nLen);
	//kprintf("disk read  %d bytes...\n", count);
	return count;

}

int decode_disk_partitions( device_geometry *psDiskGeom, 
	Partition_s * pasPartitions, int nMaxPartitions, void *pCookie,
	disk_read_op *pfReadCallback )
{
	char anBuffer[512];
	PartitionRecord_s *pasTable = ( PartitionRecord_s * ) ( anBuffer + 0x1be );
	int i;
	int nCount = 0;
	int nRealCount = 0;
	off_t nDiskSize = psDiskGeom->sector_count * psDiskGeom->bytes_per_sector;
	off_t nTablePos = 0;
	off_t nExtStart = 0;
	off_t nFirstExtended = 0;
	int nNumExtended;
	int nNumActive;
	int nError;

      repeat:
	if ( pfReadCallback( pCookie, nTablePos, anBuffer, 512 ) != 512 )
	{
		printk( "Error: decode_disk_partitions() failed to read MBR\n" );
		nError = 0;	//-EINVAL;
		goto error;
	}
	if ( *( ( u16_t * )( anBuffer + 0x1fe ) ) != 0xaa55 )
	{
		printk( "Error: decode_disk_partitions() Invalid partition table signature %04x\n", *( ( u16_t * )( anBuffer + 0x1fe ) ) );
		nError = 0;	//-EINVAL;
		goto error;
	}

	nNumActive = 0;
	nNumExtended = 0;

	for ( i = 0; i < 4; ++i )
	{
		if ( pasTable[i].p_nStatus & 0x80 )
		{
			nNumActive++;
		}
		if ( pasTable[i].p_nType == 0x05 || pasTable[i].p_nType == 0x0f || pasTable[i].p_nType == 0x85 )
		{
			nNumExtended++;
		}
		if ( nNumActive > 1 )
		{
			printk( "Warning: decode_disk_partitions() more than one active partition\n" );
		}
		if ( nNumExtended > 1 )
		{
			printk( "Error: decode_disk_partitions() more than one extended partition\n" );
			nError = -EINVAL;
			goto error;
		}
	}
	for ( i = 0; i < 4; ++i )
	{
		int j;

		if ( nCount >= nMaxPartitions )
		{
			break;
		}
		if ( pasTable[i].p_nType == 0 )
		{
			continue;
		}
		if ( pasTable[i].p_nType == 0x05 || pasTable[i].p_nType == 0x0f || pasTable[i].p_nType == 0x85 )
		{
			nExtStart = ( ( S64_t )pasTable[i].p_nStartLBA ) * ( ( S64_t )psDiskGeom->bytes_per_sector );	// + nTablePos;
			if ( nFirstExtended == 0 )
			{
				memset( &pasPartitions[nCount], 0, sizeof( pasPartitions[nCount] ) );
				nCount++;
			}
			continue;
		}

		pasPartitions[nCount].p_nType = pasTable[i].p_nType;
		pasPartitions[nCount].p_nStatus = pasTable[i].p_nStatus;
		pasPartitions[nCount].p_nStart = ( ( S64_t )pasTable[i].p_nStartLBA ) * ( ( S64_t )psDiskGeom->bytes_per_sector ) + nTablePos;
		pasPartitions[nCount].p_nSize = ( ( S64_t )pasTable[i].p_nSize ) * ( ( S64_t )psDiskGeom->bytes_per_sector );

		if ( pasPartitions[nCount].p_nStart + pasPartitions[nCount].p_nSize > nDiskSize )
		{
			printk( "Error: Partition %d extends outside the disk/extended partition\n", nCount );
			nError = -EINVAL;
			goto error;
		}

		for ( j = 0; j < nCount; ++j )
		{
			if ( pasPartitions[nCount].p_nType == 0 )
			{
				continue;
			}
			if ( pasPartitions[j].p_nStart + pasPartitions[j].p_nSize > 
				pasPartitions[nCount].p_nStart 
				&& pasPartitions[j].p_nStart <
				pasPartitions[nCount].p_nStart + pasPartitions[nCount].p_nSize )
			{
				printk( "Error: decode_disk_partitions() partition %d overlaps partition %d\n", j, nCount );
				nError = -EINVAL;
				goto error;
			}
			if ( ( pasPartitions[nCount].p_nStatus & 0x80 ) != 0 
				&& ( pasPartitions[j].p_nStatus & 0x80 ) != 0 )
			{
				printk( "Error: decode_disk_partitions() more than one active partition\n" );
				nError = -EINVAL;
				goto error;
			}
			if ( pasPartitions[nCount].p_nType == 0x05 && 
				pasPartitions[j].p_nType == 0x05 )
			{
				printk( "Error: decode_disk_partitions() more than one extended partition\n" );
				nError = -EINVAL;
				goto error;
			}
		}
		nCount++;
		nRealCount++;
	}
	if ( nExtStart != 0 )
	{
		nTablePos = nFirstExtended + nExtStart;
		if ( nFirstExtended == 0 )
		{
			nFirstExtended = nExtStart;
		}
		nExtStart = 0;
		if ( nCount < 4 )
		{
			while ( nCount & 0x03 )
			{
				memset( &pasPartitions[nCount++], 0, sizeof( Partition_s ) );
			}
		}
		goto repeat;
	}
	return ( nCount );
      error:
	if ( nCount < 4 || nRealCount == 0 )
	{
		return ( nError );
	}
	else
	{
		return ( nCount & ~3 );
	}
}

static size_t biosdisk_read_partition_data( void* pCookie, 
	off_t n_off, void* pbuf, size_t nSize )
{
    return( biosdisk_read( pCookie,  n_off, pbuf, nSize ) );
}

/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

int biosdisk_drv_write( dev_prvi_t* handler, off_t nPos, const void* pBuf, size_t nLen )
{
    biosdisk_drv_t*  Disk  = get_biosdisk_args(handler);
    int nBytesLeft;
    int nError;

    if ( nLen & (512 - 1) ) {
	printk( "Error: biosdisk_write() length has bad alignment %d\n", nLen );
	return( -EINVAL );
    }

    if ( nPos >= Disk->d_Size ) {
	printk( "Warning: biosdisk_write() Request outside partiton : %Ld\n", nPos );
	return( 0 );
    }
    if ( nPos + nLen/512+1 > Disk->d_Size ) {
	printk( "Warning: biosdisk_write() Request truncated from %d to %Ld\n", nLen, (Disk->d_Size - nPos) );
	nLen = (Disk->d_Size - nPos)*512;
    }
    nBytesLeft = nLen;
    nPos += Disk->d_Start;
  
    while ( nBytesLeft > 0 ) {
	int nCurSize = min( BDD_BUFFER_SIZE, nBytesLeft );
	//LOCK( g_hRelBufLock );
	memcpy( g_databuf, pBuf, nCurSize );
	if ( Disk->d_bCSHAddressing ) {
	    nError = bd_csh_write_sectors( Disk, g_databuf, nPos , nCurSize/512  );
	} else {
	    nError = bd_lba_write_sectors( Disk, g_databuf, nPos , nCurSize/512  );
	}
	//UNLOCK( g_hRelBufLock );
	if ( nError < 0 ) {
	    printk( "biosdisk_write() failed to write %d sectors to drive %x\n",
		    (nCurSize ), Disk->d_DriveNum );
	    return( nError );
	}
	nCurSize = nError * 512;
	pBuf = ((u8_t*)pBuf) + nCurSize;
	nPos       += nError;
	nBytesLeft -= nCurSize;
    }
    return( nLen );
}

static int biosdisk_read( biosdisk_drv_t*  Disk,  off_t nPos, void* pBuf, size_t nLen )
{
    int nError;
    int nBytesLeft;
  
    if ( nLen & (512 - 1) ) {
	printk( "Error: biosdisk_read() length has bad alignment %d\n", nLen );
	return( -EINVAL );
    }
    if ( nPos >= Disk->d_Size ) {
	printk( "Warning: biosdisk_read() Request outside partiton %Ld\n", nPos );
	return( 0 );
    }
    if ( nPos + nLen/512 > Disk->d_Size ) {
	printk( "Warning: biosdisk_read() Request truncated from %d to %Ld\n", nLen, (Disk->d_Size - nPos) );
	nLen = (Disk->d_Size - nPos)*512;
    }
    nBytesLeft = nLen;
    nPos += Disk->d_Start;
  
    while ( nBytesLeft > 0 ) {
	int nCurSize = min( BDD_BUFFER_SIZE, nBytesLeft );
    
	if ( nPos < 0 ) {
	    printk( "biosdisk_read() vierd pos = %Ld\n", nPos );
	    assert(0);
	}

	//LOCK( g_hRelBufLock );
	if ( Disk->d_bCSHAddressing ) {
		//printf("try bd_csh_read_sectors ..\n");
	    nError = bd_csh_read_sectors( Disk, g_databuf, (nPos  ), nCurSize/512  );
	} else {
		printf("try bd_lba_read_sectors ..\n");
	    nError = bd_lba_read_sectors( Disk, g_databuf, (nPos ), nCurSize/512  );
	}
	memcpy( pBuf, g_databuf, nCurSize );
	//UNLOCK( g_hRelBufLock );
	if ( nError < 0 ) {
	    printk( "biosdisk_read() failed to read %d sectors from drive %x\n",
		    (nCurSize ), Disk->d_DriveNum );
	    return( nError );
	}
	nCurSize = nError * 512;
	pBuf = ((u8_t*)pBuf) + nCurSize;
	nPos       += nError;
	nBytesLeft -= nCurSize;
    }
    return( nLen );
}


int biosdisk_decode_partitions( biosdisk_drv_t* Disk )
{
    int		    nNumPartitions;
    device_geometry sDiskGeom;
    Partition_s     asPartitions[16];
    biosdisk_drv_t*     psPartition;
    biosdisk_drv_t**    ppsTmp;
    int		    nError;
    int		    i;

    if ( Disk->d_DriveNum < 0x80 || Disk->d_PartitionType != 0 ) {
	return( -EINVAL );
    }

    sDiskGeom.sector_count      = Disk->d_Size ;
    sDiskGeom.cylinder_count    = Disk->d_Cylinders;
    sDiskGeom.sectors_per_track = Disk->d_Sectors;
    sDiskGeom.head_count	= Disk->d_Heads;
    sDiskGeom.bytes_per_sector  = 512;
    sDiskGeom.read_only 	= false;
    sDiskGeom.removable 	= Disk->d_bRemovable;

    printk( "Decode partition table for %s\n", Disk->bi_zName );
    
    nNumPartitions = decode_disk_partitions( &sDiskGeom, asPartitions, 16, Disk, biosdisk_read_partition_data );

    if ( nNumPartitions < 0 ) {
	printk( "   Invalid partition table\n" );
	return( nNumPartitions );
    }
    for ( i = 0 ; i < nNumPartitions ; ++i ) {
	if ( asPartitions[i].p_nType != 0 && asPartitions[i].p_nSize != 0 ) {
	    printk( "   Partition %d : %10Ld -> %10Ld %02x (%Ld)\n", i, asPartitions[i].p_nStart,
		    asPartitions[i].p_nStart + asPartitions[i].p_nSize - 1LL, asPartitions[i].p_nType,
		    asPartitions[i].p_nSize );
	}
    }
    //LOCK( g_hRelBufLock );
    nError = 0;

    for ( psPartition = Disk->bi_psFirstPartition ; psPartition != NULL ; psPartition = psPartition->bi_psNext ) {
	bool bFound = false;
	for ( i = 0 ; i < nNumPartitions ; ++i ) {
	    if ( asPartitions[i].p_nStart == psPartition->d_Start && asPartitions[i].p_nSize == psPartition->d_Size*512 ) {
		bFound = true;
		break;
	    }
	}
	if ( bFound == false){// && atomic_read( &psPartition->d_OpenCount ) > 0 ) {
	    printk( "biosdisk_decode_partitions() Error: Open partition %s on %s has changed\n", psPartition->bi_zName, Disk->bi_zName );
	    nError = -EBUSY;
	    goto error;
	}
    }

      // Remove deleted partitions from /dev/disk/bios/*/*
    for ( ppsTmp = &Disk->bi_psFirstPartition ; *ppsTmp != NULL ; ) {
	bool bFound = false;
	psPartition = *ppsTmp;
	for ( i = 0 ; i < nNumPartitions ; ++i ) {
	    if ( asPartitions[i].p_nStart == psPartition->d_Start && asPartitions[i].p_nSize == psPartition->d_Size ) {
		asPartitions[i].p_nSize = 0;
		psPartition->d_PartitionType = asPartitions[i].p_nType;
		sprintf( psPartition->bi_zName, "%d", i );
		bFound = true;
		break;
	    }
	}
	if ( bFound == false ) {
	    *ppsTmp = psPartition->bi_psNext;
	    //delete_device_node( psPartition->d_NodeHandle );
	    kfree( psPartition );
	} else {
	    ppsTmp = &(*ppsTmp)->bi_psNext;
	}
    }

      // Create nodes for any new partitions.
    for ( i = 0 ; i < nNumPartitions ; ++i ) {

	if ( asPartitions[i].p_nType == 0 || asPartitions[i].p_nSize == 0 ) {
	    continue;
	}

	psPartition = kmalloc( sizeof( biosdisk_drv_t ), 0 );

	if ( psPartition == NULL ) {
	    printk( "Error: biosdisk_decode_partitions() no memory for partition inode\n" );
	    break;
	}

	sprintf( psPartition->bi_zName, "%d", i );
	psPartition->d_DeviceHandle  = Disk->d_DeviceHandle;
	psPartition->d_DriveNum      = Disk->d_DriveNum;
	psPartition->d_Sectors       = Disk->d_Sectors;
	psPartition->d_Cylinders     = Disk->d_Cylinders;
	psPartition->d_Heads	       = Disk->d_Heads;
	psPartition->d_SectorSize    = Disk->d_SectorSize;
	psPartition->d_bCSHAddressing = Disk->d_bCSHAddressing;
	psPartition->d_bRemovable     = Disk->d_bRemovable;
	psPartition->d_bLockable      = Disk->d_bLockable;
	psPartition->d_bHasChangeLine = Disk->d_bHasChangeLine;

	psPartition->d_Start = asPartitions[i].p_nStart;
	psPartition->d_Size  = asPartitions[i].p_nSize;

	psPartition->bi_psNext = Disk->bi_psFirstPartition;
	Disk->bi_psFirstPartition = psPartition;
	

	//psPartition->d_NodeHandle = create_device_node( g_nDevID, psPartition->d_DeviceHandle, zNodePath, &g_sOperations, psPartition );
    }

      /* We now have to rename nodes that might have moved around in the table and
       * got new names. To avoid name-clashes while renaming we first give all
       * nodes a unique temporary name before looping over again giving them their
       * final names
       */
    
    for ( psPartition = Disk->bi_psFirstPartition ; psPartition != NULL ; psPartition = psPartition->bi_psNext ) {
	
	//rename_device_node( psPartition->d_NodeHandle, zNodePath );
    }
    for ( psPartition = Disk->bi_psFirstPartition ; psPartition != NULL ; psPartition = psPartition->bi_psNext ) {
	
	//rename_device_node( psPartition->d_NodeHandle, zNodePath );
    }
    
error:
    //UNLOCK( g_hRelBufLock );
    return( nError );
}

int biosdisk_drv_ioctl( dev_prvi_t* handler, u32_t nCommand, void* pArgs,int len, bool bFromKernel )
{
    biosdisk_drv_t*  Disk  = get_biosdisk_args(handler);
    int nError = 0;
    
    switch( nCommand )
    {
	case IOCTL_GET_DEVICE_GEOMETRY:
	{
	    device_geometry sGeo;
      
	    sGeo.sector_count      = Disk->d_Size ;
	    sGeo.cylinder_count    = Disk->d_Cylinders;
	    sGeo.sectors_per_track = Disk->d_Sectors;
	    sGeo.head_count	   = Disk->d_Heads;
	    sGeo.bytes_per_sector  = 512;
	    sGeo.read_only 	   = false;
	    sGeo.removable 	   = Disk->d_bRemovable;
	    if ( bFromKernel ) {
		memcpy( pArgs, &sGeo, sizeof(sGeo) );
	    } 
		else {
		nError = memcpy_to_user( pArgs, &sGeo, sizeof(sGeo) );
	    }
	    break;
	}
	case IOCTL_REREAD_PTABLE:
	    nError = biosdisk_decode_partitions( Disk );
	    break;
	default:
	    printk( "Error: biosdisk_ioctl() unknown command %ld\n", nCommand );
	    nError = -ENOSYS;
	    break;
    }
    return( nError );
}


biosdisk_drv_t* setup_disk_param(char *name, int drv,int i, bool bCHS, drive_param_t *param)
{
	biosdisk_drv_t*Disk = kmalloc( sizeof( biosdisk_drv_t ), 0);

	if ( Disk == NULL ) {
	    return NULL;
	}
	strcpy( Disk->bi_zName, name );
	Disk->d_DeviceHandle  = drv;
	Disk->d_DriveNum      = i;
	Disk->d_Sectors       = param->nSectors;
	Disk->d_Cylinders     = param->nCylinders;
	Disk->d_Heads	   = param->nHeads;
	Disk->d_SectorSize    = param->nBytesPerSector;
	Disk->d_Start	   = 0;
	Disk->d_Size	   = param->nTotSectors;// * param->nBytesPerSector;
	Disk->d_bRemovable     = (param->nFlags & DIF_REMOVABLE) != 0;
	Disk->d_bLockable      = (param->nFlags & DIF_CAN_LOCK) != 0;
	Disk->d_bHasChangeLine = (param->nFlags & DIF_HAS_CHANGE_LINE) != 0;
	Disk->d_bCSHAddressing = bCHS;
    
	printk( "setup_disk_param(): /dev/%s :"
	"(%x) %d sectors of %d bytes (%s) (%d:%d:%d)\n", 
		name,Disk->d_DriveNum, param->nTotSectors, param->nBytesPerSector,
		((bCHS)!=NULL ? "CSH" : "LBA"),
		Disk->d_Heads, Disk->d_Cylinders, Disk->d_Sectors );	

	if ( Disk->d_bRemovable ) {
	    printk( "Drive %s is removable\n", name );
	}
	Disk->d_NodeHandle = 0;
	return Disk;
}

int biosdisk_scan_for_disks()
{
    biosdisk_drv_t* Disk;
    int	      nError;
    int	      i;
    int       nHandle;
	driver_ops_t*dev;
    bool		bCHS;
	long nFlags=0 ;
    
   
    printk( "Scan floppy-drives exported ...\n" );
    for ( i = 0x00 ; i < 0x02 ; ++i )
    {
	drive_param_t sDriveParams;
	char	      zName[16];
	if ( bd_drive_params_csh( i, &sDriveParams ) < 0 || sDriveParams.nTotSectors == 0 )  {
	    continue;
	}
	sprintf( zName, "fd%c", 'a' + i );

	printk("kernel_driver_register floppy %s\n", zName);

	nFlags = bd_drive_capabilities( i );

	kprintf("nFlags = %x ...\n", nFlags);

	dev =  alloc_biosdisk_device();
	assert(dev);
	dev->d_name = kmalloc(strlen(zName)+1,0);
	strcpy(dev->d_name,zName);
	dev->d_index = 0x200+i;
	dev->d_data = setup_disk_param(zName,i,i,1,&sDriveParams);
	nError = kernel_driver_register(dev);

	if (nError)
	{
		printf("register error\n");
	}
	
	//nHandle = register_device( "", "bios" );
	//claim_device( nDeviceID, nHandle, "Floppy drive", DEVICE_DRIVE );

	//nError = biosdisk_create_node(  nHandle, zNodePath, zName, i, sDriveParams.nSectors, sDriveParams.nCylinders, sDriveParams.nHeads,
	//			  sDriveParams.nBytesPerSector, 0, sDriveParams.nTotSectors * sDriveParams.nBytesPerSector, true );
    }

    printk( "Scan hard-drives ...\n" );
    for ( i = 0x80 ; i < 0x90 ; ++i ) {
	drive_param_t sDriveParams;
	char	      zName[16];
	char	      zMode[64];
	bool	      bForceCSH = false;
	bool	      bForceLBA = false;
	u32_t	      nFlags;
	int	      j;
	bool		  bForceBios = false;
	
	sprintf( zName, "hd%d", i - 0x80 );

	bForceBios = true;	
	//bForceBios = false;	

	if( bForceBios == false )
		continue;
	else {
		//printk( "BIOS controller enabled for drive %s\n", zName );
		//nHandle = register_device( "", "bios" );
		//claim_device( nDeviceID, nHandle, "Harddisk", DEVICE_DRIVE );
	}


	//bForceLBA = true;
	bForceCSH = true;	
	nFlags = bd_drive_capabilities( i );
    
	if ( bForceCSH == false && ((nFlags & BCF_EXTENDED_DISK_ACCESS) || bForceLBA) &&
	     bd_drive_params_lba( i, &sDriveParams ) >= 0 && sDriveParams.nTotSectors > 0 ) {
	    bCHS = false;
	} 
	else if ( bd_drive_params_csh( i, &sDriveParams ) >= 0 && sDriveParams.nTotSectors > 0 ) {
	    bCHS = true;
	} 
	else {
	    continue;
	}
	Disk = setup_disk_param(zName,nHandle,i,bCHS,&sDriveParams);
	/*Disk = kmalloc( sizeof( biosdisk_drv_t ), 0);

	if ( Disk == NULL ) {
	    nError = -ENOMEM;
	    goto error;
	}

	strcpy( Disk->bi_zName, zName );
	Disk->d_DeviceHandle  = nHandle;
	Disk->d_DriveNum      = i;
	Disk->d_Sectors       = sDriveParams.nSectors;
	Disk->d_Cylinders     = sDriveParams.nCylinders;
	Disk->d_Heads	   = sDriveParams.nHeads;
	Disk->d_SectorSize    = sDriveParams.nBytesPerSector;
	Disk->d_Start	   = 0;
	Disk->d_Size	   = sDriveParams.nTotSectors * sDriveParams.nBytesPerSector;
	Disk->d_bRemovable     = (sDriveParams.nFlags & DIF_REMOVABLE) != 0;
	Disk->d_bLockable      = (sDriveParams.nFlags & DIF_CAN_LOCK) != 0;
	Disk->d_bHasChangeLine = (sDriveParams.nFlags & DIF_HAS_CHANGE_LINE) != 0;
	Disk->d_bCSHAddressing = bCHS;
    
	printk( "/dev/%s : (%02x) %d sectors of %d bytes (%s) (%d:%d:%d)\n",
		zName,		Disk->d_DriveNum, sDriveParams.nTotSectors,
		sDriveParams.nBytesPerSector,
		((bCHS) ? "CSH" : "LBA"),
		Disk->d_Heads, Disk->d_Cylinders, Disk->d_Sectors );	*/
	
	if ( Disk->d_bRemovable ) {
	    printk( "Drive %s is removable\n", zName );
	}


	Disk->d_NodeHandle = nError;
	dev =  alloc_biosdisk_device();
	dev->d_name = kmalloc(strlen(zName)+1,0);
	strcpy(dev->d_name,zName);
	dev->d_index = 0x300+(i-0x80);
	dev->d_data = Disk;
	kernel_driver_register(dev);
	printf("register %s ...\n",dev->d_name);

    }
	//panic("register ok ..\n");

	read_partition();

    return( 0 );
error:
	panic("error happend  ..\n");
    return( nError );
}



int init_bdd(  )
{
    g_cmdbuf = low_alloc( 512 );
  
    if ( g_cmdbuf == NULL ) {
	printk( "Error: init_bdd() failed to alloc command buffer\n" );
	return( -ENOMEM );
    }
    g_rawdata = (u8_t*) low_alloc( BDD_BUFFER_SIZE + 65535 );
    if ( g_rawdata == NULL ) {
	printk( "Error: init_bdd() failed to alloc io buffer\n" );
	low_free( g_cmdbuf ,512);
	return( -ENOMEM );
    }
    g_databuf = (u8_t*) (((u32_t)g_rawdata + 65535) & ~65535);
    
    //g_floppy_timer = create_timer();
	init_timer(&g_floppy_timer, floppy_off_timer,NULL);
	install_timer(&g_floppy_timer,5000);
    biosdisk_scan_for_disks();
    return( 0 );
}

