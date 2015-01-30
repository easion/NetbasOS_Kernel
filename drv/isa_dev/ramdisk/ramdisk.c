
/* 
** Jicama OS Loadable Kernel Module
** 2005-3-5  ramdisk driver
*/

//#include <sdk.h>
#include <drv/drv.h>
//#include <string.h>

int mk_fatfs(void);
unsigned int ramdisk_addr;
unsigned long ramdisk_end;
int ramdisk_size;

 


int ramdisk_read(unsigned minor, off_t  offset, char * buf,int count)
{
	u32_t pos;


	pos=ramdisk_addr+BLOCK_SIZE*offset;
	if(pos>=(ramdisk_end))return -1;

	kprintf("ramdisk_read called at %x!\n", pos);
	memcpy(buf, (void *)pos, count);
   return OK;
}

int ramdisk_write(unsigned minor, off_t  offset, char * buf,int count)
{
	u32_t pos;

	pos=ramdisk_addr+BLOCK_SIZE*offset;
	if(pos>=(ramdisk_end))return -1;

	memcpy(buf, (void *)pos, count);
   return OK;
   }

static void mark_fat12 (long cluster, u32_t value, u8_t *fat)
{
		value &= 0x0fff;
		if (((cluster * 3) & 0x1) == 0)
		{
			fat [(3 * cluster / 2)    ] = (u8_t) (value & 0x00ff);
			fat [(3 * cluster / 2) + 1] = (u8_t) ((fat [(3 * cluster / 2) + 1] & 0x00f0) | ((value & 0x0f00) >> 8));
		}
		else
		{
			fat [(3 * cluster / 2)    ] = (u8_t) ((fat [3 * cluster / 2] & 0x000f) | ((value & 0x000f) << 4));
			fat [(3 * cluster / 2) + 1] = (u8_t) ((value & 0x0ff0) >> 4);
		}
}

int mk_fatfs(void)
{
	int i;
	buffer_t *bp;
	struct msdos_super *sb;
    char *fatbuf;

	bp=bread(DEV_RAM, 0);
	sb=(struct msdos_super*)bp->b_data;
	memset(bp->b_data, 0, dev_block_size(DEV_RAM));

   sb->jmp_boot[0] = 0xEB; 
   sb->jmp_boot[1] = 0x00;
   sb->jmp_boot[2] = 0x90 ;

    memcpy(sb->oem,"MSWIN4.1",8);
    sb->sector_size =HD_SECTOR_SIZE;
    sb->cluster_size = 1;
    sb->reserved=1;
    sb->fats=2;
    sb->dir_entries=224;
    sb->sectors=0xBF0;
    sb->media_descriptor=0xF0;
    sb->fat16_length   = 0x9;
    sb->sec_per_track = 0x12;
    sb->heads= 2;
    bp->b_data[510]=0x55;
    bp->b_data[511]=0xaa;

  	kprintf("make fat..\n");
  

	return 0;
}


/*dll entry*/
int dll_main(char **argv)
{
	ramdisk_size=1440*1024;

	//if(argv)	ramdisk_size=hex2num(argv[1]);

	ramdisk_addr=mm_malloc(ramdisk_size);

	if(!ramdisk_addr){
		return -1;
	}

	ramdisk_end=ramdisk_addr+ramdisk_size;

	 mk_fatfs();
	 register_fs(DEV_RAM, "FAT12", 0);
	return 0;
}


int dll_destroy()
{
	kprintf("Destroy Ramdisk ...\n");
	if(ramdisk_addr)
		free(ramdisk_addr, ramdisk_size);
	else
		kprintf("Done!\n");
	return 0;
}

int dll_version()
{
	kprintf("Ramdisk VERSION 0.01!\n");
	return 0;
}


