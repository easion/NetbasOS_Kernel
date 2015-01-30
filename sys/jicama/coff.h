
#ifndef _COFF_H_
#define _COFF_H_

enum{
	 F_RELFLG	=(0x0001),
	 F_EXEC		=(0x0002),
	 F_LNNO		=(0x0004),
	 F_LSYMS	=(0x0008)
};

#define	I386MAGIC	0x14c
#define I386AIXMAGIC	0x175
#define I386BADMAG(x) (((x)->f_magic!=I386MAGIC) && (x)->f_magic!=I386AIXMAGIC)

enum{
	 RELOC_REL32=	20,	/* 32-bit PC-relative address */
	 RELOC_ADDR32=	6,	/* 32-bit absolute address */
};

typedef struct coff_hdr {
	unsigned short f_magic;		/* magic number: 0x14c			*/
	unsigned short f_num_sections;		/* number of sections		*/
	unsigned long f_time;	/* time & date stamp		*/
	unsigned long f_sym_ptr;	/* file offset of symbol table */
	unsigned long f_num_syms;		/* number of symbol table entries	*/
	unsigned short f_opthdr;	/* sizeof(coff_hdr)		*/
	unsigned short f_flag;		/* flags			*/
}coff_file_hdr_t;

typedef struct coff_aout
{
  unsigned short 	magic;		/* type of file	: 0x10b			*/
  unsigned short	version;		/* version stamp			*/
  unsigned long	tsize;		/* text size in bytes, padded to FW bdry*/
  unsigned long	dsize;		/* initialized data "  "		*/
  unsigned long	bsize;		/* uninitialized data "   "		*/
  unsigned long	entry;		/* entry point.				*/
  unsigned long 	t_base;	/* base of text used for this file */
  unsigned long 	d_base;	/* base of data used for this file */
}coff_file_aout_t;

/*
 * s_flags "type"
 */
 enum{
	STYP_TEXT	= (0x0020),	/* section contains text only */
	STYP_DATA	 =(0x0040),	/* section contains data only */
	 STYP_BSS	= (0x0080)	/* section contains bss only */
 };

typedef struct coff_section {
	char		s_name[8];	/* section name			*/
	unsigned long		s_paddr;	/* physical address, aliased s_nlib */
	unsigned long		s_vis_run_addr;	/* virtual address		*/
	unsigned long		s_raw_size;		/* section size			*/
	unsigned long		s_raw_data_addr;	/* file ptr to raw data for section */
	unsigned long		s_relptr;	/* file ptr to relocation	*/
	unsigned long		s_lnnoptr;	/* file ptr to line numbers	*/
	unsigned short		s_nreloc;	/* number of relocation entries	*/
	unsigned short		s_nlnno;	/* number of line number entries*/
	unsigned long		s_flags;	/* flags			*/
}coff_section_t;

typedef struct
{
	u32_t adr		__attr_packet;
	u32_t symtab_index	__attr_packet;
	u16_t type		__attr_packet;
} coff_reloc_t;

typedef struct
{
	union
	{
		char name[8]	__attr_packet;
		struct
		{
			u32_t zero		__attr_packet;
			u32_t strtab_index	__attr_packet;
		} x		__attr_packet;
	} x			__attr_packet;
	u32_t val		__attr_packet;
	u16_t sect_num	__attr_packet;
	u16_t type		__attr_packet;
	u8_t sym_class	__attr_packet;
	u8_t num_aux		__attr_packet;
} coff_sym_t;

/*
NULL 0 无存储类型。 
AUTOMATIC 1 自动类型。通常是在栈中分配的变量。 
EXTERNAL 2 外部符号。当为外部符号时，iSection的值应该为0，如果不为0，则ulValue为符号在段中的偏移。 
STATIC 3 静态类型。ulValue为符号在段中的偏移。如果偏移为0，那么这个符号代表段名。 
REGISTER 4 寄存器变量。 
MEMBER_OF_STRUCT 8 结构成员。ulValue值为该符号在结构中的顺序。 
STRUCT_TAG 10 结构标识符。 
MEMBER_OF_UNION 11 联合成员。ulValue值为该符号在联合中的顺序。 
UNION_TAG 12 联合标识符。 
TYPE_DEFINITION 13 类型定义。 
FUNCTION 101 函数名。 
FILE 102 文件名。 
*/
enum
{
	NULL_CLASS = 0, AUTOMATIC_CLASS, EXTERNAL_CLASS, STATIC_CLASS, 
	REGISTER_CLASS, MEMBER_OF_STRUCT_CLASS, STRUCT_TAG_CLASS, 
	MEMBER_OF_UNION_CLASS, UNION_TAG_CLASS, TYPE_DEFINITION_CLASS, 
	FUNCTION_CLASS, FILE_CLASS
} ;
	
/********************** LINE NUMBERS **********************/

/* 1 line number entry for every "breakpointable" source line in a section.
 * Line numbers are grouped on a per function basis; first entry in a function
 * grouping will have l_lnno = 0 and in place of physical address will be the
 * symbol table index of the function name.
 */
struct external_lineno {
	union {
		unsigned long l_symndx __attr_packet;	/* function name symbol index, iff l_lnno == 0 */
		unsigned long l_paddr __attr_packet;		/* (physical) address of line number */
	} l_addr;
	unsigned short l_lnno;						/* line number */
};



/* max. number of sections in coff_section structure */
#define	MAX_SECTIONS	5 /* code, data, BSS, heap, stack */
#define	MAX_CPPSECTIONS	900 /* code, data, BSS, heap, stack */


enum{
 TSCN    =0,
 DSCN   =1,
 BSCN    =2,
};


struct coff_peaout
{
	u16_t 	magic;		/* type of file	: 0x10b			*/
	u16_t	version;		/* version stamp			*/
	u32_t	tsize;		/* text size in bytes, padded to FW bdry*/
	u32_t	dsize;		/* initialized data "  "		*/
	u32_t	bsize;		/* uninitialized data "   "		*/
	u32_t	entry;		/* entry point.				*/
	u32_t 	t_base;	/* base of text used for this file */
	u32_t 	d_base;	/* base of data used for this file */
	u32_t image_base	;
	char unused[192]	;
};

#define _TEXT_SECT	".text"
#define _DATA_SECT	".data"
#define _BSS_SECT	".bss"
#define _COMMENT_SECT ".comment"
#define _LIB_SECT ".lib"

int coff_read_head(const struct coff_hdr* hdr);
int coff_check(const struct coff_hdr* );

#endif /* _COFF_H_ */
