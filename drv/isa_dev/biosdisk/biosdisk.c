
#include <drv/drv.h>
//#include <drv/cpplib.h>
#include <drv/timer.h>
#include <drv/ia.h>
#include <drv/vm86.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "util.h"
char*	g_cmdbuf;
char*	g_databuf;
char*	g_rawdata; // Same as g_databuf but is not aligned to 16 byte boundary
krnl_timer_t g_floppy_timer;

void floppy_off_timer( void* pData )
{
    //outportb(  0x3f2,0x0c ); // Stop the floppy motor.
}
#if 1
int bd_mediachange(int bios_number)
{
    vm86regs_t reg86;

    memset( &reg86, 0, sizeof( reg86 ) );
    reg86.h.ah	= 0x16;
    reg86.h.dl	= bios_number;
    reg86.x.si	= 0x0000;

	do_int86( 0x13, &reg86,&reg86 );

   
    if (!(reg86.x.flags & 0x1)) return 0; /* Carry clear is disk not changed */
    if (reg86.x.flags == 0x06) return 1; /* Disk changed */
    return -1; /* If AH is not 06h an error occurred */
}

int bd_reset(int bios_number)
{
    vm86regs_t reg86;

    memset( &reg86, 0, sizeof( reg86 ) );
    reg86.h.ah	= 0x00;
    reg86.h.dl	= bios_number;

	do_int86( 0x13, &reg86,&reg86 );

    if (reg86.x.flags & 0x1) return -1; /* Error code in AH */
    return 0;
}

int bd_get_disk_type(unsigned bios_number)
{
    vm86regs_t reg86;

    memset( &reg86, 0, sizeof( reg86 ) );
    reg86.h.ah	= 0x15;
    reg86.h.dl	= bios_number;

	do_int86( 0x13, &reg86,&reg86 );
   
    if (reg86.x.flags & 0x1) return -1; /* FIX ME: Convert BIOS error codes! */
    return reg86.h.ah;
}

int bd_drive_capabilities( int nDrive )
{
    vm86regs_t reg86;

    memset( &reg86, 0, sizeof( reg86 ) );

    reg86.x.ax	= 0x4100;
    reg86.x.bx	= 0x55AA;
    reg86.x.dx	= nDrive;

    do_int86( 0x13, &reg86,&reg86 );

    if ( reg86.x.flags & 0x01 ) {
	return( 0 );
    }
    if ( (reg86.x.bx & 0xffff) != 0xAA55 ) {
	return( 0 );
    }
    return( reg86.x.cx & 0xffff );
}



int bd_drive_params_csh( int nDrive, drive_param_t* psParams )
{
    vm86regs_t reg86;

    memset( &reg86, 0, sizeof( reg86 ) );

    reg86.x.ax	= 0x0800;
    reg86.x.dx	= nDrive;


    do_int86( 0x13, &reg86,&reg86 );
    
    psParams->nStructSize     = sizeof(drive_param_t);
    psParams->nFlags	      = 0;
    psParams->nCylinders      = (((reg86.x.cx >> 8) & 0xff) | ((reg86.x.cx & 0xc0) << 2)) + 1;
    psParams->nHeads	      = ((reg86.x.dx >> 8) & 0xff) + 1;
    psParams->nSectors	      = reg86.x.cx & 0x3f;
    psParams->nTotSectors     = psParams->nCylinders * psParams->nHeads * psParams->nSectors;
    psParams->nBytesPerSector = 512;
	if(psParams->nTotSectors)
	printk("bd_drive_params_csh 0x%x sec%d...\n", nDrive,psParams->nTotSectors);
/*    if ( nDrive < 2 ) {
	switch( reg86.x.bx & 0xff )
	{
	    case 3:
		psParams->nCylinders = 40;
		psParams->nHeads     = 2;
		psParams->nSectors   = 18;
		break;
	    case 4:
		psParams->nCylinders = 80;
		psParams->nHeads     = 2;
		psParams->nSectors   = 18;
		break;
	}
	psParams->nTotSectors     = psParams->nCylinders * psParams->nHeads * psParams->nSectors;
	psParams->nBytesPerSector = 512;
    }*/

	if( ! reg86.x.flags & 0x01 )
	printk( "Number of drives = 0x%x 0x%x\n", (reg86.x.dx & 0xff),reg86.x.flags );
  
    return( ( reg86.x.flags & 0x01 ) ? -1 : 0 );
}



int bd_drive_params_lba( int nDrive, drive_param_t* psParams )
{
    vm86regs_t reg86;

//    if ( bd_drive_params_csh( nDrive, psParams ) < 0 ) {
//	return( -1 );
//    }
    
    memset( &reg86, 0, sizeof( reg86 ) );

    reg86.x.ax	= 0x4800;
    reg86.x.dx	= nDrive;

    reg86.x.ds	= ((unsigned int) (g_cmdbuf) ) >> 4;
    reg86.x.si	= ((unsigned int) (g_cmdbuf) ) & 0x0f;

    (*(u16_t*)(g_cmdbuf)) = 0x2c;	/* Size */

    do_int86( 0x13, &reg86,&reg86 );
  
    psParams->nStructSize = (*(u16_t*)(g_cmdbuf + 0));
    psParams->nFlags      = (*(u16_t*)(g_cmdbuf + 2));
    psParams->nCylinders  = (*(u32_t*)(g_cmdbuf + 4));
    psParams->nHeads      = (*(u32_t*)(g_cmdbuf + 8));
    psParams->nSectors    = (*(u32_t*)(g_cmdbuf + 12));
    psParams->nTotSectors = (*(u64_t*)(g_cmdbuf + 16));
    psParams->nBytesPerSector  = (*(u16_t*)(g_cmdbuf + 24));

    return( ( reg86.x.flags & 0x01 ) ? -1 : 0 );
}


/*****************************************************************************
 * NAME:
 * DESC:
 * NOTE:
 * SEE ALSO:
 ****************************************************************************/

int bd_lba_read_sectors( biosdisk_drv_t* Disk, void* pbuf, S64_t sector_pos, int Count )
{
    vm86regs_t reg86;
    int	 seg = ((unsigned int) (pbuf) ) >> 4;
    int	 off = ((unsigned int) (pbuf) ) & 0x0f;
    int  nRetryCount = 0;
	int my_sectors = ((int)sector_pos+Count-1) / Disk->d_Sectors;
	int my_sectors2 = (int)sector_pos / Disk->d_Sectors;
    
    
    if ( Disk->d_bTruncateToCyl && Disk->d_Sectors > 1 ) {
		if (my_sectors2  != my_sectors ) {
			Count = Disk->d_Sectors - sector_pos % Disk->d_Sectors;
		}
    }
    if ( Count > 128 ) {
		Count = 128;
    }
    
retry:
    memset( &reg86, 0, sizeof( reg86 ) );

    reg86.x.ax	= 0x4200;
    reg86.x.dx	= Disk->d_DriveNum;
	
    (*(u8_t*)(g_cmdbuf  + 0)) = 0x10;	/* Size */
    (*(u8_t*)(g_cmdbuf  + 1)) = 0x00;	/* reserved */
    (*(u16_t*)(g_cmdbuf + 2)) = Count;
    (*(u32_t*)(g_cmdbuf + 4)) = (seg << 16) | off;
    (*(u64_t*)(g_cmdbuf + 8)) = sector_pos;

	
    reg86.x.ds  = ((unsigned int) (g_cmdbuf) ) >> 4;
    reg86.x.si = ((unsigned int) (g_cmdbuf) ) & 0x0f;

    do_int86( 0x13, &reg86,&reg86 );

    //schedule();
//  printk( "Status = %lx (%lx)\n", ((reg86.x.ax >> 8) & 0xff), g_databuf );

    if ( ( reg86.x.flags & 0x01 ) ) {
    	if ( nRetryCount++ < 3 ) {
	    if ( nRetryCount == 2 ) {
		Count = 1;
		printk( "Error: bd_lba_read_sectors() failed to read %d sectors (%lx), try with 1\n", Count, ((reg86.x.ax >> 8) & 0xff) );
	    }
	    goto retry;
	}
    } else {
	if ( nRetryCount > 0 ) {
	    Disk->d_bTruncateToCyl = true;
	    printk( "*** LBA BIOS SEEMS TO HAVE PROBLEMS WITH IO CROSSING CYLINDER BOUNDARIES ***" );
	    printk( "*** WILL TRUNCATE FURTHER REQUESTS TO FIT WITHIN CYLINDER BOUNDARIES     ***" );
	}
    }
    return( ( reg86.x.flags & 0x01 ) ? -EIO : Count );
}


int bd_lba_write_sectors( biosdisk_drv_t* Disk, const void* pbuf, 
	off_t sector_pos, int Count )
{
    vm86regs_t reg86;
    int	 seg = ((unsigned int) (pbuf) ) >> 4;
    int	 off = ((unsigned int) (pbuf) ) & 0x0f;
    int  nRetryCount = 0;
    
    if ( Disk->d_bTruncateToCyl && Disk->d_Sectors > 1 ) {
	if ( sector_pos / Disk->d_Sectors != (sector_pos+Count-1) / Disk->d_Sectors ) {
	    Count = Disk->d_Sectors - sector_pos % Disk->d_Sectors;
	}
    }
    if ( Count > 128 ) {
	Count = 128;
    }
retry:
    memset( &reg86, 0, sizeof( reg86 ) );

    reg86.x.ax	= 0x4301; // Write sector without verify
    reg86.x.dx	= Disk->d_DriveNum;
	
    (*(u8_t*)(g_cmdbuf  + 0)) = 0x10;	/* Size */
    (*(u8_t*)(g_cmdbuf  + 1)) = 0x00;	/* reserved */
    (*(u16_t*)(g_cmdbuf + 2)) = Count;
    (*(u32_t*)(g_cmdbuf + 4)) = (seg << 16) | off;
    (*(u64_t*)(g_cmdbuf + 8)) = sector_pos;

    reg86.x.ds  = ((unsigned int) (g_cmdbuf) ) >> 4;
    reg86.x.si = ((unsigned int) (g_cmdbuf) ) & 0x0f;

    do_int86( 0x13, &reg86,&reg86 );
    //schedule();
    if ( reg86.x.flags & 0x01 ) {
	if ( nRetryCount++ < 3 ) {
	    if ( nRetryCount == 2 ) {
		Count = 1;
		printk( "Error: bd_lba_write_sectors() failed to write %d sectors (%lx), try with 1\n", Count, ((reg86.x.ax >> 8) & 0xff) );
	    }
	    goto retry;
	}
	printk( "Error: bd_lba_write_sectors() failed to write %d sectors at pos %Ld (Status = %lx)\n", Count, sector_pos, ((reg86.x.ax >> 8) & 0xff) );
	return( -EIO );
    } else {
	if ( nRetryCount > 0 ) {
	    Disk->d_bTruncateToCyl = true;
	    printk( "*** LBA BIOS SEEMS TO HAVE PROBLEMS WITH IO CROSSING CYLINDER BOUNDARIES ***" );
	    printk( "*** WILL TRUNCATE FURTHER REQUESTS TO FIT WITHIN CYLINDER BOUNDARIES     ***" );
	}
	return( Count );
    }
}

int bd_csh_read_sectors( biosdisk_drv_t* Disk, const void* pbuf,
S64_t sector_lba_pos, int Count )
{
    vm86regs_t reg86;
    S64_t sector_pos;
    S64_t nCyl;
    S64_t nHead;
    S64_t nTmp;
    int   nRetryCount = 0;

    nCyl    = (int)sector_lba_pos / ((S64_t)Disk->d_Sectors * Disk->d_Heads);
    nTmp    = (int)sector_lba_pos % ((S64_t)Disk->d_Sectors * Disk->d_Heads);
    nHead   = nTmp / (S64_t)Disk->d_Sectors;
    sector_pos = (nTmp % (S64_t)Disk->d_Sectors) + 1;

    if ( Count > ((S64_t)Disk->d_Sectors) - sector_pos + 1LL  ) {
	Count = ((S64_t)Disk->d_Sectors) - sector_pos + 1LL;
    }

    if ( ((int)sector_lba_pos + Count - 1) / (Disk->d_Sectors * Disk->d_Heads) != nCyl ) {
	printk( "bd_csh_read_sectors() Failed to reduse sectors at %Ld:%Ld:%Ld (%d)\n",
		nCyl, sector_pos, nHead, Count );
	Count = 1;
    }

    assert( Count > 0 );
    assert( sector_pos > 0 && sector_pos < 64 );
retry:  
    memset( &reg86, 0, sizeof( reg86 ) );

    if ( Count > 255 ) {
	printk( "bd_csh_read_sectors() attempt to read more than 255 sectors (%d)\n", Count );
	return( -EINVAL );
    }
  
    reg86.x.ax = 0x0200 | Count;
    reg86.x.cx = ((nCyl & 0xff) << 8) | (sector_pos & 0x3f) | ((nCyl & 0x300) >> 2);
    reg86.x.dx = (nHead << 8) | Disk->d_DriveNum;
  
    reg86.x.es  = ((u32_t)pbuf) >> 4;
    reg86.x.bx = ((u32_t)pbuf) & 0x0f;

    if ( nCyl < 0 || nCyl >= 1024 ) {
	printk( "bd_csh_read_sectors() Invalid cylinder number %Ld\n", nCyl );
	printk( "LBA = %Ld\n", sector_lba_pos );
    }
  
    if ( nCyl    != (((reg86.x.cx >> 8) & 0xff) | ((reg86.x.cx & 0xc0) << 2)) ) {
	printk( "bd_csh_read_sectors() Failed to split cylinder # %Ld, got %ld\n",
		nCyl, (((reg86.x.cx >> 8) & 0xff) | ((reg86.x.cx & 0xc0) << 2)) );
    }
  
    assert( nHead   == ((reg86.x.dx >> 8) & 0xff) );
    assert( sector_pos == (reg86.x.cx & 0x3f) );

    if ( Disk->d_DriveNum < 2 ) {
	restart_timer( &g_floppy_timer,  0LL);
    }
    
    do_int86( 0x13, &reg86,&reg86 );
	
    if ( Disk->d_DriveNum < 2 ) {
	restart_timer( &g_floppy_timer, 4000L );
    }
    if ( reg86.x.flags & 0x01 ) {
    	if ( nRetryCount++ < 3 ) {
	    if ( Disk->d_DriveNum < 2 ) {
		memset( &reg86, 0, sizeof( reg86 ) );
		reg86.x.ax = 0x0000; // Reset the floppy drive
		reg86.x.dx = Disk->d_DriveNum;
		do_int86( 0x13, &reg86,&reg86 );
	    }
	    if ( nRetryCount == 2 ) {
		Count = 1;
		printk( "Error: bd_csh_read_sectors() failed to read %d sectors (%lx), try with 1\n", Count, ((reg86.x.ax >> 8) & 0xff) );
	    }
	    goto retry;
	}
	
	printk( "Read sector %Ld (%Ld:%Ld:%Ld) (%d:%d)\n",
		sector_lba_pos, nCyl, nHead, sector_pos, Disk->d_Heads, Disk->d_Sectors );
	printk( "Status = %lx (%p)\n", ((reg86.x.ax >> 8) & 0xff), pbuf );
    }
    return( ( reg86.x.flags & 0x01 ) ? -EIO : Count );
}

int bd_csh_write_sectors( biosdisk_drv_t* Disk, const void* pbuf, 
	S64_t sector_lba_pos, int Count )
{
    vm86regs_t reg86;
    S64_t	 sector_pos;
    S64_t    nCyl;
    S64_t	 nHead;
    S64_t	 nTmp;
    int          nRetryCount = 0;

    nCyl    = (int)sector_lba_pos / ((S64_t)Disk->d_Sectors * Disk->d_Heads);
    nTmp    = (int)sector_lba_pos % ((S64_t)Disk->d_Sectors * Disk->d_Heads);
    nHead   = nTmp / (S64_t)Disk->d_Sectors;
    sector_pos = (nTmp % (S64_t)Disk->d_Sectors) + 1;

    if ( Count > ((S64_t)Disk->d_Sectors) - sector_pos + 1LL ) {
	Count = ((S64_t)Disk->d_Sectors) - sector_pos + 1LL;
    }

    if ( ((int)sector_lba_pos + Count - 1) / (Disk->d_Sectors * Disk->d_Heads) != nCyl ) {
	printk( "bd_csh_write_sectors() Failed to reduse sectors at %Ld:%Ld:%Ld (%d)\n",
		nCyl, sector_pos, nHead, Count );
	Count = 1;
    }
  
    assert( Count > 0 );
retry:    
    memset( &reg86, 0, sizeof( reg86 ) );

    if ( Count > 255 ) {
	printk( "bd_csh_write_sectors() attempt to write more than 255 sectors (%d)\n", Count );
	return( -EINVAL );
    }
  
    reg86.x.ax = 0x0300 | Count;
    reg86.x.cx = ((nCyl & 0xff) << 8) | (sector_pos & 0x3f) | ((nCyl & 0x300) >> 2);
    reg86.x.dx = (nHead << 8) | Disk->d_DriveNum;

    if ( nCyl < 0 || nCyl >= 1024 ) {
	printk( "bd_csh_write_sectors() Invalid cylinder number %Ld\n", nCyl );
//      return( -EINVAL );
    }
  
    if ( nCyl != (((reg86.x.cx >> 8) & 0xff) | ((reg86.x.cx & 0xc0) << 2)) ) {
	printk( "bd_csh_write_sectors() Failed to split cylinder # %Ld, got %ld\n", nCyl, (((reg86.x.cx >> 8) & 0xff) | ((reg86.x.cx & 0xc0) << 2)) );
    }
  
    reg86.x.es  = ((u32_t)pbuf) >> 4;
    reg86.x.bx = ((u32_t)pbuf) & 0x0f;

    if ( Disk->d_DriveNum < 2 ) {
	restart_timer( &g_floppy_timer, 0L  );
    }
    
    do_int86( 0x13, &reg86,&reg86 );
    
    if ( Disk->d_DriveNum < 2 ) {
	restart_timer( &g_floppy_timer, 2000L );
    }
    
    if ( reg86.x.flags & 0x01 ) {
    	if ( nRetryCount++ < 3 ) {
	    if ( Disk->d_DriveNum < 2 ) {
		memset( &reg86, 0, sizeof( reg86 ) );
		reg86.x.ax = 0x0000; // Reset the floppy drive
		reg86.x.dx = Disk->d_DriveNum;
		do_int86( 0x13, &reg86,&reg86 );
	    }
	    if ( nRetryCount == 2 ) {
		Count = 1;
		printk( "Error: bd_csh_write_sectors() failed to write %d sectors (%lx), try with 1\n", Count, ((reg86.x.ax >> 8) & 0xff) );
	    }
	    goto retry;
	}
	printk( "Write sector %Ld (%Ld:%Ld:%Ld) (%d:%d)\n",
		sector_lba_pos, nCyl, nHead, sector_pos, Disk->d_Heads, Disk->d_Sectors );
	printk( "Status = %lx (%p)\n", ((reg86.x.ax >> 8) & 0xff), pbuf );
    }
  
    return( ( reg86.x.flags & 0x01 ) ? -EIO : Count );
}

#endif





