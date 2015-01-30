
#ifndef _pe_h_
#define _pe_h_



#define IMAGE_PE_SIGNATURE 0x00004550  // PE00
#define PE32MAGIC  0x010B
#define DOS_MZBLK_SZ 512L

struct dos_header {
  u8_t e_mz_magic[2];		/* Magic number 			*/
  u16_t e_cblp;		/* Bytes on last page of file		*/
  u16_t e_cp;		/* Pages in file (size of the file in blocks)*/

  u16_t e_crlc;		/* Number of Relocations		*/
  u16_t e_cparhdr;	/* Size of header in paragraphs		*/
  u16_t e_minalloc;	/* Minimum extra paragraphs needed	*/
  u16_t e_maxalloc;	/* Maximum extra paragraphs needed	*/

  u16_t e_ss;		/* Initial (relative) SS value		*/
  u16_t e_sp;		/* Initial SP value			*/
  u16_t e_csum;		/* Checksum				*/
  u16_t e_ip;		/* Initial IP value			*/
  u16_t e_cs;		/* Initial (relative) CS value		*/
  u16_t e_lfarlc;	/* File address of relocation table	*/
  u16_t e_ovno;		/* Overlay number			*/

  /* DJGPP Ends HERE!!! */

  u16_t e_res[4];	/* Reserved words 			*/
  u16_t e_oemid;	        /* OEM identifier (for e_oeminfo) 	*/
  u16_t e_oeminfo;	/* OEM information; e_oemid specific	*/
  u16_t e_res2[10];	/* Reserved words			*/
  long int e_lfanew;	/* File address of new exe header	*/
};

//
// PE image file header
//

struct image_directory
{
  u32_t virtual_address;
  u32_t size;
};


#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

typedef struct
{
	char magic[2]		__attr_packet;
/* not really unused, but we don't care about them for PE */
	char unused[6]		__attr_packet;
	u16_t hdr_size	__attr_packet ;
	char unused2[50]	 __attr_packet ;
	u32_t new_exe_offset __attr_packet ;
} pehdr_t; /* 64 bytes */

typedef struct   
{
  u16_t  magic;      /* type of file        */
  u16_t  vstamp;      /* linker version stamp    */
  u32_t code_size;    /* text size        */
  u32_t data_size;    /* initialized data      */
  u32_t bss_size;    /* uninitialized data    */
  u32_t entry;      /* entry pt.        */

  u32_t t_base;    /* base of text used for this file */
  u32_t d_base;    /* base of data used for this file */

  u32_t image_base;   /* image linear memory base */
  
  u32_t sect_align;
  u32_t file_align;
  u32_t version[4];
  u32_t image_size;
  u32_t hdr_size;
  u32_t checksum;
  u16_t  subsystem;
  u16_t  dllflags;
  
  u32_t stackr_size;  /* stack reserved size  */
  u32_t stackc_size;  /* stack commit size  */
  u32_t heapr_size;    /* heap reserved size  */
  u32_t heapc_size;    /* heap commit size    */
  u32_t loader_flags;
  u32_t rva_num;
  
  struct image_directory data_directory[16]; /* tables and description */
}pe_aouthdr_t;


#define MZ_DIR_EXPORT          0
#define MZ_DIR_IMPORT          1
#define MZ_DIR_RESOURCE        2
#define MZ_DIR_EXCEPTION       3
#define MZ_DIR_SECURITY        4
#define MZ_DIR_BASERELOC       5
#define MZ_DIR_DEBUG           6
#define MZ_DIR_COPYRIGHT       7
#define MZ_DIR_GLOBALPTR       8
#define MZ_DIR_TLS             9
#define MZ_DIR_LOAD_CONFIG     10
#define MZ_DIR_BOUND_IMPORT    11
#define MZ_DIR_IAT             12
#define MZ_DIR_DELAY_IMPORT    13 // Delay Load Import Descriptors
#define MZ_DIR_COM_DESCRIPTOR  14 // COM Runtime descriptor



#endif 
