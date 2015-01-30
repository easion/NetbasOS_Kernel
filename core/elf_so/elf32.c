#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/module.h>
#include <string.h>
#include <assert.h>
#include "elf32.h"

atomic_t si_nLoadedImageCount;
  atomic_t si_nImageInstanceCount;

static sem_id g_nImageListLock;
static ELFImage_t* g_psFirstImage;

static ImageContext_t* g_psKernelImageContext;

static inline u32_t elf_sym_hash( const char* pcName ) {
  register u32_t nHash = 0;

  while ( *pcName != 0 ) {
    nHash = ( nHash << 2 ) + ( *pcName++ ) * 1009;
  }

  return nHash;
}

static inline u32_t elf_sysv_sym_hash( const char* pcName ) {
  u32_t nHash = 0, g;

  while ( *pcName != 0 ) {
    nHash = ( nHash << 4 ) + ( *pcName++ );

    if ( ( g = ( nHash & 0xF0000000 ) ) ) {
      nHash ^= g >> 24;
    }

    nHash &= ~g;
  }

  return nHash;
}

int read_image_data( ELFImage_t* psImage, void* pBuffer, off_t nOffset, size_t nSize ) {
  if ( psImage->ei_psModule != NULL ) {
    if ( nOffset + nSize > psImage->ei_psModule->bm_nSize ) {
      nSize = psImage->ei_psModule->bm_nSize - nOffset;
    }

    if ( nSize <= 0 ) {
      return 0;
    }

    memcpy( pBuffer, ( char* )psImage->ei_psModule->bm_pAddress + nOffset, nSize );

    return nSize;
  } else {
	  fseek(psImage->ei_psFile, nOffset, 0);
    //return read_pos_p( psImage->ei_psFile, nOffset, pBuffer, nSize );
    return fread(pBuffer,   nSize , 1,psImage->ei_psFile);
  }
}

static int parse_dynamic_section( ELFImage_t* psImage, ELF32SectionHeader_t* psSection ) {
  int i;
  int nLibNum;
  int nDynRelSize = 0;
  u32_t nDynRelAddress = -1;
  int nDynPltRelSize = 0;
  u32_t nDynPltRelAddress = -1;
  int nDynRelEntSize = sizeof( ELF32Reloc_t );
  ELF32Dynamic_t* psDynTab;
  int nError;

  psDynTab = ( ELF32Dynamic_t* )kmalloc( psSection->esh_nSize, MEM_OKTOFAILHACK );
  if ( psDynTab == NULL ) {
    kprintf( "parse_dynamic_section(): No memory for dynamic section\n" );
    nError = -ENOMEM;
    goto error1;
  }

  nError = read_image_data( psImage, psDynTab, psSection->esh_nOffset, psSection->esh_nSize );
  if ( nError != psSection->esh_nSize ) {
    kprintf( "parse_dynamic_section(): Failed to load dynamic section\n" );

    if ( nError >= 0 ) {
      nError = -ENOEXEC;
    }

    goto error2;
  }

  for ( i = 0; i < psSection->esh_nSize / psSection->esh_nEntrySize; i++ ) {
    ELF32Dynamic_t* psDyn = &psDynTab[ i ];

    switch ( psDyn->ed_nTag ) {
      case DT_NEEDED :
        psImage->ei_nSubImageCount++;
        break;

      case DT_REL :
        nDynRelAddress = psDyn->ed_un.ed_nPointer;
        break;

      case DT_RELSZ :
        nDynRelSize = psDyn->ed_un.ed_nValue;
        break;

      case DT_JMPREL :
        nDynPltRelAddress = psDyn->ed_un.ed_nPointer;
        break;

      case DT_PLTRELSZ :
        nDynPltRelSize = psDyn->ed_un.ed_nValue;
        break;

      case DT_RELENT :
        nDynRelEntSize = psDyn->ed_un.ed_nValue;
        break;

      case DT_INIT :
        psImage->ei_nInit = psDyn->ed_un.ed_nPointer;
        break;

      case DT_FINI :
        psImage->ei_nFini = psDyn->ed_un.ed_nPointer;
        break;

      default:
        break;
    }
  }

  nDynRelEntSize = 8;

  if ( nDynRelSize + nDynPltRelSize > 0 ) {
    psImage->ei_psRelocs = ( ELF32Reloc_t* )kmalloc( nDynRelSize + nDynPltRelSize, MEM_OKTOFAILHACK | MEM_DEBUG );
    if ( psImage->ei_psRelocs == NULL ) {
      kprintf( "parse_dynamic_section(): No memory for reloc table\n" );
      nError = -ENOMEM;
      goto error2;
    }

    if ( nDynRelSize > 0 ) {
      nError = read_image_data( psImage, psImage->ei_psRelocs, nDynRelAddress - psImage->ei_nVirtualAddress + psImage->ei_nOffset, nDynRelSize );

      if ( nError != nDynRelSize ) {
        kprintf( "parse_dynamic_section(): Failed to read reloc table\n" );

        if ( nError >= 0 ) {
          nError = -ENOEXEC;
        }

        goto error3;
      }
    }

    if ( nDynPltRelSize > 0 ) {
      nError = read_image_data( psImage, ( ( char* )psImage->ei_psRelocs ) + nDynRelSize, nDynPltRelAddress - psImage->ei_nVirtualAddress + psImage->ei_nOffset, nDynPltRelSize );

      if ( nError != nDynPltRelSize ) {
        kprintf( "parse_dynamic_section(): Failed to read plt reloc table\n" );

        if ( nError >= 0 ) {
          nError = -ENOEXEC;
        }

        goto error3;
      }
    }

    psImage->ei_nRelocCount = ( nDynRelSize + nDynPltRelSize ) / sizeof( ELF32Reloc_t );
  }

  if ( psImage->ei_nSubImageCount == 0 ) {
    psImage->ei_ppcSubImages = NULL;
  } else {
    psImage->ei_ppcSubImages = ( const char** )kmalloc( sizeof( char* ) * psImage->ei_nSubImageCount, MEM_CLEAR | MEM_OKTOFAILHACK );
    if ( psImage->ei_ppcSubImages == NULL ) {
      kprintf( "parse_dynamic_section(): No memory for subimage list\n" );
      nError = -ENOMEM;
      goto error3;
    }

    nLibNum = 0;

    for ( i = 0; i < psSection->esh_nSize / psSection->esh_nEntrySize; i++ ) {
      ELF32Dynamic_t* psDyn = &psDynTab[ i ];

      if ( psDyn->ed_nTag == DT_NEEDED ) {
        psImage->ei_ppcSubImages[ nLibNum ] = psImage->ei_pcStrings + psDyn->ed_un.ed_nValue;
        nLibNum++;
      }
    }
  }

  kfree( psDynTab );

  return 0;

error3:
  kfree( psImage->ei_psRelocs );
  psImage->ei_psRelocs = NULL;

error2:
  kfree( psDynTab );

error1:
  return nError;
}

static int parse_section_headers( ELFImage_t* psImage ) {
  ELF32Symbol_t* psSymTab = NULL;
  int nDynSymSec = -1;
  int nDynamicSec = -1;
  int nHashSec = -1;
  u32_t nEndAddress = 0;
  int nError = 0;
  int i;

  psImage->ei_nVirtualAddress = -1;

  for ( i = 0; i < psImage->ei_nSectionCount; i++ ) {
    ELF32SectionHeader_t* psSection;

    psSection = &psImage->ei_psSections[ i ];

    if ( ( psSection->esh_nType != SHT_NULL ) && ( psImage->ei_nVirtualAddress == -1 ) ) {
      psImage->ei_nVirtualAddress = psSection->esh_nAddress;
      psImage->ei_nOffset = psSection->esh_nOffset;
    }

    switch ( psSection->esh_nType ) {
      case SHT_PROGBITS :
        if ( psSection->esh_nAddress + psSection->esh_nSize > nEndAddress ) {
          nEndAddress = psSection->esh_nAddress + psSection->esh_nSize;
        }

        break;

      case SHT_DYNSYM :
        nDynSymSec = i;
        break;

      case SHT_DYNAMIC :
        nDynamicSec = i;
        break;

      case SHT_HASH :
        nHashSec = i;
        break;
    }
  }

  if ( psImage->ei_nVirtualAddress == -1 ) {
    kprintf( "parse_section_headers(): Image has no text segment!\n" );
    return -ENOEXEC;
  }

  psImage->ei_nOffset -= psImage->ei_nVirtualAddress & ~PAGE_MASK;
  psImage->ei_nVirtualAddress &= PAGE_MASK;

  psImage->ei_nTextSize = nEndAddress - psImage->ei_nVirtualAddress;

  if ( nDynSymSec != -1 ) {
    void* pCtorStart = NULL;
    void* pCtorEnd = NULL;

    ELF32SectionHeader_t* psSection = &psImage->ei_psSections[ nDynSymSec ];
    ELF32SectionHeader_t* psStrSection = &psImage->ei_psSections[ psSection->esh_nLink ];

    psImage->ei_nSymCount = psSection->esh_nSize / psSection->esh_nEntrySize;

    psSymTab = ( ELF32Symbol_t* )kmalloc( psSection->esh_nSize, MEM_OKTOFAILHACK );
    if ( psSymTab == NULL ) {
      kprintf( "parse_section_headers(): No memory for raw symbol table\n" );
      nError = -ENOMEM;
      goto error1;
    }

    psImage->ei_pcStrings = ( char* )kmalloc( psStrSection->esh_nSize, MEM_OKTOFAILHACK );
    if ( psImage->ei_pcStrings == NULL ) {
      kprintf( "parse_section_headers(): No memory for string table\n" );
      nError = -ENOMEM;
      goto error2;
    }

    nError = read_image_data( psImage, psImage->ei_pcStrings, psStrSection->esh_nOffset, psStrSection->esh_nSize );
    if ( nError != psStrSection->esh_nSize ) {
      kprintf( "parse_section_headers(): Failed to load string table\n" );

      if ( nError >= 0 ) {
        nError = -ENOEXEC;
      }

      goto error3;
    }

    nError = read_image_data( psImage, psSymTab, psSection->esh_nOffset, psSection->esh_nSize );
    if ( nError != psSection->esh_nSize ) {
      kprintf( "parse_section_headers(): Failed to load symbol table\n" );

      if ( nError >= 0 ) {
        nError = -ENOEXEC;
      }

      goto error3;
    }

    psImage->ei_psSymbols = ( ELFSymbol_t* )kmalloc( psImage->ei_nSymCount * sizeof( ELFSymbol_t ), MEM_OKTOFAILHACK );
    if ( psImage->ei_psSymbols == NULL ) {
      kprintf( "parse_section_headers(): No memory for symbol table\n" );
      nError = -ENOMEM;
      goto error3;
    }

    for ( i = 0; i < psImage->ei_nSymCount; i++ ) {
      psImage->ei_psSymbols[ i ].es_pcName = psImage->ei_pcStrings + psSymTab[ i ].es_nName;
      psImage->ei_psSymbols[ i ].es_nInfo = psSymTab[ i ].es_nInfo;
      psImage->ei_psSymbols[ i ].es_nAddress = psSymTab[ i ].es_nValue;
      psImage->ei_psSymbols[ i ].es_nSize = psSymTab[ i ].es_nSize;
      psImage->ei_psSymbols[ i ].es_nSection = psSymTab[ i ].es_nSecIndex;
      psImage->ei_psSymbols[ i ].es_nImage = 0;

      if ( strcmp( psImage->ei_psSymbols[ i ].es_pcName, "start_ctors" ) == 0 ) {
        pCtorStart = ( void* )psImage->ei_psSymbols[ i ].es_nAddress;
      } else if ( strcmp( psImage->ei_psSymbols[ i ].es_pcName, "end_ctors" ) == 0 ) {
        pCtorEnd = ( void* )psImage->ei_psSymbols[ i ].es_nAddress;
      }
    }

    kfree( psSymTab );
    psSymTab = NULL;

    if ( ( pCtorStart != NULL ) && ( pCtorEnd != NULL ) ) {
      psImage->ei_nCtors = ( u32_t )pCtorStart;
      psImage->ei_nCtorCount = ( int )( pCtorEnd - pCtorStart ) / sizeof( void* );
    }
  }

  if ( nDynSymSec != -1 ) {
    nError = parse_dynamic_section( psImage, &psImage->ei_psSections[ nDynamicSec ] );
    if ( nError < 0 ) {
      goto error4;
    }
  }

  if ( nHashSec != -1 ) {
    ELF32SectionHeader_t* psSection = &psImage->ei_psSections[ nHashSec ];

    psImage->ei_psHashTable = ( ELF32HashTable_t* )kmalloc( psSection->esh_nSize, MEM_OKTOFAILHACK );
    if ( psImage->ei_psHashTable == NULL ) {
      kprintf( "parse_section_headers(): No memory for hash symbol table\n" );
      nError = -ENOMEM;
      goto error4;
    }

    nError = read_image_data( psImage, psImage->ei_psHashTable, psSection->esh_nOffset, psSection->esh_nSize );
    if ( nError != psSection->esh_nSize ) {
      kprintf( "parse_section_headers(): Failed to load hash table\n" );

      if ( nError >= 0 ) {
        nError = -ENOEXEC;
      }

      goto error5;
    }

    psImage->ei_pnHashBucket = ( u32_t* )( psImage->ei_psHashTable + 1 );
    psImage->ei_pnHashChain = psImage->ei_pnHashBucket + psImage->ei_psHashTable->eht_nBucketEntries;
  }

  return 0;

error5:
  kfree( psImage->ei_psHashTable );
  psImage->ei_psHashTable = NULL;

error4:
  kfree( psImage->ei_psSymbols );
  psImage->ei_psSymbols = NULL;

error3:
  kfree( psImage->ei_pcStrings );
  psImage->ei_pcStrings = NULL;

error2:
  kfree( psSymTab );

error1:
  return nError;
}

static inline ELFSymbol_t* lookup_symbol( ELFImage_t* psImage, const char* pcName ) {
  ELFSymbol_t* psSym;

  if ( psImage->ei_psHashTable ) {
    ELF32HashTable_t* psHashTab = psImage->ei_psHashTable;
    register u32_t nSysvHash = elf_sysv_sym_hash( pcName );

    nSysvHash %= psHashTab->eht_nBucketEntries;

    if ( psImage->ei_pnHashBucket[ nSysvHash ] != STN_UNDEF ) {
      u32_t nSymbol = psImage->ei_pnHashBucket[ nSysvHash ];

      do {
        psSym = &psImage->ei_psSymbols[ nSymbol ];

        if ( ( psSym->es_nSection != SHN_UNDEF ) && ( strcmp( psSym->es_pcName, pcName ) == 0 ) ) {
          return psSym;
        }

        nSymbol = psImage->ei_pnHashChain[ nSymbol ];
      } while ( nSymbol != STN_UNDEF );
    }
  } else {
    register u32_t nHash = elf_sym_hash( pcName ) % ELF_KERNEL_SYM_HASHTAB_SIZE;

    for ( psSym = psImage->ei_apsKernelSymHash[ nHash ]; psSym != NULL; psSym = psSym->es_psHashNext ) {
      if ( ( psSym->es_nSection != SHN_UNDEF ) && ( strcmp( psSym->es_pcName, pcName ) == 0 ) ) {
        return psSym;
      }
    }
  }

  return NULL;
}

static int find_symbol( ImageContext_t* psContext, const char* pcName, ELFImageInstance_t** ppsInstance, ELFSymbol_t** ppsSym ) {
  int i;
  ELFSymbol_t* psSym;
  ELFSymbol_t* psWeak = NULL;
  ELFImageInstance_t* psWeakInstance = NULL;

  for ( i = 0; i < MAX_IMAGE_COUNT; i++ ) {
    ELFImage_t* psImage;
    ELFImageInstance_t* psInstance = psContext->ic_psInstances[ i ];

    if ( psInstance == NULL ) {
      continue;
    }

    psImage = psInstance->eii_psImage;
    psSym = lookup_symbol( psImage, pcName );

    if ( psSym != NULL ) {
      if ( ELF32_ST_BIND( psSym->es_nInfo ) == STB_LOCAL ) {
        kprintf( "Warning: find_symbol() Found local symbol '%s'\n", pcName );
      }

      if ( ELF32_ST_BIND( psSym->es_nInfo ) == STB_WEAK ) {
        if ( psWeak == NULL ) {
          psWeak = psSym;
          psWeakInstance = psInstance;
        }
      } else {
        *ppsSym = psSym;
        *ppsInstance = psInstance;

        return 0;
      }
    }
  }

  if ( psWeak != NULL ) {
    *ppsSym = psWeak;
    *ppsInstance = psWeakInstance;

    return 0;
  }

  return -ENOSYM;
}

static void unload_image( ELFImage_t* psImage ) {
  int nLastUnload = 1;

  if ( atomic_read( &psImage->ei_nOpenCount ) != -1 ) {
    nLastUnload = atomic_dec_and_test( &psImage->ei_nOpenCount );

    if ( atomic_read( &psImage->ei_nOpenCount ) < 0 ) {
      kprintf( "PANIC: unload_image() Image %s got open count of %d\n", psImage->ei_acName, atomic_read( &psImage->ei_nOpenCount ) );
      return;
    }
  }

  if ( nLastUnload ) {
    ELFImage_t** ppsTmp;

    LOCK( g_nImageListLock );

    for ( ppsTmp = &g_psFirstImage; *ppsTmp != NULL; ppsTmp = &( *ppsTmp )->ei_psNext ) {
      if ( *ppsTmp == psImage ) {
        *ppsTmp = psImage->ei_psNext;
        break;
      }
    }

    UNLOCK( g_nImageListLock );

    if ( psImage->ei_psSections != NULL ) {
      kfree( psImage->ei_psSections );
      psImage->ei_psSections = NULL;
    }

    if ( psImage->ei_psSymbols != NULL ) {
      kfree( psImage->ei_psSymbols );
      psImage->ei_psSymbols = NULL;
    }

    if ( psImage->ei_psRelocs != NULL ) {
      kfree( psImage->ei_psRelocs );
      psImage->ei_psRelocs = NULL;
    }

    if ( psImage->ei_psHashTable != NULL ) {
      kfree( psImage->ei_psHashTable );
      psImage->ei_psHashTable = NULL;
    }

    if ( psImage->ei_ppcSubImages != NULL ) {
      kfree( psImage->ei_ppcSubImages );
      psImage->ei_ppcSubImages = NULL;
    }

    if ( psImage->ei_pcStrings != NULL ) {
      kfree( psImage->ei_pcStrings );
      psImage->ei_pcStrings = NULL;
    }

    if ( psImage->ei_psFile != NULL ) {
      put_fd( psImage->ei_psFile );
    }

    kfree( psImage );
    atomic_dec( &si_nLoadedImageCount );
  }
}

#define CMP_INODES( a, b ) ((a)->i_nInode == (b)->i_nInode && (a)->i_psVolume->v_nDevNum == (b)->i_psVolume->v_nDevNum)

static ELFImage_t* find_image( struct filp* psFile ) {
  ELFImage_t* psImage;
#if 0 //fixme,dpp
  if ( psFile == NULL ) {
    kprintf( "find_image(): psFile == NULL\n" );
    return NULL;
  }

  for ( psImage = g_psFirstImage; psImage != NULL; psImage = psImage->ei_psNext ) {
    if ( ( psImage->ei_psFile != NULL ) && CMP_INODES( psImage->ei_psFile->f_psInode, psFile->f_psInode ) ) {
      return psImage;
    }
  }
#endif
  return NULL;
}

static int load_image( const char* pcImageName, const char* pcPath, int nFile, BootModule_t* psModule, ELFImage_t** ppsRes ) {
  int nError;
  ELFImage_t* psImage = NULL;
  ELF32ElfHeader_t sElfHeader;
  struct filp* psFile = NULL;
  int nPathLen = 0;

  if ( pcPath != NULL ) {
    nPathLen = strlen( pcPath ) + 1;
  }

again:
  LOCK( g_nImageListLock );

  if ( psModule == NULL ) {
    psFile = get_fd( 1, nFile );
    if ( psFile == NULL ) {
      UNLOCK( g_nImageListLock );
      return -EBADF;
    }

   /* if ( psFile->f_nType == FDT_DIR ) {
      UNLOCK( g_nImageListLock );
      return -EISDIR;
    }

    if ( ( psFile->f_nType == FDT_INDEX_DIR ) || ( psFile->f_nType == FDT_ATTR_DIR ) || ( psFile->f_nType == FDT_SYMLINK ) ) {
      UNLOCK( g_nImageListLock );
      return -EINVAL;
    }*/

    psImage = find_image( psFile );
    if ( psImage != NULL ) {
      if ( atomic_read( &psImage->ei_nOpenCount ) == -1 ) {
        UNLOCK( g_nImageListLock );
        delay_thread( 1000 );
        /* TODO: use putd_fd here as well to prevent memory leaks? */

        goto again;
      } else {
        put_fd( psFile );
        atomic_inc( &psImage->ei_nOpenCount );
        *ppsRes = psImage;

        UNLOCK( g_nImageListLock );

        return 0;
      }
    }
  }

  psImage = ( ELFImage_t* )kmalloc( sizeof( ELFImage_t ) + nPathLen, MEM_CLEAR | MEM_OKTOFAILHACK );
  if ( psImage == NULL ) {
    if ( psFile != NULL ) {
      put_fd( psFile );
    }

    UNLOCK( g_nImageListLock );

    kprintf( "load_image(): Out of memory\n" );

    return -ENOMEM;
  }

  atomic_set( &psImage->ei_nOpenCount, -1 );
  psImage->ei_psFile = psFile;
  psImage->ei_psModule = psModule;

  psImage->ei_psNext = g_psFirstImage;
  g_psFirstImage = psImage;

  UNLOCK( g_nImageListLock );

  psImage->ei_pcPath = ( char* )( psImage + 1 );
  memcpy( psImage->ei_pcPath, pcPath, nPathLen );

  strncpy( psImage->ei_acName, pcImageName, OS_NAME_LENGTH - 1 );
  psImage->ei_acName[ OS_NAME_LENGTH - 1 ] = 0;

  nError = read_image_data( psImage, &sElfHeader, 0, sizeof( ELF32ElfHeader_t ) );
  if ( nError != sizeof( ELF32ElfHeader_t ) ) {
    kprintf( "load_image(): Failed to load ELF header\n" );

    if ( nError >= 0 ) {
      nError = -ENOEXEC;
    }

    goto error;
  }

  if ( sElfHeader.eeh_nSecHdrSize != sizeof( ELF32SectionHeader_t ) ) {
    kprintf( "load_image(): Invalid section size %d, expected %d\n", sElfHeader.eeh_nSecHdrSize, sizeof( ELF32SectionHeader_t ) );
    nError = -EINVAL;
    goto error;
  }

  psImage->ei_nSectionCount = sElfHeader.eeh_nSecHdrCount;
  psImage->ei_nEntry = sElfHeader.eeh_nEntry;

  psImage->ei_psSections = ( ELF32SectionHeader_t* )kmalloc( sizeof( ELF32SectionHeader_t ) * psImage->ei_nSectionCount, MEM_OKTOFAILHACK );
  if ( psImage->ei_psSections == NULL ) {
    kprintf( "load_image(): Out of memory\n" );
    nError = -ENOMEM;
    goto error;
  }

  nError = read_image_data( psImage, psImage->ei_psSections, sElfHeader.eeh_nSecHdrOff, sElfHeader.eeh_nSecHdrCount * sizeof( ELF32SectionHeader_t ) );
  if ( nError != sElfHeader.eeh_nSecHdrCount * sizeof( ELF32SectionHeader_t ) ) {
    kprintf( "load_image(): Failed to load section header's\n" );

    if ( nError >= 0 ) {
      nError = -ENOEXEC;
    }

    goto error;
  }

  nError = parse_section_headers( psImage );
  if ( nError < 0 ) {
    goto error;
  }

  atomic_inc( &si_nLoadedImageCount );
  atomic_set( &psImage->ei_nOpenCount, 1 );

  *ppsRes = psImage;

  return 0;

error:
  unload_image( psImage );

  return nError;
}

static int memmap_instance( ELFImageInstance_t* psInstance, int nMode ) {
  ELFImage_t* psImage = psInstance->eii_psImage;
  int nError = 0;
  u32_t nTextStart = -1;
  u32_t nTextEnd = -1;
  u32_t nTextOffset = 0;
  u32_t nFileTextSize;
  u32_t nMemTextSize;
  u32_t nDataStart = -1;
  u32_t nDataEnd = -1;
  u32_t nBSSEnd = -1;
  u32_t nDataOffset = 0;
  u32_t nFileDataSize;
  u32_t nMemDataSize;
  int nAllocMode;
  int bData = 0;

  int nLocking = AREA_NO_LOCK;
  u32_t nAllocAddress;
  int nAreaCount;
  u32_t anAreaSizes[ 3 ];
  u32_t anAreaOffsets[ 3 ];
  area_id anAreas[ 3 ];
  char acTextName[ 64 ];
  char acBSSName[ 64 ];
  const char* apcAreaNames[] = { acTextName, acBSSName };
  AreaInfo_t sAreaInfo;
  int i;

  /*if ( strcmp( psImage->ei_acName, "libkernel.so" ) == 0 ) {
    kprintf( "Eeek sombody tried to memmap the kernel image!!!\n" );
    return ( 0 );
  }*/

  strcpy( acTextName, "ro_" );
  strncpy( acTextName + 3, psImage->ei_acName, 64 - 3 );
  acTextName[ 63 ] = 0;

  strcpy( acBSSName, "rw_" );
  strncpy( acBSSName + 3, psImage->ei_acName, 64 - 3 );
  acBSSName[ 63 ] = 0;

  for ( i = 0; i < psImage->ei_nSectionCount; i++ ) {
    ELF32SectionHeader_t* psSection = &psImage->ei_psSections[ i ];

    if ( psSection->esh_nType == SHT_NULL ) {
      continue;
    }

    if ( psSection->esh_nFlags & SHF_ALLOC ) {
      if ( ( ( psSection->esh_nFlags & SHF_WRITE ) == 0 ) && ( bData == 0 ) ) {
        if ( nTextStart == -1 ) {
          nTextStart = psSection->esh_nAddress;
          nTextOffset = psSection->esh_nOffset;
        }

        nTextEnd = psSection->esh_nAddress + psSection->esh_nSize - 1;
      } else {
        if ( nDataStart == -1 ) {
          nDataStart = psSection->esh_nAddress;
          nDataOffset = psSection->esh_nOffset;
          bData = 1;
        }

        if ( psSection->esh_nType == SHT_NOBITS ) {
          nBSSEnd = psSection->esh_nAddress + psSection->esh_nSize - 1;
          break;
        } else {
          nDataEnd = psSection->esh_nAddress + psSection->esh_nSize - 1;
          nBSSEnd = psSection->esh_nAddress + psSection->esh_nSize - 1;
        }
      }
    }
  }

  nTextOffset -= nTextStart & ~PAGE_MASK;
  nTextStart &= PAGE_MASK;
  nFileTextSize = nTextEnd - nTextStart + 1;
  nMemTextSize = ( nTextEnd - nTextStart + 1 + PAGE_SIZE - 1 ) & PAGE_MASK;

  nDataOffset -= nDataStart & ~PAGE_MASK;
  nDataStart &= PAGE_MASK;
  nFileDataSize = nDataEnd - nDataStart + 1;
  nMemDataSize = ( nBSSEnd - nDataStart + 1 + PAGE_SIZE - 1 ) & PAGE_MASK;

  if ( nTextStart + nMemTextSize > nDataStart ) {
    kprintf( "memmap_instance(): RO overlap RW (%08x + %08x -> %08x)\n", nTextStart, nMemTextSize, nDataStart );
    return -EINVAL;
  }

  if ( nMode == IM_APP_SPACE ) {
    nAllocMode = AREA_EXACT_ADDRESS | AREA_FULL_ACCESS;
    nAllocAddress = nTextStart;
  } else if ( nMode == IM_KERNEL_SPACE ) {
    nAllocMode = AREA_ANY_ADDRESS | AREA_KERNEL | AREA_FULL_ACCESS;
    nLocking = AREA_FULL_LOCK;
    nAllocAddress = 0;
  } else {
    nAllocMode = AREA_BASE_ADDRESS | AREA_FULL_ACCESS;
    nAllocAddress = AREA_FIRST_USER_ADDRESS + 512 * 1024 * 1024;
  }

  anAreaOffsets[ 0 ] = 0;
  anAreaOffsets[ 1 ] = nDataStart - nTextStart;
  anAreaSizes[ 0 ] = nMemTextSize;
  anAreaSizes[ 1 ] = nMemDataSize;

  anAreas[ 0 ] = -1;
  anAreas[ 1 ] = -1;

  nAreaCount = ( nMemDataSize > 0 ) ? 2 : 1;

  nError = alloc_area_list( nAllocMode, nLocking, nAllocAddress, nAreaCount, apcAreaNames, anAreaOffsets, anAreaSizes, anAreas );
  if ( nError < 0 ) {
    kprintf( "memmap_instance(): Failed to alloc areas\n" );
    return nError;
  }

  psInstance->eii_nTextArea = anAreas[ 0 ];
  psInstance->eii_nDataArea = anAreas[ 1 ];

  get_area_info( psInstance->eii_nTextArea, &sAreaInfo );
  psInstance->eii_nTextAddress = ( u32_t )sAreaInfo.ai_pAddress;

  if ( nMode == IM_KERNEL_SPACE ) {
    nError = read_image_data( psImage, sAreaInfo.ai_pAddress, nTextOffset, nFileTextSize );
    if ( nError != nFileTextSize ) {
      kprintf( "memmap_instance(): Failed to load text segment\n" );

      if ( nError >= 0 ) {
        nError = -ENOEXEC;
      }

      goto error;
    }

    if ( nFileDataSize > 0 ) {
      get_area_info( psInstance->eii_nDataArea, &sAreaInfo );
      nError = read_image_data( psImage, sAreaInfo.ai_pAddress, nDataOffset, nFileDataSize );
      if ( nError != nFileDataSize ) {
        kprintf( "memmap_instance(): Failed to load data segment\n" );

        if ( nError >= 0 ) {
          nError = -ENOEXEC;
        }

        goto error;
      }
    }
  } else {
    nError = map_area_to_file( psInstance->eii_nTextArea, psImage->ei_psFile, AREA_FULL_ACCESS, nTextOffset, nFileTextSize );
    if ( nError < 0 ) {
      kprintf( "memmap_instance(): Failed to memmap text segment\n" );
      goto error;
    }

    if ( nFileDataSize > 0 ) {
      nError = map_area_to_file( psInstance->eii_nDataArea, psImage->ei_psFile, AREA_FULL_ACCESS, nDataOffset, nFileDataSize );
      if ( nError < 0 ) {
        kprintf( "memmap_instance(): Failed to memmap text segment\n" );
        goto error;
      }
    }
  }

error:
  return nError;
}

static inline char* get_sym_address( ELFImageInstance_t* psInstance, ELFSymbol_t* psSym ) {
  ELFImage_t* psImage = psInstance->eii_psImage;

  if ( psInstance == NULL ) {
    kprintf( "get_sym_address() psSymInst == NULL\n" );
    return NULL;
  }

  if ( psInstance->eii_nTextArea == -1 ) {
    return ( char* )psSym->es_nAddress;
  }

  return ( char* )( psInstance->eii_nTextAddress + psSym->es_nAddress - psImage->ei_nVirtualAddress );
}

static int do_reloc_image( ImageContext_t* psContext, ELFImageInstance_t* psInstance, int nMode, ELFImageInstance_t* psParent, ELF32Reloc_t* psRelTab, int nRelCount ) {
  ELFImage_t* psImage = psInstance->eii_psImage;
  int nError;
  int i;
  int bNeedReloc = ( psInstance->eii_nTextAddress != psImage->ei_nVirtualAddress );


  for ( i = 0; i < nRelCount; i++ ) {
    ELF32Reloc_t* psReloc = &psRelTab[ i ];
    ELFSymbol_t* psSym = &psImage->ei_psSymbols[ ELF32_R_SYM( psReloc->er_nInfo ) ];
    int nRelocType = ELF32_R_TYPE( psReloc->er_nInfo );
    u32_t* pnTarget;

    if ( ( bNeedReloc == 0 ) && ( ( psSym->es_nSection != SHN_UNDEF ) || ( nRelocType == R_386_RELATIVE ) ) ) {
      continue;
    }

    pnTarget = ( void* )( psInstance->eii_nTextAddress + psReloc->er_nOffset - psImage->ei_nVirtualAddress );

    if ( ( nMode != IM_KERNEL_SPACE ) && ( nRelocType != R_386_NONE ) ) {
     // nError = arch_elf_check_reloc( current_proc()->p_psMemoryContext, pnTarget, sizeof( *pnTarget ) );
     // if ( nError < 0 ) {
      //  return nError;
      //}
    }

    nError = 0;

    switch ( nRelocType ) {
      case R_386_NONE :
        break;

      case R_386_32 : {
        ELFImageInstance_t* psDllInst;
        ELFSymbol_t* psDllSym;

        nError = find_symbol( psContext, psSym->es_pcName, &psDllInst, &psDllSym );
        if ( nError < 0 ) {
          kprintf( "Could not find symbol %s\n", psSym->es_pcName );
          break;
        }

        *pnTarget = *pnTarget + ( ( u32_t )get_sym_address( psDllInst, psDllSym ) );

        break;
      }

      case R_386_PC32 : {
        ELFImageInstance_t* psDllInst;
        ELFSymbol_t* psDllSym;

        nError = find_symbol( psContext, psSym->es_pcName, &psDllInst, &psDllSym );
        if ( nError < 0 ) {
          kprintf( "Could not find symbol %s\n", psSym->es_pcName );
          break;
        }

        *pnTarget = *pnTarget + ( ( u32_t )get_sym_address( psDllInst, psDllSym ) ) - ( ( u32_t )pnTarget );
        break;
      }

      case R_386_GOT32 :
        kprintf( "R_386_GOT32\n" );
        break;

      case R_386_PLT32 :
        kprintf( "R_386_PLT32\n" );
        break;

      case R_386_COPY : {
        ELFImageInstance_t* psDllInst;
        ELFSymbol_t* psDllSym;

        nError = find_symbol( psContext, psSym->es_pcName, &psDllInst, &psDllSym );
        if ( nError < 0 ) {
          kprintf( "Could not find symbol %s\n", psSym->es_pcName );
          break;
        }

        if ( psDllSym != NULL ) {
          kprintf( "R_386_COPY %d %d %08x - %08x\n", psDllSym->es_nSize, psSym->es_nSize, psDllSym->es_nAddress, psSym->es_nAddress );
          kprintf( "Copy %d bytes from %p to %p\n", psDllSym->es_nSize, get_sym_address( psDllInst, psDllSym ), pnTarget );
          memcpy( pnTarget, get_sym_address( psDllInst, psDllSym ), psDllSym->es_nSize );
        }

        break;
      }

      case R_386_GLOB_DATA : {
        ELFImageInstance_t* psDllInst;
        ELFSymbol_t* psDllSym;

        nError = find_symbol( psContext, psSym->es_pcName, &psDllInst, &psDllSym );
        if ( nError < 0 ) {
          if ( ELF32_ST_BIND( psSym->es_nInfo ) == STB_WEAK ) {
            *pnTarget = 0;
            nError = 0;
          } else {
            kprintf( "Could not find symbol %s\n", psSym->es_pcName );
          }

          break;
        }

        *pnTarget = ( u32_t )get_sym_address( psDllInst, psDllSym );

        break;
      }

      case R_386_JMP_SLOT : {
        ELFImageInstance_t* psDllInst;
        ELFSymbol_t* psDllSym;

        nError = find_symbol( psContext, psSym->es_pcName, &psDllInst, &psDllSym );
        if ( nError < 0 ) {
          if ( ELF32_ST_BIND( psSym->es_nInfo ) == STB_WEAK ) {
            *pnTarget = 0;
            nError = 0;
          } else {
            kprintf( "Could not find symbol %s\n", psSym->es_pcName );
          }

          break;
        }

        *pnTarget = ( u32_t )get_sym_address( psDllInst, psDllSym );

        break;
      }

      case R_386_RELATIVE :
        if ( *pnTarget != 0 ) {
          *pnTarget = *pnTarget + psInstance->eii_nTextAddress - psImage->ei_nVirtualAddress;
        }

        break;

      case R_386_GOTOFF :
        kprintf( "R_386_GOTOFF\n" );
        break;

      case R_386_GOTPC :
        kprintf( "R_386_GOTPC\n" );
        break;

      default :
        kprintf( "Error: Unknown relocation type %d\n", ELF32_R_TYPE( psReloc->er_nInfo ) );
        break;
    }

    if ( nError < 0 ) {
      return nError;
    }
  }

  return 0;
}

static int reloc_image( ImageContext_t* psContext, ELFImageInstance_t* psInstance, int nMode, ELFImageInstance_t* psParent ) {
  int i;
  int nError = 0;
  ELFImage_t* psImage = psInstance->eii_psImage;

  if ( psInstance == NULL ) {
    kprintf( "reloc_image() psInst == NULL!\n" );
    return -EINVAL;
  }

  if ( psImage == NULL ) {
    kprintf( "reloc_image() psImage == NULL!\n" );
    return -EINVAL;
  }

  if ( psInstance->eii_bRelocated ) {
    return 0;
  }

  psInstance->eii_bRelocated = 1;

  for ( i = 0; i < psImage->ei_nSubImageCount; i++ ) {
    nError = reloc_image( psContext, psInstance->eii_ppsSubImages[ i ], nMode, psInstance );
    if ( nError < 0 ) {
      goto error;
    }
  }

  if ( psImage->ei_nRelocCount > 0 ) {
    nError = do_reloc_image( psContext, psInstance, nMode, psParent, psImage->ei_psRelocs, psImage->ei_nRelocCount );
  }

error:
  return nError;
}

static void close_image_instance( ImageContext_t* psContext, ELFImageInstance_t* psInstance ) {
  ELFImage_t* psImage = psInstance->eii_psImage;

  if ( psInstance->eii_ppsSubImages != NULL ) {
    int i;

    for ( i = 0; i < psImage->ei_nSubImageCount; i++ ) {
      close_image_instance( psContext, psInstance->eii_ppsSubImages[ i ] );
    }
  }

  if ( atomic_dec_and_test( &psInstance->eii_nOpenCount ) ) {
    if ( psInstance->eii_ppsSubImages != NULL ) {
      kfree( psInstance->eii_ppsSubImages );
      psInstance->eii_ppsSubImages = NULL;
    }

    psContext->ic_psInstances[ psInstance->eii_nHandle ] = NULL;

    if ( psInstance->eii_nTextArea != -1 ) {
      delete_area( psInstance->eii_nTextArea );
      psInstance->eii_nTextArea = -1;
    }

    if ( psInstance->eii_nDataArea != -1 ) {
      delete_area( psInstance->eii_nDataArea );
      psInstance->eii_nDataArea = -1;
    }

    kfree( psInstance );
    atomic_dec( &si_nImageInstanceCount );

    unload_image( psImage );
  }
}

static int open_library_file( const char* pcName, const char* pcSearchPath, char* pcFullPath ) {
  int nFile;
  char acPath[ 512 ];
  char acSysLibPath[ 256 ];

  nFile = server_open( pcName, "r" );
  if ( nFile >= 0 ) {
    return nFile;
  }

  if ( pcSearchPath == NULL ) {
    snprintf( acSysLibPath, 256, "/giszOS/libraries/" );
    pcSearchPath = acSysLibPath;
  }

  if ( pcSearchPath != NULL ) {
    int nNameLen = strlen( pcName );
    const char* pcStart = pcSearchPath;

    for ( ;; ) {
      int nLen;
      const char* pcSep = strchr( pcStart, ':' );

      if ( pcSep == NULL ) {
        nLen = strlen( pcStart );
      } else {
        nLen = pcSep - pcStart;
      }

      if ( nLen != 0 ) {
        int nTotLen = nLen + nNameLen + 1;
        Inode_t* psParent = NULL;

        if ( pcStart[ nLen - 1 ] != '/' ) {
          nTotLen++;
        }

        if ( ( nLen > 8 ) && ( strncmp( pcStart, "@bindir@/", 9 ) == 0 ) ) {
          nLen -= 9;
          pcStart += 9;

         // psParent = current_proc()->p_psIoContext->ic_psBinDir;
        }

        if ( nTotLen > sizeof( acPath ) ) {
          pcStart += nLen + 1;
          continue;
        }

        memcpy( acPath, pcStart, nLen );
        pcStart += nLen + 1;

        if ( acPath[ nLen - 1 ] != '/' ) {
          acPath[ nLen++ ] = '/';
        }

        strcpy( acPath + nLen, pcName );

        nFile = extended_open( 1, psParent, acPath, NULL, O_RDONLY, 0 );
        if ( nFile >= 0 ) {
          return nFile;
        }
      } else {
        pcStart++;
      }

      if ( pcSep == NULL ) {
        break;
      }
    }
  }

  kprintf( "open_library_file(): Failed to find %s in search path\n", pcName );

  return -1;
}

static ELFImageInstance_t* find_instance( ImageContext_t* psContext, int nFile ) {
#if 0
  int i;
  struct filp* psFile = get_fd( 1, nFile );

  if ( psFile == NULL ) {
    kprintf( "find_inst() psFile == NULL\n" );
    return NULL;
  }

  for ( i = 0; i < MAX_IMAGE_COUNT; i++ ) {
    if ( ( psContext->ic_psInstances[ i ] != NULL ) && ( psContext->ic_psInstances[ i ]->eii_psImage->ei_psFile != NULL ) ) {
      if ( CMP_INODES( psContext->ic_psInstances[ i ]->eii_psImage->ei_psFile->f_psInode, psFile->f_psInode ) ) {
        put_fd( psFile );

        return psContext->ic_psInstances[ i ];
      }
    }
  }

  put_fd( psFile );
#endif
  return NULL;
}

static void increase_instance_reference_count( ELFImageInstance_t* psInstance ) {
  int i;

  atomic_inc( &psInstance->eii_nOpenCount );

  for ( i = 0; i < psInstance->eii_psImage->ei_nSubImageCount; i++ ) {
    increase_instance_reference_count( psInstance->eii_ppsSubImages[ i ] );
  }
}

int load_image_instance( ImageContext_t* psContext, const char* pcPath, BootModule_t* psModule, int nMode, ELFImageInstance_t** ppsInstance, const char* pcLibraryPath ) {
  ELFImage_t* psImage = NULL;
  ELFImageInstance_t* psInstance = NULL;
  const char* pcImageName = strrchr( pcPath, '/' );
  int nFile = -1;
  Inode_t* psParentInode = NULL;
  int nError;
  int i;

  /*if ( nMode == IM_KERNEL_SPACE && strcmp( pzPath, "libkernel.so" ) == 0 )
  {
    atomic_inc( &g_psKernelCtx->ic_psInstances[0]->ii_nOpenCount );
    *ppsInst = g_psKernelCtx->ic_psInstances[0];
    return ( 0 );
  }*/

  if ( psModule == NULL ) {
    if ( nMode == IM_APP_SPACE ) {
      nFile = extended_open( 1, NULL, pcPath, &psParentInode, O_RDONLY, 0 );
    } else if ( nMode == IM_KERNEL_SPACE ) {
      nFile = open( pcPath, O_RDONLY );
    } else {
      nFile = open_library_file( pcPath, pcLibraryPath, NULL );
      if ( nFile < 0 ) {
        kprintf( "Error: Failed to open library %s\n", pcPath );
      }
    }

    if ( nFile < 0 ) {
      nError = nFile;
      goto error1;
    }

    psInstance = find_instance( psContext, nFile );
  }

  if ( psInstance != NULL ) {
    increase_instance_reference_count( psInstance );
    *ppsInstance = psInstance;
    close( nFile );

    if ( psParentInode != NULL ) {
      put_inode( psParentInode );
    }

    return 0;
  }

  if ( pcImageName != NULL ) {
    pcImageName++;
  } else {
    pcImageName = pcPath;
  }

  kassert( ( psModule != NULL ) || ( nFile >= 0 ) );

  nError = load_image( pcImageName, pcPath, nFile, psModule, &psImage );

  if ( psModule == NULL ) {
    close( nFile );
  }

  if ( nError < 0 ) {
    goto error1;
  }

  psInstance = ( ELFImageInstance_t* )kmalloc( sizeof( ELFImageInstance_t ), MEM_CLEAR | MEM_OKTOFAILHACK );
  if ( psInstance == NULL ) {
    nError = -ENOMEM;
    goto error2;
  }

  psInstance->eii_nTextArea = -1;
  psInstance->eii_nDataArea = -1;
  atomic_set( &psInstance->eii_nOpenCount, 1 );

  if ( psImage->ei_nSubImageCount == 0 ) {
    psInstance->eii_ppsSubImages = NULL;
  } else {
    psInstance->eii_ppsSubImages = ( ELFImageInstance_t** )kmalloc( psImage->ei_nSubImageCount * sizeof( ELFImageInstance_t* ), MEM_CLEAR | MEM_OKTOFAILHACK );
    if ( psInstance->eii_ppsSubImages == NULL ) {
      nError = -ENOMEM;
      goto error3;
    }
  }

  psInstance->eii_psImage = psImage;

  nError = memmap_instance( psInstance, nMode );
  if ( nError < 0 ) {
    goto error4;
  }

  psInstance->eii_nHandle = -1;

  for ( i = 0; i < MAX_IMAGE_COUNT; i++ ) {
    if ( psContext->ic_psInstances[ i ] == NULL ) {
      psContext->ic_psInstances[ i ] = psInstance;
      psInstance->eii_nHandle = i;

      break;
    }
  }

  if ( psInstance->eii_nHandle < 0 ) {
    kprintf( "load_image_inst(): Too many open images\n" );
    nError = -EMFILE;
    goto error5;
  }

  if ( nMode == IM_APP_SPACE ) {
    proc_t* psProcess = current_proc();

   // if ( psProcess->p_psIoContext->ic_psBinDir != NULL ) {
   //   put_inode( psProcess->p_psIoContext->ic_psBinDir );
   // }

   // psProcess->p_psIoContext->ic_psBinDir = psParentInode;
    psParentInode = NULL;
  }

  for ( i = 0; i < psImage->ei_nSubImageCount; i++ ) {
    if ( nMode == IM_KERNEL_SPACE ) {
      nError = load_image_instance( psContext, psImage->ei_ppcSubImages[ i ], NULL, IM_KERNEL_SPACE, &psInstance->eii_ppsSubImages[ i ], pcLibraryPath );
    } else {
      nError = load_image_instance( psContext, psImage->ei_ppcSubImages[ i ], NULL, IM_LIBRARY_SPACE, &psInstance->eii_ppsSubImages[ i ], pcLibraryPath );
    }

    if ( nError < 0 ) {
      int j;

      for ( j = 0; j < i; j++ ) {
        close_image_instance( psContext, psInstance->eii_ppsSubImages[ j ] );
      }

      goto error6;
    }
  }

  if ( nMode != IM_LIBRARY_SPACE ) {
    nError = reloc_image( psContext, psInstance, nMode, ( nMode == IM_ADDON_SPACE ) ? psContext->ic_psInstances[ 0 ] : NULL );
    if ( nError < 0 ) {
      goto error7;
    }
  }

  *ppsInstance = psInstance;

  atomic_inc( &si_nImageInstanceCount );

  return 0;

error7:
  for ( i = 0; i < psImage->ei_nSubImageCount; i++ ) {
    close_image_instance( psContext, psInstance->eii_ppsSubImages[ i ] );
  }

error6:
  psContext->ic_psInstances[ psInstance->eii_nHandle ] = NULL;

error5:
  if ( psInstance->eii_nTextArea != -1 ) {
    delete_area( psInstance->eii_nTextArea );
    psInstance->eii_nTextArea = -1;
  }

  if ( psInstance->eii_nDataArea != -1 ) {
    delete_area( psInstance->eii_nDataArea );
    psInstance->eii_nDataArea = -1;
  }

error4:
  if ( psInstance->eii_ppsSubImages != NULL ) {
    kfree( psInstance->eii_ppsSubImages );
    psInstance->eii_ppsSubImages = NULL;
  }

error3:
  kfree( psInstance );

error2:
  unload_image( psImage );

error1:
  if ( psParentInode != NULL ) {
    put_inode( psParentInode );
  }

  return nError;
}

static int do_find_module_by_address( ImageContext_t* psCtx, u32_t nAddress ) {
  ELFImageInstance_t* psInst = NULL;
  AreaInfo_t sAreaInfo;
  int i;
  int nModule = -ENOENT;

  LOCK( psCtx->ic_nLock );
  for ( i = 0; i < MAX_IMAGE_COUNT; ++i )
  {
    psInst = psCtx->ic_psInstances[i];
    if ( psInst == NULL )
    {
      continue;
    }
    if ( psInst->eii_nTextArea != -1 && get_area_info( psInst->eii_nTextArea, &sAreaInfo ) >= 0 && nAddress >= ( ( u32_t )sAreaInfo.ai_pAddress ) && nAddress < ( ( u32_t )sAreaInfo.ai_pAddress ) + sAreaInfo.ai_nSize )
    {
      break;
    }
    if ( psInst->eii_nDataArea != -1 && get_area_info( psInst->eii_nDataArea, &sAreaInfo ) >= 0 && nAddress >= ( ( u32_t )sAreaInfo.ai_pAddress ) && nAddress < ( ( u32_t )sAreaInfo.ai_pAddress ) + sAreaInfo.ai_nSize )
    {
      break;
    }
    psInst = NULL;
  }
  if ( psInst != NULL )
  {
    nModule = i;
  }
  UNLOCK( psCtx->ic_nLock );
  return ( nModule );
}

int find_module_by_address( const void *pAddress ) {
  ImageContext_t* psCtx;

  if ( ( ( u32_t )pAddress ) < AREA_FIRST_USER_ADDRESS )
  {
    psCtx = g_psKernelImageContext;
  }
  else
  {
    psCtx = CURRENT_PROC_IMAGE_CONTEXT;
  }

  if ( psCtx == NULL )
  {
    return ( -EINVAL );
  }

  return ( do_find_module_by_address( psCtx, ( u32_t )pAddress ) );
}

int get_symbol_by_address( int nLibrary, const char *pAddress, char *pzName, int nMaxNamLen, void **ppAddress )
{
  ImageContext_t* psCtx;

  if ( ( ( u32_t )pAddress ) < AREA_FIRST_USER_ADDRESS )
  {
    psCtx = g_psKernelImageContext;
  }
  else
  {
    psCtx = CURRENT_PROC_IMAGE_CONTEXT;
  }
  
  ELFImageInstance_t* psInst;
  ELFImage_t* psImage;
  ELFSymbol_t* psClosestSymbol = NULL;
  u32_t nClosest = 0xffffffff;
  int i;

  if ( nLibrary < 0 || nLibrary >= MAX_IMAGE_COUNT )
  {
    return ( -EINVAL );
  }
  LOCK( psCtx->ic_nLock );
  psInst = psCtx->ic_psInstances[ nLibrary ];

  if ( psInst == NULL )
  {
    UNLOCK( psCtx->ic_nLock );
    return ( -EINVAL ); // FIXME: Should may return someting like EBADF instead?
  }

  psImage = psInst->eii_psImage;

  for ( i = psImage->ei_nSymCount - 1; i >= 0; --i )
  {
    ELFSymbol_t* psSymbol = &psImage->ei_psSymbols[i];
    char *pSymAddr = get_sym_address( psInst, psSymbol );

    if ( pSymAddr > pAddress )
    {
      continue;
    }
    if ( pSymAddr == pAddress )
    {
      psClosestSymbol = psSymbol;
      *ppAddress = pSymAddr;
      break;
    }
    if ( pAddress - pSymAddr < nClosest )
    {
      nClosest = pAddress - pSymAddr;
      psClosestSymbol = psSymbol;
      *ppAddress = pSymAddr;
    }
  }
  UNLOCK( psCtx->ic_nLock );

  if ( psClosestSymbol == NULL )
  {
    return ( -ENOSYM );
  }
  strncpy( pzName, psClosestSymbol->es_pcName, nMaxNamLen );
  pzName[nMaxNamLen - 1] = '\0';
  return ( strlen( pzName ) );
}

int do_get_symbol_address( ImageContext_t* psContext, int nLibrary, const char* pcName, int nIndex, void** ppPtr ) {
  ELFImageInstance_t* psInstance;
  ELFSymbol_t* psSymbol;
  int nError = 0;

  if ( ( nLibrary < 0 ) || ( nLibrary >= MAX_IMAGE_COUNT ) ) {
    return -EINVAL;
  }

  LOCK( psContext->ic_nLock );

  psInstance = psContext->ic_psInstances[ nLibrary ];
  if ( psInstance == NULL ) {
    nError = -EINVAL;
    goto error;
  }

  if ( nIndex < 0 ) {
    psSymbol = lookup_symbol( psInstance->eii_psImage, pcName );
  } else {
    if ( nIndex < psInstance->eii_psImage->ei_nSymCount ) {
      psSymbol = &psInstance->eii_psImage->ei_psSymbols[ nIndex ];
    } else {
      psSymbol = NULL;
    }
  }

  if ( psSymbol == NULL ) {
    nError = -ENOSYM;
    goto error;
  }

  *ppPtr = get_sym_address( psInstance, psSymbol );

error:
  UNLOCK( psContext->ic_nLock );

  return nError;
}

int get_symbol_address( int nLibrary, const char* pcName, int nIndex, void** ppPtr ) {
  return do_get_symbol_address( g_psKernelImageContext, nLibrary, pcName, nIndex, ppPtr );
}

ImageContext_t* create_image_context( void ) {
  ImageContext_t* psContext;

  psContext = ( ImageContext_t* )kmalloc( sizeof( ImageContext_t ), MEM_CLEAR | MEM_OKTOFAILHACK );
  if ( psContext == NULL ) {
    kprintf( "create_image_context(): Out of memory\n" );
    return NULL;
  }

  psContext->ic_nLock = create_semaphore( "image_ctx_lock", 1, SEM_RECURSIVE );
  if ( psContext->ic_nLock < 0 ) {
    kfree( psContext );
    return NULL;
  }

  return psContext;
}


void delete_image_context( ImageContext_t* psContext ) {
  delete_semaphore( psContext->ic_nLock );
  kfree( psContext );
}

void close_all_images( ImageContext_t* psContext ) {
  int i;

  LOCK( psContext->ic_nLock );

  for ( i = 0; i < MAX_IMAGE_COUNT; i++ ) {
    if ( psContext->ic_psInstances[ i ] != NULL ) {
      unload_image( psContext->ic_psInstances[ i ]->eii_psImage );

      if ( psContext->ic_psInstances[ i ]->eii_ppsSubImages != NULL ) {
        kfree( psContext->ic_psInstances[ i ]->eii_ppsSubImages );
        psContext->ic_psInstances[ i ]->eii_ppsSubImages = NULL;
      }

      kfree( psContext->ic_psInstances[ i ] );
      psContext->ic_psInstances[ i ] = NULL;

      atomic_dec( &si_nImageInstanceCount );
    }
  }

  UNLOCK( psContext->ic_nLock );
}

int sys_load_library( const char* a_pcPath, const char* pcSearchPath ) {
  ImageContext_t* psContext = CURRENT_PROC_IMAGE_CONTEXT;
  ELFImageInstance_t* psInstance;
  char* pcPath;
  int nError;

  nError = strndup_from_user( a_pcPath, PATH_MAX, &pcPath );
  if ( nError < 0 ) {
    return nError;
  }

  LOCK( psContext->ic_nLock );

  nError = load_image_instance( psContext, pcPath, NULL, IM_ADDON_SPACE, &psInstance, pcSearchPath );

  kfree( pcPath );

  if ( nError >= 0 ) {
    atomic_inc( &psInstance->eii_nAppOpenCount );
    nError = psInstance->eii_nHandle;
  }

  UNLOCK( psContext->ic_nLock );

  return nError;
}

int sys_unload_library( int nLibrary ) {
  ImageContext_t* psContext = CURRENT_PROC_IMAGE_CONTEXT;
  ELFImageInstance_t* psInstance;
  int nError = 0;

  if ( ( nLibrary < 0 ) || ( nLibrary >= MAX_IMAGE_COUNT ) ) {
    return -EINVAL;
  }

  LOCK( psContext->ic_nLock );

  psInstance = psContext->ic_psInstances[ nLibrary ];
  if ( psInstance == NULL ) {
    nError = -EINVAL;
    goto error;
  }

  if ( atomic_read( &psInstance->eii_nAppOpenCount ) == 0 ) {
    kprintf( "sys_unload_library() attempt to unload non load_library() loaded image %s\n", psInstance->eii_psImage->ei_acName );
    nError = -EINVAL;
    goto error;
  }

  atomic_dec( &psInstance->eii_nAppOpenCount );
  close_image_instance( psContext, psInstance );

error:
  UNLOCK( psContext->ic_nLock );

  return nError;
}

int sys_get_symbol_address( int nLibrary, const char* pcName, int nIndex, void** pPtr ) {
  return do_get_symbol_address( CURRENT_PROC_IMAGE_CONTEXT, nLibrary, pcName, nIndex, pPtr );
}

static inline void do_get_image_info( ImageInfo_t* psInfo, ELFImageInstance_t* psInst ) {
  ELFImage_t* psImage = psInst->eii_psImage;
  u32_t nPhysAddr = psInst->eii_nTextAddress;
  AreaInfo_t sTextInfo;
  AreaInfo_t sDataInfo;

  strcpy( psInfo->ii_name, psImage->ei_acName );

  if ( psImage->ei_nEntry != 0 )
  {
    psInfo->ii_entry_point = ( image_entry * )( nPhysAddr + psImage->ei_nEntry - psImage->ei_nVirtualAddress );
  }
  else
  {
    psInfo->ii_entry_point = NULL;
  }
  if ( psImage->ei_nInit != 0 )
  {
    psInfo->ii_init = ( image_init * )( nPhysAddr + psImage->ei_nInit - psImage->ei_nVirtualAddress );
  }
  else
  {
    psInfo->ii_init = NULL;
  }
  if ( psImage->ei_nFini != 0 )
  {
    psInfo->ii_fini = ( image_term * )( nPhysAddr + psImage->ei_nFini - psImage->ei_nVirtualAddress );
  }
  else
  {
    psInfo->ii_fini = NULL;
  }

  get_area_info( psInst->eii_nTextArea, &sTextInfo );
  get_area_info( psInst->eii_nDataArea, &sDataInfo );

  psInfo->ii_image_id = psInst->eii_nHandle;
  psInfo->ii_type = IM_APP_SPACE;
  psInfo->ii_open_count = atomic_read( &psInst->eii_nOpenCount );
  psInfo->ii_sub_image_count = psImage->ei_nSubImageCount;
  psInfo->ii_init_order = 0;

  if ( psImage->ei_psFile != NULL ) {
   // psInfo->ii_device = psImage->ei_psFile->f_psInode->i_psVolume->v_nDevNum;
    //psInfo->ii_inode = psImage->ei_psFile->f_psInode->i_nInode;
  } else {
   // psInfo->ii_device = 0;
    //psInfo->ii_inode = 0;
  }

  psInfo->ii_text_addr = sTextInfo.ai_pAddress;
  psInfo->ii_data_addr = sDataInfo.ai_pAddress;
  psInfo->ii_text_size = sTextInfo.ai_nSize;
  psInfo->ii_data_size = sDataInfo.ai_nSize;
  psInfo->ii_ctor_addr = ( void* )psImage->ei_nCtors;
  psInfo->ii_ctor_count = psImage->ei_nCtorCount;
}

int get_image_info( int bKernel, int nImage, int nSubImage, ImageInfo_t* psInfo ) {
  ImageContext_t* psCtx;
  ELFImageInstance_t* psInst;
  int nError = 0;

  if ( bKernel ) {
    psCtx = g_psKernelImageContext;
  } else {
    psCtx = CURRENT_PROC_IMAGE_CONTEXT;
  }

  LOCK( psCtx->ic_nLock );

  if ( nImage < 0 || nImage >= MAX_IMAGE_COUNT || psCtx->ic_psInstances[nImage] == NULL ) {
    nError = -EINVAL;
    goto error;
  }

  psInst = psCtx->ic_psInstances[nImage];

  if ( nSubImage >= 0 )
  {
    if ( nSubImage >= psInst->eii_psImage->ei_nSubImageCount )
    {
      nError = -EINVAL;
      goto error;
    }
    else
    {
      psInst = psInst->eii_ppsSubImages[nSubImage];
    }
  }

  do_get_image_info( psInfo, psInst );

error:
  UNLOCK( psCtx->ic_nLock );

  return nError;
}

int sys_get_image_info( int nImage, int nSubImage, ImageInfo_t* psInfo ) {
  if ( verify_memory_area( psInfo, sizeof( ImageInfo_t ), 1 ) < 0 ) {
    return -EFAULT;
  }

  return get_image_info( 0, nImage, nSubImage, psInfo );
}
#if 0


void clone_image_instance( ELFImageInstance_t* psDst, ELFImageInstance_t* psSrc, MemoryContext_t* psMemoryContext ) {
  memcpy( psDst, psSrc, sizeof( ELFImageInstance_t ) );

  psDst->eii_ppsSubImages = NULL;

  atomic_inc( &psDst->eii_psImage->ei_nOpenCount );

  if ( psSrc->eii_nTextArea != -1 ) {
    AreaInfo_t sInfo;
    MemoryArea_t* psArea;

    get_area_info( psSrc->eii_nTextArea, &sInfo );
    psArea = get_area( psMemoryContext, ( u32_t )sInfo.ai_pAddress );

    if ( psArea != NULL ) {
      psDst->eii_nTextArea = psArea->ma_nAreaID;
      put_area( psArea );
    }
  }

  if ( psSrc->eii_nDataArea != -1 ) {
    AreaInfo_t sInfo;
    MemoryArea_t* psArea;

    get_area_info( psSrc->eii_nDataArea, &sInfo );
    psArea = get_area( psMemoryContext, ( u32_t )sInfo.ai_pAddress );

    if ( psArea != NULL ) {
      psDst->eii_nDataArea = psArea->ma_nAreaID;
      put_area( psArea );
    }
  }
}

ImageContext_t* clone_image_context( ImageContext_t* psOrigContext, MemoryContext_t* psMemoryContext ) {
  int i;
  ImageContext_t* psNewContext;

  if ( psOrigContext == NULL ) {
    kprintf( "clone_image_context(): psOrig == NULL\n" );
    return NULL;
  }

  psNewContext = create_image_context();
  if ( psNewContext == NULL ) {
    return NULL;
  }

  LOCK( psOrigContext->ic_nLock );

  for ( i = 0; i < MAX_IMAGE_COUNT; i++ ) {
    if ( psOrigContext->ic_psInstances[ i ] != NULL ) {
      ELFImageInstance_t* psSrc = psOrigContext->ic_psInstances[ i ];
      ELFImageInstance_t* psDst;

      psDst = ( ELFImageInstance_t* )kmalloc( sizeof( ELFImageInstance_t ), MEM_OKTOFAILHACK );
      if ( psDst == NULL ) {
        int j;

        kprintf( "clone_image_context(): No memory for image instance\n" );

        for ( j = i - 1; j >= 0; j-- ) {
          if ( psNewContext->ic_psInstances[ j ] != NULL ) {
            kfree( psNewContext->ic_psInstances[ j ] );
            atomic_dec( &si_nImageInstanceCount );
          }
        }

        delete_image_context( psNewContext );
        psNewContext = NULL;

        break;
      }

      psDst->eii_nTextArea = -1;
      psDst->eii_nDataArea = -1;

      psNewContext->ic_psInstances[ i ] = psDst;

      clone_image_instance( psDst, psSrc, psMemoryContext );
      atomic_inc( &si_nImageInstanceCount );
    }
  }

  if ( psNewContext != NULL ) {
    for ( i = 0; i < MAX_IMAGE_COUNT; i++ ) {
      if ( psOrigContext->ic_psInstances[ i ] != NULL ) {
        int j;
        ELFImageInstance_t* psSrc = psOrigContext->ic_psInstances[ i ];
        ELFImageInstance_t* psDst = psNewContext->ic_psInstances[ i ];

        if ( psDst->eii_psImage->ei_nSubImageCount == 0 ) {
          psDst->eii_ppsSubImages = NULL;
        } else {
          psDst->eii_ppsSubImages = kmalloc( psDst->eii_psImage->ei_nSubImageCount * sizeof( ELFImageInstance_t* ), MEM_CLEAR | MEM_OKTOFAILHACK );
          if ( psDst->eii_ppsSubImages == NULL ) {
            for ( j = 0; j < MAX_IMAGE_COUNT; j++ ) {
              if ( psNewContext->ic_psInstances[ j ] != NULL ) {
                if ( psNewContext->ic_psInstances[ j ]->eii_ppsSubImages != NULL ) {
                  kfree( psNewContext->ic_psInstances[ j ]->eii_ppsSubImages );
                }

                atomic_dec( &psNewContext->ic_psInstances[ j ]->eii_psImage->ei_nOpenCount );
                kfree( psNewContext->ic_psInstances[ j ] );

                atomic_dec( &si_nImageInstanceCount );
              }
            }

            delete_image_context( psNewContext );
            psNewContext = NULL;

            break;
          }

          for ( j = 0; j < psDst->eii_psImage->ei_nSubImageCount; j++ ) {
            psDst->eii_ppsSubImages[ j ] = psNewContext->ic_psInstances[ psSrc->eii_ppsSubImages[ j ]->eii_nHandle ];
          }
        }
      }
    }
  }

  UNLOCK( psOrigContext->ic_nLock );

  return psNewContext;
}



int load_kernel_driver( const char* a_pcPath ) {
  ImageContext_t* psContext = g_psKernelImageContext;
  ELFImageInstance_t* psInstance;
  char* pcPath;
  int nError;
  int i;

  if ( g_bRootFsMounted ) {
    nError = normalize_path( a_pcPath, &pcPath );
    if ( nError < 0 ) {
      return nError;
    }
  } else {
    pcPath = ( char* )a_pcPath;
  }

  LOCK( psContext->ic_nLock );

  for ( i = 0; i < MAX_IMAGE_COUNT; i++ ) {
    if ( ( psContext->ic_psInstances[ i ] != NULL ) && ( strcmp( pcPath, psContext->ic_psInstances[ i ]->eii_psImage->ei_pcPath ) == 0 ) ) {
      atomic_inc( &psContext->ic_psInstances[ i ]->eii_nAppOpenCount );
      nError = i;
      goto done;
    }
  }

  nError = load_image_instance( psContext, pcPath, NULL, IM_KERNEL_SPACE, &psInstance, NULL );
  if ( nError >= 0 ) {
    atomic_inc( &psInstance->eii_nAppOpenCount );
    nError = psInstance->eii_nHandle;
  }

done:
  UNLOCK( psContext->ic_nLock );

  if ( pcPath != a_pcPath ) {
    kfree( pcPath );
  }

  return nError;
}

int unload_kernel_driver( int nLibrary ) {
  ImageContext_t* psContext = g_psKernelImageContext;
  ELFImageInstance_t* psInstance;
  int nError = 0;

  if ( ( nLibrary < 0 ) || ( nLibrary >= MAX_IMAGE_COUNT ) ) {
    return -EINVAL;
  }

  LOCK( psContext->ic_nLock );

  psInstance = psContext->ic_psInstances[ nLibrary ];
  if ( psInstance == NULL ) {
    kprintf( "unload_kernel_driver(): Invalid image handle %d\n", nLibrary );
    nError = -EINVAL;
    goto error;
  }

  if ( atomic_read( &psInstance->eii_nAppOpenCount ) == 0 ) {
    kprintf( "unload_kernel_driver(): Attempt to unload non load_kernel_driver() loaded image %s\n", psInstance->eii_psImage->ei_acName );
    nError = -EINVAL;
    goto error;
  }

  atomic_dec( &psInstance->eii_nAppOpenCount );
  close_image_instance( psContext, psInstance );

error:
  UNLOCK( psContext->ic_nLock );

  return nError;
}



static void create_kernel_image_context( void ) {
  int i;
  ELFImage_t* psImage;
  ELFImageInstance_t* psInstance;

  psImage = ( ELFImage_t* )kmalloc( sizeof( ELFImage_t ), MEM_CLEAR | MEM_OKTOFAILHACK );
  psInstance = ( ELFImageInstance_t* )kmalloc( sizeof( ELFImageInstance_t ), MEM_CLEAR | MEM_OKTOFAILHACK );

  if ( ( psImage == NULL ) || ( psInstance == NULL ) ) {
    panic( "create_kernel_image() no memory for image\n" );
    return;
  }

  psInstance->eii_nTextArea = -1;
  psInstance->eii_nDataArea = -1;

  psImage->ei_nSymCount = get_kernel_symbol_count();
  psImage->ei_psSymbols = ( ELFSymbol_t* )kmalloc( psImage->ei_nSymCount * sizeof( ELFSymbol_t ), MEM_CLEAR | MEM_OKTOFAILHACK );
  if ( psImage->ei_psSymbols == NULL ) {
    panic( "create_kernel_image() no memory for symbols\n" );
    return;
  }

  g_psKernelImageContext = create_image_context();
  if ( g_psKernelImageContext == NULL ) {
    panic( "create_kernel_image() no memory for kernel image context\n" );
    return;
  }

  for ( i = 0; i < psImage->ei_nSymCount; i++ ) {
    void* pValue;
    const char* pcName;
    u32_t nHash;

    pcName = get_kernel_symbol( i, &pValue );

    psImage->ei_psSymbols[ i ].es_pcName = pcName;
    psImage->ei_psSymbols[ i ].es_nInfo = ELF32_ST_INFO( STB_GLOBAL, STT_FUNC );
    psImage->ei_psSymbols[ i ].es_nImage = 0;
    psImage->ei_psSymbols[ i ].es_nAddress = ( u32_t )pValue;
    psImage->ei_psSymbols[ i ].es_nSize = 0;
    psImage->ei_psSymbols[ i ].es_nSection = 1;

    nHash = elf_sym_hash( pcName ) % ELF_KERNEL_SYM_HASHTAB_SIZE;
    psImage->ei_psSymbols[ i ].es_psHashNext = psImage->ei_apsKernelSymHash[ nHash ];
    psImage->ei_apsKernelSymHash[ nHash ] = &psImage->ei_psSymbols[ i ];
  }

  psImage->ei_psNext = NULL;
  strcpy( psImage->ei_acName, "libkernel.so" );
  psImage->ei_pcPath = "";
  atomic_set( &psImage->ei_nOpenCount, 1 );
  psImage->ei_nSectionCount = 0;
  psImage->ei_psSections = NULL;
  psImage->ei_pcStrings = NULL;
  psImage->ei_nRelocCount = 0;
  psImage->ei_psRelocs = 0;
  psImage->ei_nSubImageCount = 0;
  psImage->ei_ppcSubImages = NULL;
  psImage->ei_nVirtualAddress = 1024 * 1024;
  psImage->ei_nTextSize = 0;
  psImage->ei_nOffset = 0;
  psImage->ei_nEntry = 0;
  psImage->ei_nInit = 0;
  psImage->ei_nFini = 0;
  psImage->ei_psFile = NULL;

  psInstance->eii_psImage = psImage;
  psInstance->eii_ppsSubImages = NULL;
  psInstance->eii_nHandle = 0;
  atomic_set( &psInstance->eii_nOpenCount, 1 );
  atomic_set( &psInstance->eii_nAppOpenCount, 1 );
  psInstance->eii_nTextArea = -1;
  psInstance->eii_nDataArea = -1;
  psInstance->eii_bRelocated = 1;

  g_psFirstImage = psImage;

  g_psKernelImageContext->ic_psInstances[ 0 ] = psInstance;
}

static void init_boot_modules_elf( void ) {
  int i;
  BootModule_t* psModule;
  ELFImageInstance_t* psInstance;

  kprintf( "Initializing boot modules\n" );

  for ( i = 0; i < MAX_BOOTMODULE_COUNT; i++ ) {
    psModule = get_boot_module( i );

    if ( !psModule->bm_bIsValid ) {
      continue;
    }

    char acFullPath[ 1024 ];
    const char* pcPath;
    int nError;
    int j;

    pcPath = strstr( psModule->bm_pcParameters, "/kernel/" );
    if ( pcPath == NULL ) {
      continue;
    }

    strcpy( acFullPath, "/giszOS/system" );
    j = strlen( acFullPath );
    while ( ( *pcPath != 0 ) && ( isspace( *pcPath ) == 0 ) ) {
      acFullPath[ j++ ] = *pcPath++;
    }

    acFullPath[ j ] = 0;

    while ( isspace( *pcPath ) ) {
      pcPath++;
    }

    //kprintf( "Initializing boot module: %s\n", acFullPath );
    nError = load_image_instance( g_psKernelImageContext, acFullPath, psModule, IM_KERNEL_SPACE, &psInstance, NULL );
    if ( nError < 0 ) {
      kprintf( "  Error: init_boot_modules() failed to initialize %s\n", acFullPath );
      continue;
    }

    atomic_inc( &psInstance->eii_nAppOpenCount );
  }

  for ( i = 0; i < MAX_IMAGE_COUNT; i++ ) {
    if ( ( g_psKernelImageContext->ic_psInstances[ i ] != NULL ) &&
         ( strstr( g_psKernelImageContext->ic_psInstances[ i ]->eii_psImage->ei_pcPath, "/kernel/drivers/" ) != NULL ) ) {
      init_boot_device( g_psKernelImageContext->ic_psInstances[ i ]->eii_psImage->ei_pcPath );
    }
  }
}

void init_elf_loader( void ) {
  g_psFirstImage = NULL;

  g_nImageListLock = create_semaphore( "image_list", 1, SEM_RECURSIVE );
  kassert( g_nImageListLock > 0 );

  create_kernel_image_context();
  init_boot_modules_elf();
}


Inode_t* get_image_inode( int nLibrary ) {
  ImageContext_t* psContext;
  ELFImageInstance_t* psInstance;
  Inode_t* psInode = NULL;

  if ( ( nLibrary < 0 ) || ( nLibrary >= MAX_IMAGE_COUNT ) ) {
    return NULL;
  }

  psContext = CURRENT_PROC_IMAGE_CONTEXT;  // FIXME: Should also consider kernel images

  LOCK( psContext->ic_nLock );

  psInstance = psContext->ic_psInstances[ nLibrary ];
  if ( psInstance == NULL ) {
    UNLOCK( psContext->ic_nLock );

    return NULL;
  }

  if ( psInstance->eii_psImage->ei_psFile != NULL ) {
    psInode = psInstance->eii_psImage->ei_psFile->f_psInode;
  }

  if ( psInode != NULL ) {
    atomic_inc( &psInode->i_nCount );
  }

  UNLOCK( psContext->ic_nLock );

  return psInode;
}


#endif

