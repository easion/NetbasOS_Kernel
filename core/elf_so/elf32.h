/*
 * giszOS kernel
 * Copyright (C) 2007-2008 Zoltan Kovacs
 *
 * This file contains the ELF32 support of the kernel.
 */
#ifndef __ELF32_H
#define __ELF32_H


struct __atomic_fool_gcc_struct { unsigned long a[100]; };
#define __atomic_fool_gcc(x) (*(volatile struct __atomic_fool_gcc_struct *)(x))

typedef struct {
  volatile int32 counter;
} atomic_t;

#define ATOMIC_INIT(i)  { (i) }

#define atomic_read(v)  ((v)->counter)
#define atomic_set(v,i) (((v)->counter) = (i))

static inline void atomic_add( atomic_t* psAtomic, int nNumber ) {
  __asm__ __volatile__( "lock ; addl %1, %0\n"
    : "=m" ( psAtomic->counter )
    : "ir" ( nNumber ), "m" ( psAtomic->counter ) );
}

static inline void atomic_sub( atomic_t* psAtomic, int nNumber ) {
  __asm__ __volatile__( "lock ; subl %1, %0\n"
    : "=m" ( psAtomic->counter )
    : "ir" ( nNumber ), "m" ( psAtomic->counter ) );
}

static inline void atomic_or( atomic_t* psAtomic, int nNumber ) {
  __asm__ __volatile__( "lock ; orl %1, %0\n"
    : "=m" ( psAtomic->counter )
    : "ir" ( nNumber ), "m" ( psAtomic->counter ) );
}

static inline void atomic_and( atomic_t* psAtomic, int nNumber ) {
  __asm__ __volatile__( "lock ; andl %1, %0\n"
    : "=m" ( psAtomic->counter )
    : "ir" ( nNumber ), "m" ( psAtomic->counter ) );
}

static inline int atomic_sub_and_test( atomic_t* psAtomic, int nNumber ) {
  uint8 c;

  __asm__ __volatile__( "lock ; subl %2, %0; sete %1\n"
    : "=m" ( psAtomic->counter ), "=qm" (c)
    : "ir" ( nNumber ), "m" ( psAtomic->counter )
    : "memory" );

  return c;
}

static inline void atomic_inc( atomic_t* psAtomic ) {
  __asm__ __volatile__( "lock ; incl %0\n"
    : "=m" ( psAtomic->counter )
    : "m" ( psAtomic->counter ) );
}

static inline void atomic_dec( atomic_t* psAtomic ) {
  __asm__ __volatile__( "lock ; decl %0\n"
    : "=m" ( psAtomic->counter )
    : "m" ( psAtomic->counter ) );
}

static inline int atomic_dec_and_test( atomic_t* psAtomic ) {
  uint8 c;

  __asm__ __volatile__( "lock ; decl %0; sete %1\n"
    : "=m" ( psAtomic->counter ), "=qm" ( c )
    : "m" ( psAtomic->counter )
    : "memory" );

  return ( c != 0 );
}

static inline int atomic_inc_and_test( atomic_t* psAtomic ) {
  uint8 c;

  __asm__ __volatile__( "lock ; incl %0; sete %1\n"
    : "=m" ( psAtomic->counter ), "=qm" ( c )
    : "m" ( psAtomic->counter )
    : "memory" );

  return ( c != 0 );
}

static inline int atomic_inc_and_read( atomic_t* psAtomic ) {
  int nOldVal;
  int nTmp = 0;

  __asm__ __volatile__( "movl %3, %0\n"
    "0:\n"
    "movl %0, %2\n"
    "incl %2\n"
    "lock ; cmpxchgl %2, %3\n"
    "jnz 0b\n"
    : "=&a" ( nOldVal ), "=m" ( psAtomic->counter ), "=&q" ( nTmp )
    : "m" ( psAtomic->counter )
    : "memory" );

  return nOldVal;
}

static inline int atomic_add_negative( atomic_t* psAtomic, int nNumber ) {
  uint8 c;

  __asm__ __volatile__( "lock ; addl %2,%0; sets %1\n"
    : "=m" ( psAtomic->counter ), "=qm" ( c )
    : "ir" ( nNumber ), "m" ( psAtomic->counter )
    : "memory" );

  return c;
}

static inline int atomic_swap( volatile void* pAtomic, int nNumber ) {
  __asm__ __volatile__( "xchgl %0, %1"
    : "=r" ( nNumber )
    : "m" ( __atomic_fool_gcc( pAtomic ) ), "0" ( nNumber )
    : "memory" );

  return nNumber;
}


typedef long area_id;
typedef long sem_id;

typedef struct MemoryContext_s MemoryContext_t;
typedef struct AreaInfo_s AreaInfo_t;

typedef struct BootModule_s BootModule_t;


struct MemoryContext_s {
  int mc_bBusy;

  int mc_nAreaCount;
  int mc_nMaxAreaCount;
 // struct MemoryArea_s** mc_ppsAreas;

  void* mc_pArchData;

  struct MemoryContext_s* mc_psNext;
};

struct AreaInfo_s {
  area_id ai_nAreaID;
  size_t ai_nSize;
  int ai_nLock;
  uint32 ai_nProtection;
  void* ai_pAddress;
};


struct BootModule_s {
  int bm_bIsValid;

  void* bm_pAddress;
  uint32 bm_nSize;
  const char* bm_pcParameters;
  area_id bm_nAreaID;
}; // struct BootModule_s


enum {
  MEM_NONE = 0x00,
  MEM_CLEAR = 0x01,
  MEM_NOBLOCK = 0x02,
  MEM_OKTOFAILHACK = 0x04,
  MEM_DEBUG = 0x08
};

  enum {
  AREA_ANY_ADDRESS = 0x0000,
  AREA_EXACT_ADDRESS = 0x0100,
  AREA_BASE_ADDRESS = 0x0200,
  AREA_ADDR_SPEC_MASK = 0x0F00,
  AREA_TOP_DOWN = 0x1000
};


enum {
  FDT_FILE = 1,
  FDT_DIR,
  FDT_SYMLINK,
  FDT_ATTR_DIR,
  FDT_INDEX_DIR,
  FDT_SOCKET,
  FDT_FIFO
};

  enum {
  AREA_NO_LOCK = 0x00,
  AREA_LAZY_LOCK = 0x01,
  AREA_FULL_LOCK = 0x02,
  AREA_CONTIGUOUS = 0x03
};

#define LOCK
#define UNLOCK

#define PAGE_MASK  ( ~( PAGE_SIZE - 1 ) )
enum {
  AREA_READ = 0x01,
  AREA_WRITE = 0x02,
  AREA_EXEC = 0x04,
  AREA_FULL_ACCESS = ( AREA_READ | AREA_WRITE | AREA_EXEC ),
  AREA_KERNEL = 0x08,
  AREA_WRCOMB = 0x10
};

  #define AREA_FIRST_USER_ADDRESS   0x80000000
#define PATH_MAX 255
enum {
  SEM_RECURSIVE = 0x0001,
  SEM_WARN_DBL_LOCK = 0x0002,
  SEM_WARN_DBL_UNLOCK = 0x0004,
  SEM_GLOBAL = 0x0008
};


#define ENOSYM ENOENT
#define CURRENT_PROC_IMAGE_CONTEXT 0//CURRENT_PROCESS->p_psImageContext


#define MAX_IMAGE_COUNT 256

#define ELF_KERNEL_SYM_HASHTAB_SIZE 1024

#define ELF32_R_SYM(i)    ((i)>>8)
#define ELF32_R_TYPE(i)   ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8) + (unsigned char) (t) )

#define ELF32_ST_BIND(i)    (((i)>>4) & 0x0f)
#define ELF32_ST_TYPE(i)    ((i) & 0x0f)
#define ELF32_ST_INFO(b,t)  (((b) << 4) + ((t) & 0x0f))

typedef struct ELF32SectionHeader_s ELF32SectionHeader_t;
typedef struct ELF32ElfHeader_s ELF32ElfHeader_t;
typedef struct ELF32Reloc_s ELF32Reloc_t;
typedef struct ELF32HashTable_s ELF32HashTable_t;
typedef struct ELF32Symbol_s ELF32Symbol_t;
typedef struct ELF32Dynamic_s ELF32Dynamic_t;

typedef struct ELFSymbol_s ELFSymbol_t;
typedef struct ELFImageInstance_s ELFImageInstance_t;
typedef struct ELFImage_s ELFImage_t;
typedef struct ImageContext_s ImageContext_t;
typedef int Inode_t;

enum {
  SHF_WRITE  = 0x01,
  SHF_ALLOC  = 0x02,
  SHF_EXECINST = 0x04,
  SHF_MASCPROC = 0xF0000000
};



enum {
  R_386_NONE,
  R_386_32,
  R_386_PC32,
  R_386_GOT32,
  R_386_PLT32,
  R_386_COPY,
  R_386_GLOB_DATA,
  R_386_JMP_SLOT,
  R_386_RELATIVE,
  R_386_GOTOFF,
  R_386_GOTPC
};

enum {
  EI_MAG0,
  EI_MAG1,
  EI_MAG2,
  EI_MAG3,
  EI_CLASS,
  EI_DATA,
  EI_VERSION,
  EI_PAD,
  EI_NIDENT = 16
};

enum {
  DT_NULL,
  DT_NEEDED,
  DT_PLTRELSZ,
  DT_PLTGOT,
  DT_HASH,
  DT_STRTAB,
  DT_SYMTAB,
  DT_RELA,
  DT_RELASZ,
  DT_RELAENT,
  DT_STRSZ,
  DT_SYMENT,
  DT_INIT,
  DT_FINI,
  DT_SONAME,
  DT_RPATH,
  DT_SYMBOLIC,
  DT_REL,
  DT_RELSZ,
  DT_RELENT,
  DT_PLTREL,
  DT_DEBUG,
  DT_TEXTREL,
  DT_JMPREL,
  DT_LOPROC  = 0x70000000,
  DT_HIPROC  = 0x7FFFFFFF
};

enum {
  STB_LOCAL,
  STB_GLOBAL,
  STB_WEAK,
  STB_LOPROC   = 13,
  STB_EXPORTED = STB_LOPROC,
  STB_HIPROC   = 15
};

enum {
  STT_NOTYPE,
  STT_OBJECT,
  STT_FUNC,
  STT_SECTION,
  STT_FILE,
  STT_LOPROC  = 13,
  STT_HIPROC  = 15
};

enum {
  STN_UNDEF
};

enum {
  SHT_NULL,
  SHT_PROGBITS,
  SHT_SYMTAB,
  SHT_STRTAB,
  SHT_RELA,
  SHT_HASH,
  SHT_DYNAMIC,
  SHT_NOTE,
  SHT_NOBITS,
  SHT_REL,
  SHT_SHLIB,
  SHT_DYNSYM,
  SHT_LOPROC  = 0x70000000,
  SHT_HIPROC  = 0x7FFFFFFF,
  SHT_LOUSER  = 0x80000000,
  SHT_RELOC_MAP,
  SHT_LIBTAB,
  SHT_HIUSER  = 0xFFFFFFFF
};

enum {
  SHN_UNDEF,
  SHN_LORESERVE = 0xFF00,
  SHN_LOPROC    = 0xFF00,
  SHN_HIPROC    = 0xFF1F,
  SHN_ABS       = 0xFFF1,
  SHN_COMMON    = 0xFFF2,
  SHN_HIRESERVE = 0xFFFF
};

enum {
  IM_KERNEL_SPACE,
  IM_APP_SPACE,
  IM_ADDON_SPACE,
  IM_LIBRARY_SPACE,
};

struct ELF32SectionHeader_s {
  int esh_nName;
  int esh_nType;
  u32_t esh_nFlags;
  u32_t esh_nAddress;
  u32_t esh_nOffset;
  u32_t esh_nSize;
  u32_t esh_nLink;
  u32_t esh_nInfo;
  u32_t esh_nAddrAlign;
  u32_t esh_nEntrySize;
}; // struct ELF32SectionHeader_s

struct ELF32ElfHeader_s {
  char eeh_anIdent[ EI_NIDENT ];
  uint16 eeh_nType;
  uint16 eeh_nMachine;
  int eeh_nVersion;
  u32_t eeh_nEntry;
  int eeh_nProgHdrOff;
  int eeh_nSecHdrOff;
  u32_t eeh_nFlags;
  uint16 eeh_nElfHdrSize;
  uint16 eeh_nProgHdrEntrySize;
  uint16 eeh_nProgHdrCount;
  uint16 eeh_nSecHdrSize;
  uint16 eeh_nSecHdrCount;
  uint16 eeh_nSecNameStrTabIndex;
}; // struct ELF32ElfHeader_s

struct ELF32Reloc_s {
  u32_t er_nOffset;
  u32_t er_nInfo;
}; // struct ELF32Reloc_s

struct ELF32HashTable_s {
  u32_t eht_nBucketEntries;
  u32_t eht_nChainEntries;
}; // struct ELF32HashTable_s

struct ELF32Symbol_s {
  int es_nName;
  u32_t es_nValue;
  int es_nSize;
  uint8 es_nInfo;
  char es_nOther;
  int16 es_nSecIndex;
}; // struct ELF32Symbol_s

struct ELF32Dynamic_s {
  int32 ed_nTag;

  union {
    int32 ed_nValue;
    u32_t ed_nPointer;
  } ed_un;
}; // struct ELF32Dynamic_s

struct ELFSymbol_s {
  struct ELFSymbol_s* es_psHashNext;
  const char* es_pcName;
  uint16 es_nInfo;
  int16 es_nImage;
  u32_t es_nAddress;
  int es_nSize;
  int es_nSection;
}; // struct ELFSymbol_s

struct ELFImageInstance_s {
  struct ELFImage_s* eii_psImage;
  struct ELFImageInstance_s** eii_ppsSubImages;
  int eii_nHandle;
  atomic_t eii_nOpenCount;
  atomic_t eii_nAppOpenCount;
  u32_t eii_nTextAddress;
  area_id eii_nTextArea;
  area_id eii_nDataArea;
  int eii_bRelocated;
}; // struct ELFImageInstance_s

struct ELFImage_s {
  struct ELFImage_s* ei_psNext;
  char ei_acName[ 64 ];
  char* ei_pcPath;
  char* ei_pcArguments;
  atomic_t ei_nOpenCount;
  int ei_nSectionCount;
  struct ELF32SectionHeader_s* ei_psSections;

  char* ei_pcStrings;
  int ei_nSymCount;
  struct ELFSymbol_s* ei_psSymbols;
  struct ELFSymbol_s* ei_apsKernelSymHash[ ELF_KERNEL_SYM_HASHTAB_SIZE ];
  int ei_nRelocCount;
  struct ELF32Reloc_s* ei_psRelocs;

  struct ELF32HashTable_s* ei_psHashTable;
  u32_t* ei_pnHashBucket;
  u32_t* ei_pnHashChain;

  int ei_nSubImageCount;
  const char** ei_ppcSubImages;
  u32_t ei_nVirtualAddress;
  u32_t ei_nTextSize;
  u32_t ei_nOffset;
  u32_t ei_nEntry;
  u32_t ei_nInit;
  u32_t ei_nFini;

  u32_t ei_nCtors;
  int ei_nCtorCount;

  struct File_s* ei_psFile;
  BootModule_t* ei_psModule;
}; // struct ELFImage_s

struct ImageContext_s {
  struct ELFImageInstance_s* ic_psInstances[ MAX_IMAGE_COUNT ];
  sem_id ic_nLock;
}; // struct ImageContext_s

typedef int image_entry( char** argv, char** envv );
typedef int image_init( int nImageID );
typedef void image_term( int nImageID );

typedef struct {
  int    ii_image_id;   // image id.
  int    ii_type;   // image type (IM_APP_SPACE, IM_ADDON_SPACE or IM_LIBRARY_SPACE).
  int    ii_open_count;   // open count private to the owner process.
  int    ii_sub_image_count;  // number of dll's linked to this image.
  int    ii_init_order;   // the larger the later to be initiated.
  image_entry* ii_entry_point;  // entry point (_start())
  image_init*  ii_init;   // init routine. (global contructors)
  image_term*  ii_fini;   // terminate routine. (global destructors)
  //dev_t  ii_device;   // device number for file.
  //int    ii_inode;    // inode number for file.
  char   ii_name[ 256 ];    // full pathname of image.
  void*  ii_text_addr;    // address of text segment.
  void*  ii_data_addr;    // address of data segment.
  int    ii_text_size;    // size of text segment.
  int    ii_data_size;    // size of data segment (including BSS).
  void*  ii_ctor_addr;
  int    ii_ctor_count;
} ImageInfo_t;


/**
 * Loads the file specified by the path as a kernel driver.
 *
 * @param a_pcPath The path of the driver to load
 * @return On error a negative value is returned, any other value means success
 */
int load_kernel_driver( const char* a_pcPath );
/**
 * Unloads a previously loaded kernel driver.
 *
 * @param nLibrary The handle of the kernel driver to unload
 * @return On error a negative value is returned, any other value represents success
 */
int unload_kernel_driver( int nLibrary );
/**
 * Gets the address of the specified symbol in a loaded ELF library.
 *
 * @param nLibrary The handle of the library in which the symbol will be searched
 * @param pcName The name of the symbol
 * @param nIndex The index where the search will be started in the symbol table
 * @param ppPtr A pointer where the address of the symbol will be passed to the caller
 * @return On success 0 is returned
 */
int get_symbol_address( int nLibrary, const char* pcName, int nIndex, void** ppPtr );

/**
 * Loads an ELF image.
 *
 * @param psContext The image context where the new ELF image should be loaded
 * @param pcPath The path of the ELF image file in the case of loading it from a file
 * @param psModule The bootmodule instance if the ELF image should be loaded from a bootmodule
 * @param nMode The image space where the new library will be loaded, eg. kernel/application space
 * @param ppsInstance The new instance of the loaded ELF image will be passed to the caller through this pointer
 * @param pcLibraryPath The path where the shared libraries required by the ELF image would be searched
 * @return On error a negative value is returned
 */
int load_image_instance( ImageContext_t* psContext, const char* pcPath, BootModule_t* psModule, int nMode, ELFImageInstance_t** ppsInstance, const char* pcLibraryPath );

/**
 * Gets info from the specified image.
 *
 * @param bKernel True when the request is coming from the kernel, false if the request is from the userspace
 * @param nImage The image to request the information from
 * @param nSubImage If we want info from the image specified by the nImage parameter this should be -1
 *                  If this parameter is a valid image index then the info will be returned about the 
 *                  specified subimage of the image referenced by nImage
 * @param psInfo A pointer to the image info structure. The info from the image will be stored here
 * @return On error a negative value is returned
 */
int get_image_info( int bKernel, int nImage, int nSubImage, ImageInfo_t* psInfo );
int find_module_by_address( const void *pAddress );
int get_symbol_by_address( int nLibrary, const char *pAddress, char *pzName, int nMaxNamLen, void **ppAddress );

ImageContext_t* create_image_context( void );
ImageContext_t* clone_image_context( ImageContext_t* psOrigContext, MemoryContext_t* psMemoryContext );
void delete_image_context( ImageContext_t* psContext );

void close_all_images( ImageContext_t* psContext );

void init_elf_loader( void );

#endif
