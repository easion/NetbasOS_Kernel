
#include <ansi.h>
#include <jicama/system.h>
#include <jicama/grub.h>
#include <jicama/global.h>
#include <jicama/grub.h>
#include <string.h>

 __asmlink void low_mem_init(grub_info_t* info);

__local int ramsize;

unsigned long KERN_PG_DIR_ADDR = 0xc00000L;
unsigned long grub_modules_end_address(grub_info_t* info);

static char kernel_command[1024];

__public int init_kernel_command(void)
{
    if (!CHECK_GRUB(sys_boot_info.flags, MULTIBOOT_COMMAND_LINE))
		return -1;

	strncpy(kernel_command,(char *)sys_boot_info.cmdline,1024);
	return 0;
}

__public char* get_kernel_command(void)
{
		return kernel_command;
}

int get_str_arg( char* value, const char* label, const char* arg, int arglen )
{
    int len = strlen( label );

    if ( len >= arglen ) {
	return( FALSE );
    }
    
    if ( strncmp( arg, label, len ) == 0 ) {
	memcpy( value, arg + len, arglen - len );
	value[arglen - len] = '\0';
	return( TRUE );
    }
    return( FALSE );
}

int grub_apm(void)
{
    trace( "APM table entry: " );

    if ( CHECK_GRUB( sys_boot_info.flags, 10 ) ){
	  trace( "found!\n" );
	  return 0;
	}
     else
	  trace( "not found!\n" );
	  return -1;
}


void cp_grub_info(grub_info_t* boot_jicama)
{
	unsigned long end=0;
	const int kern_stack_size = 0x0000ffff; //min 64 k for kernel used

	end = grub_modules_end_address(boot_jicama);

	if (end<0x400000)
		end = 0x400000;
	else
		end += kern_stack_size;
	//else if ((end+kern_stack_size)&0x000fffff < kern_stack_size)
	//	end += kern_stack_size;

	KERN_PG_DIR_ADDR = (end+0x0000ffff) & 0xffff0000; //64k内核堆栈
	memcpy(&sys_boot_info, boot_jicama, sizeof(grub_info_t));
}

int grub_get_boot( dev_t* dev_no)
{
	int i, dev = -1; 

   /*if microkernel here will be zero*/
   if (!CHECK_GRUB(sys_boot_info.flags,MULTIBOOT_BOOT_DEVICE)){
		for(i = 0; i<4; i++)
		trace("Boot Device = %x \n", sys_boot_info.boot_device[i]);
		return -1;
   }

/*if boot from floppy*/
    if (sys_boot_info.boot_device[3]<0x80)
	{
	  dev = sys_boot_info.boot_device[3];  //if fp
	  trace("Boot Kernel From FD%d\n",dev);
	  *dev_no = dev_nr(FD,0);
	  return dev_nr(FD,0);
	}
    else {/*if boot from harddisk*/
		dev = sys_boot_info.boot_device[3]-0x80;
		// *dev_no = sys_boot_info.boot_device[2]+1;
          *dev_no = dev_nr(HD,dev*4+sys_boot_info.boot_device[2]+1);
	     trace("Boot Kernel From hd%c (Linux Format)dev_no %d\n",'a'+dev, sys_boot_info.boot_device[2]+1);
	     return dev;
	}

	return dev;
}


int grub_mem_useable(unsigned long* lower, unsigned long* upper)
{
   if (!CHECK_GRUB(sys_boot_info.flags, MULTIBOOT_MEMORY))
	   return -1;
		//kprintf("mem_lower=%dKB, mem_upper=%dKB\n",
	      //   (int)sys_boot_info.mem_lower,(int)sys_boot_info.mem_upper);

   *upper = (unsigned long)((sys_boot_info.mem_upper + 1024) << 10); 
   *lower = sys_boot_info.mem_lower;
   return SUCCESS;
}


char *get_boot_loader(char *loader)
{
    if (!CHECK_GRUB(sys_boot_info.flags,MULTIBOOT_OS_NAME))
		return (char *)0;
	else if(!loader)
		return (char *)0;
	else{
		strcpy(loader, (char*)sys_boot_info.boot_loader_name);
		trace("Boot Manager: %s\n",(char*)sys_boot_info.boot_loader_name);
	}

	return (char*)sys_boot_info.boot_loader_name;
}

/*获取GRUB的版本信息*/
int grub_version(void)
{
    if (CHECK_GRUB(sys_boot_info.flags,9))
		trace("Boot Manager: %s\n",(char*)sys_boot_info.boot_loader_name);

	return SUCCESS;
}



int grub_mmap(grub_info_t* info)
{
      memory_map_t *mmap;

   if (!CHECK_GRUB (info->flags, 6))
      return -1;
      
      trace ("\nmap_addr = 0x%X, mmap_length = 0x%X\n",
	      (unsigned) info->mmap_addr, (unsigned) info->mmap_length);

      for (mmap = (memory_map_t *) info->mmap_addr;
	   (unsigned long) mmap < info->mmap_addr + info->mmap_length;
	   mmap = (memory_map_t *) ((unsigned long) mmap
				    + mmap->size + sizeof (mmap->size))){

	trace ("Size = 0x%X, BASE = 0x%X-%XX,"
		"Length = 0x%x%X, Type = %u\n",
		(unsigned) mmap->size,
		(unsigned) mmap->base_addr_high,
		(unsigned) mmap->base_addr_low,
		(unsigned) mmap->length_high,
		(unsigned) mmap->length_low,
		(unsigned) mmap->type);
	   }
	   return 0;
}
