
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama OS  
// Copyright (C) 2002-2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------

#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>

#if 1
struct t_bios32
{
    unsigned long magic;   ///////魔数
    unsigned long phys_bsd_entry; //////物理入口
    unsigned char vers;
    unsigned char prg_lens;
    unsigned char crc;
} ;

unsigned long bios32_entry=0;

int bios_init( void )
{
    unsigned char *bios32_base = ( unsigned char * ) 0xE0000;
    struct t_bios32 *bios32, *master;
    unsigned char bios32_flag = 1, bios32_crc;
    int bios32_index;
    
    // Check present of BIOS32
    master = NULL;

    
    while ( bios32_flag == 1 && ( unsigned long ) bios32_base < 0x100000 )
    {
	bios32 = (struct t_bios32 * ) bios32_base;

	if ( bios32->magic == 0x5F32335F )    ////////test the magic
	{
		//magic like "_32_" 
	    for ( bios32_index = 0, bios32_crc = 0; bios32_index < ( bios32->prg_lens * 16 ) ; bios32_index++ )
		bios32_crc += * ( bios32_base + bios32_index );

	    if ( bios32_crc == 0 )
	    {
		bios32_flag = 0;
		master = bios32;
	    }

	}

	  else   ////////magic error
	    bios32_base += 0x10;

    }     ///////////out loop.
    
    if ( bios32_flag == 0 )
    {
	#ifdef  DEBUG
	printk( "bios: service directory found, entry=0x%x\n", bios32->phys_bsd_entry );
	bios32_entry = bios32->phys_bsd_entry;
     #endif

	 if ( master->phys_bsd_entry > 0x100000 )
	    printk( "bios: bsd entry is in high memory!!\n" );

    }

    return bios32_flag;
}
#endif

