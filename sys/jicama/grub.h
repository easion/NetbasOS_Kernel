#ifndef GRUB_H
#define GRUB_H


#include <type.h>

/* The module structure. */
typedef struct grub_module
{
  size_t mod_start;
  size_t mod_end;
  size_t string;
  size_t reserved;
} module_t;

/* The memory map. Be careful that the offset 0 is base_addr_low
 *    but no size. */
typedef struct grub_memory_map
{
  size_t size;
  size_t base_addr_low;
  size_t base_addr_high;
  size_t length_low;
  size_t length_high;
  size_t type;
} memory_map_t;

#define MULTIBOOT_MEMORY	0
#define MULTIBOOT_BOOT_DEVICE	1
#define MULTIBOOT_COMMAND_LINE	    2
#define MULTIBOOT_MEM_MAP 	6
#define MULTIBOOT_OS_NAME 	9

#define MULTIBOOT_HEADER_FLAGS		0x00010003



/* The magic number for the Multiboot header. */
#define MULTIBOOT_HEADER_MAGIC          0x1BADB002

/* The magic number passed by a Multiboot-compliant boot loader. */
#define MULTIBOOT_BOOTLOADER_MAGIC      0x2BADB002

typedef struct 
{
  unsigned long flags;
  /* Available memory from BIOS */
  unsigned long mem_lower;
  unsigned long mem_upper;
 
  /* "root" partition */
  unsigned char boot_device[4];
  
   /* Kernel command line */
 unsigned long cmdline;
  /* Boot-Module list */
  unsigned long mods_count; //MODULE
  unsigned long mods_addr;

      /*  Kernel symbol table info */
  unsigned long HeaderTable[4];

  /* Memory Mapping buffer */
  unsigned long mmap_length;
  unsigned long mmap_addr;

  /* Drive Info buffer */
  unsigned long drives_count;
  unsigned long drives_addr;

  /* ROM configuration table */
  unsigned long config_table;
  unsigned long boot_loader_name;
  
	  /*other information*/
  unsigned long apm_table;

  /* Video */
  unsigned long vbe_control_info;
  unsigned long vbe_mode_info	;
  unsigned short vbe_mode	;
  unsigned short vbe_interface_seg;
  unsigned short vbe_interface_off;
  unsigned short vbe_interface_len;

  /*
     Gerardo: I need to add also the phisical address base for
     both low ( < 1MB) & upper ( > 1MB) memory, as X starts from DOS
     which could have preallocated some of this memory...
     For example, GRUB assumes that mem_lowbase = 0x0 &
     mem_upbase = 0x100000
  */
  unsigned long mem_lowbase;
  unsigned long mem_upbase;

  }grub_info_t;

#define CHECK_GRUB(flags,bit) ((flags)&(1<<(bit)))


extern int grub_get_boot( dev_t* partition);
extern int grub_mem_useable(unsigned long* lower, unsigned long* upper);
extern int grub_version(void);
extern void cp_grub_info(grub_info_t* b);
extern int grub_load_modules(grub_info_t* info);
int grub_mmap(grub_info_t* info);
int grub_read_lfb(grub_info_t* info);

/* The Multiboot header. */
typedef struct multiboot_header
{
   uint32_t magic			: 32;
   uint32_t flags			: 32;
   uint32_t checksum		: 32;
   uint32_t header_addr		: 32;
   uint32_t load_addr		: 32;
   uint32_t load_end_addr		: 32;
   uint32_t bss_end_addr		: 32;
   uint32_t entry_addr		: 32;
   uint32_t mode_type		: 32;
   uint32_t width			: 32;
   uint32_t height		: 32;
   uint32_t depth			: 32;
} multiboot_header_t;

extern module_t MOD_logo;
extern module_t MOD_shell;

//#include <jicama/process.h>
#define MAX_DLL_FILE 8

extern module_t MOD_dll[MAX_DLL_FILE];
extern module_t MOD_server[16];
int grub_apm(void);
__asmlink grub_info_t sys_boot_info;


#endif

