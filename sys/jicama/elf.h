#ifndef _ELF_H
#define _ELF_H


typedef u32_t elfaddr_t;
typedef unsigned short elfhalf_t;
typedef unsigned long elfoff_t;
typedef long elfsword_t;
typedef u32_t elfword_t;

/* 32-bit ELF base types. */
typedef u32_t	Elf32_Addr;
typedef u16_t	Elf32_Half;
typedef u32_t	Elf32_Off;
typedef s32_t	Elf32_Sword;
typedef u32_t	Elf32_Word;

/* 64-bit ELF base types. */
typedef u64_t	Elf64_Addr;
typedef u16_t	Elf64_Half;
typedef s16_t	Elf64_SHalf;
typedef u64_t	Elf64_Off;
typedef s32_t	Elf64_Sword;
typedef u32_t	Elf64_Word;
typedef u64_t	Elf64_Xword;
typedef s64_t	Elf64_Sxword;

#define PF_R	0x4
#define PF_W	0x2
#define PF_X	0x1


/*describes the header file of one ELF file*/
#define EINIDENT	16

enum{
 ELFDATANONE=	0	,// invalid data encoding
 ELFDATA2LSB	=1,	// (for intel32)
 ELFDATA2MSB	=2
};

/*
 * definitions used in "e_version"
 */
 enum{
	 EV_NONE		=0,	// invalid version
	EV_CURRENT	=1 	// current version of the opject file
};

// ELF class or capacity						//
enum{
 ELF_CLASSNONE	=0,	//!< Invalid class.
 ELF_CLASS32	=1,	//!< 32-bit objects.
 ELF_CLASS64	=2	//!< 64-bit objects.
};
#define	ELFMAG0		0x7f	/* EIMAG */
#define	ELFMAG1		'E'
#define	ELFMAG2		'L'
#define	ELFMAG3		'F'
#define	ELFMAG		"\177ELF"
#define	SELFMAG		4

#define EI_MAG0		0
#define EI_MAG1		1
#define EI_MAG2		2
#define EI_MAG3		3
#define EI_CLASS	4
#define EI_DATA		5
#define EI_VERSION	6
#define EI_PAD		7

enum {
  SHF_WRITE  = 0x01,
  SHF_ALLOC  = 0x02,
  SHF_EXECINST = 0x04,
  SHF_MASCPROC = 0xF0000000
};

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_LOOS   0x60000000
#define SHT_HIOS   0x6fffffff
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff

/*
 * definitions used in "file_type"
 */

enum {
	ET_NONE	=	0, ET_REL	=	1, ET_EXEC	=	2, ET_DYN	=	3,
	ET_CORE	=	4, ET_LOPROC	= 0xFF00, ET_HIPROC	= 0XFFFF,
};
/*
 * definintions used in "e_machine"
 */
enum {
	EM_NONE =	0,	// no machine
	EM_M32		=1,	// AT&T WE32100
	EM_SPARC	=2,	// SPARC
	EM_386		=3,	// intel x86
	EM_68K		=4,	// motorola 68000
	EM_88k		=5,	// motorola 88000
	EM_860		=7,	// intel 80860
	EM_MIPS		=8,	// MIPS RS3000
	EM_PPC64=20,
	EM_ARM= 0x28,
	EM_IA_64=50,
	EM_X86_64=64,
};

typedef struct
{
	 u8_t ident[EINIDENT];
	u16_t file_type	__attr_packet;
	u16_t machine	__attr_packet;
	u32_t elf_ver_2	__attr_packet;
	u32_t entry_pt	__attr_packet;
	u32_t phtab_offset	__attr_packet;
	u32_t shtab_offset	__attr_packet;
	u32_t flags		__attr_packet;
	u16_t file_hdr_size	__attr_packet;
	u16_t phtab_ent_size	__attr_packet;
	u16_t num_phtab_ents	__attr_packet;
	u16_t shtab_ent_size	__attr_packet;
	u16_t num_sects	__attr_packet;
	u16_t shstrtab_index	__attr_packet;
} elf_file_t;

/* No flags.  */
#define BFD_NO_FLAGS   	0x00

/* BFD contains relocation entries.  */
#define HAS_RELOC   	0x01

/* BFD is directly executable.  */
#define EXEC_P      	0x02

/* BFD has line number information (basically used for F_LNNO in a
   COFF header).  */
#define HAS_LINENO  	0x04

/* BFD has debugging information.  */
#define HAS_DEBUG   	0x08

/* BFD has symbols.  */
#define HAS_SYMS    	0x10

/* BFD has local symbols (basically used for F_LSYMS in a COFF
   header).  */
#define HAS_LOCALS  	0x20

/* BFD is a dynamic object.  */
#define DYNAMIC     	0x40

/* Text section is write protected (if D_PAGED is not set, this is
   like an a.out NMAGIC file) (the linker sets this by default, but
   clears it for -r or -N).  */
#define WP_TEXT     	0x80

/* BFD is dynamically paged (this is like an a.out ZMAGIC file) (the
   linker sets this by default, but clears it for -r or -n or -N).  */
#define D_PAGED     	0x100

/* BFD is relaxable (this means that bfd_relax_section may be able to
   do something) (sometimes bfd_relax_section can do something even if
   this is not set).  */
#define BFD_IS_RELAXABLE 0x200

/* This may be set before writing out a BFD to request using a
   traditional format.  For example, this is used to request that when
   writing out an a.out object the symbols not be hashed to eliminate
   duplicates.  */
#define BFD_TRADITIONAL_FORMAT 0x400

/* This flag indicates that the BFD contents are actually cached in
   memory.  If this is set, iostream points to a bfd_in_memory struct.  */
#define BFD_IN_MEMORY 0x800


typedef struct elf64_hdr {
  unsigned char	ident[16];		/* ELF "magic number" */
  Elf64_Half file_type;
  Elf64_Half machine;
  Elf64_Word elf_ver_2;
  Elf64_Addr entry_pt;		/* Entry point virtual address */
  Elf64_Off phtab_offset;		/* Program header table file offset */
  Elf64_Off shtab_offset;		/* Section header table file offset */
  Elf64_Word flags;
  Elf64_Half file_hdr_size;
  Elf64_Half phtab_ent_size;
  Elf64_Half num_phtab_ents;
  Elf64_Half shtab_ent_size;
  Elf64_Half num_sects;
  Elf64_Half shstrtab_index;
} elf64_file_t;

/*
 * segment types
 */
enum {
	PT_NULL		=0, PT_LOAD		=1, PT_DYNAMIC	=2, 
	 PT_INTERP	=3, PT_NOTE		=4, PT_SHLIB	=5, 
	 PT_PHDR		=6, PT_LOPROC	=0x70000000, PT_HIPROC	=0x7FFFFFFF
};

/*
 * special section types
 */

#define DT_NULL 0
#define DT_NEEDED 1
#define DT_PLTRELSZ 2
#define DT_PLTGOT 3
#define DT_HASH 4
#define DT_STRTAB 5
#define DT_SYMTAB 6
#define DT_RELA 7
#define DT_RELASZ 8
#define DT_RELAENT 9
#define DT_STRSZ 10
#define DT_SYMENT 11
#define DT_INIT 12
#define DT_FINI 13
#define DT_SONAME 14
#define DT_RPATH 15
#define DT_SYMBOLIC 16
#define DT_REL 17
#define DT_RELSZ 18
#define DT_RELENT 19
#define DT_PLTREL 20
#define DT_DEBUG 21
#define DT_TEXTREL 22
#define DT_JMPREL 23
#define DT_BIND_NOW 24
#define DT_INIT_ARRAY 25
#define DT_FINI_ARRAY 26
#define DT_INIT_ARRAYSZ 27
#define DT_FINI_ARRAYSZ 28
#define DT_RUNPATH 29
#define DT_FLAGS 30
#define DT_ENCODING 32
#define DT_PREINIT_ARRAY 32
#define DT_PREINIT_ARRAYSZ 33
#define DT_LOOS   0x6000000d
#define DT_HIOS   0x6fff0000
#define DT_LOPROC 0x70000000
#define DT_HIPROC 0x7fffffff

/*
 * program header
*/

/*
 * structure that stores information about an process's memory image
 */
typedef struct elfphdr
{
  elfword_t type;
  elfoff_t offset;
  elfaddr_t vaddr;
  elfaddr_t paddr;
  elfword_t filesz;
  elfword_t memsz;
  elfword_t flags;
  elfword_t align;
}elfphdr_t;

typedef struct elf64_phdr {
  Elf64_Word type;
  Elf64_Word flags;
  Elf64_Off offset;		/* Segment file offset */
  Elf64_Addr vaddr;		/* Segment virtual address */
  Elf64_Addr paddr;		/* Segment physical address */
  Elf64_Xword filesz;		/* Segment size in file */
  Elf64_Xword memsz;		/* Segment size in memory */
  Elf64_Xword align;		/* Segment alignment, file & memory */
} elf64phdr_t;

/*
 * struct elf_sect_t describes the header for one section in an elf file
*/
typedef struct
{
	u32_t sect_name	__attr_packet;
	u32_t type		__attr_packet;
	u32_t flags		__attr_packet;
	u32_t virt_adr	__attr_packet;
	u32_t offset		__attr_packet;
	u32_t size		__attr_packet;
	u32_t link		__attr_packet;
	u32_t info		__attr_packet;
	u32_t align		__attr_packet;
	u32_t ent_size	__attr_packet;
} elf_sect_t;

typedef struct elf64_shdr {
  Elf64_Word sect_name;		/* Section name, index in string tbl */
  Elf64_Word type;		/* Type of section */
  Elf64_Xword flags;		/* Miscellaneous section attributes */
  Elf64_Addr virt_adr;		/* Section virtual addr at execution */
  Elf64_Off offset;		/* Section file offset */
  Elf64_Xword size;		/* Size of section in bytes */
  Elf64_Word link;		/* Index of another section */
  Elf64_Word info;		/* Additional section information */
  Elf64_Xword align;	/* Section alignment */
  Elf64_Xword ent_size;	/* Entry size if section holds table */
} elf64_sect_t;

/*
 * symbol table stuff
*/

#define STN_UNDEF	0
#define SHN_UNDEF 0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_HIPROC 0xff1f
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff
/*
 * for ELF32_ST_BIND
 */
enum {
	STB_LOCAl	=0, STB_GLOBAL	=1 /*use it*/, 
	STB_WEAK	=2, STB_LOPROC	=13, STB_HIPROC	=15
};

/*
 * for ELF32_ST_TYPE
 */
enum {
	STT_NOTYPE	=0, STT_OBJECT	=1, STT_FUNC	=2  /*use it*/, 
	STT_SECTION	=3, STT_FILE	=4, STT_LOPROC	=13, STT_HIPROC	=15
};

typedef struct
{
	u32_t name;
	u32_t value;
	u32_t size;

	//unsigned type : 4	__attr_packet;
	//unsigned binding : 4	__attr_packet;
	u8_t info;

	u8_t zero;
	u16_t section;
} elf_sym_t;	

typedef struct elf64_sym {
  Elf64_Word st_name;		/* Symbol name, index in string tbl */
  unsigned char	st_info;	/* Type and binding attributes */
  unsigned char	st_other;	/* No defined meaning, 0 */
  Elf64_Half st_shndx;		/* Associated section index */
  Elf64_Addr st_value;		/* Value of the symbol */
  Elf64_Xword st_size;		/* Associated symbol size */
} elf64_sym_t;


/*
 * some operations on struct elf32_sym
 */
#define ELF32_ST_BIND(i)	((i) >> 4)
#define ELF32_ST_TYPE(i)	((i)&0xF)
#define ELF32_ST_INFO(b,t)	(((b)<<4)+((t)&0xF))

/*
 * relocation entries
*/

typedef struct {
	elfaddr_t	r_offset;
	elfword_t 	r_info;
	}elf32_rel_t;

	/*64 bit*/
typedef struct elf64_rel {
  Elf64_Addr r_offset;	/* Location at which to apply the action */
  Elf64_Xword r_info;	/* index and type of relocation */
} Elf64_Rel;

typedef struct {
	elfaddr_t	r_offset;
	elfword_t 	r_info;
	elfsword_t	r_addend;
	}elf32_rela_t;

	//64 bit
typedef struct elf64_rela {
  Elf64_Addr r_offset;	/* Location at which to apply the action */
  Elf64_Xword r_info;	/* index and type of relocation */
  Elf64_Sxword r_addend;	/* Constant addend used to compute value */
} Elf64_Rela;
/*
 * some operations on above structres
 */
#define ELF32_R_SYM(i)		((i)>>8)
#define ELF32_R_TYPE(i)		((unsigned char)i)
#define ELF32_R_INFO(s,t)	(((s)<<8)+(unsigned char)(t))

/*
 * relocation types
 */
enum{
 R_386_NONE	=0,
 R_386_32	=1,
 R_386_PC32	=2,
 R_386_GOT32	=3,
 R_386_PLT32	=4,
 R_386_COPY	=5,
 R_386_GLOB_DAT	=6,
 R_386_JMP_SLOT	=7,
 R_386_RELATIVE	=8,
 R_386_GOTOFF	=9,
 R_386_GOTPC	=10
};

/* ARM relocs.  */
#define R_ARM_NONE		0	/* No reloc */
#define R_ARM_PC24		1	/* PC relative 26 bit branch */
#define R_ARM_ABS32		2	/* Direct 32 bit  */
#define R_ARM_REL32		3	/* PC relative 32 bit */
#define R_ARM_PC13		4
#define R_ARM_GLOB_DAT		21	/* Create GOT entry */
#define R_ARM_JUMP_SLOT		22	/* Create PLT entry */

extern elf_sym_t *get_elf_sym_byname( elf_file_t *elf32_header,  char *name );
struct Elf32_Dyn {
	Elf32_Sword d_tag;
	union {
		Elf32_Word d_val;
		Elf32_Addr d_ptr;
	} d_un;
};


#endif

