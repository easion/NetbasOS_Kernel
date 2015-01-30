/*
** Copyright 2001, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef _TYPES_H
#define _TYPES_H

#if defined(__i386__) || defined(__I386__)
#undef i386
#define INC_ARCH(path, x) <path/i386/x>
#endif
#if defined(__ppc__) || defined(__PPC__)
#undef ppc
#define INC_ARCH(path, x) <path/ppc/x>
#endif
#if defined(__x86_64__) || defined(__X86_64__)
#undef x86_64
#define INC_ARCH(path, x) <path/x86_64/x>
#endif
#if defined(__arm__) || defined(__ARM__)
#undef arm
#define INC_ARCH(path, x) <path/arm/x>
#endif

//#include INC_ARCH(arch,types.h)

#ifndef __cplusplus

#define false 0
#define true 1
typedef int bool;

#endif

/*
   XXX serious hack that doesn't really solve the problem.
   As of right now, some versions of the toolchain expect size_t to
   be unsigned long (newer ones than 2.95.2 and beos), and the older
   ones need it to be unsigned int. It's an actual failure when
   operator new is declared. This will have to be resolved in the
   future.
*/

#ifdef ARCH_m68k
#define __SIZE_T_LONG       1
#endif

#ifdef ARCH_ppc
#define __SIZE_T_INT        1
#endif

#ifdef ARCH_sh4
#define __SIZE_T_INT        1
#endif

#if !__SIZE_T_LONG && !__SIZE_T_INT
/* when in doubt, set it to LONG */
#define __SIZE_T_LONG       1
#endif

/* uncomment the following if you want to override LONG vs INT */
#if 0
#ifdef __SIZE_T_LONG
#undef __SIZE_T_LONG
#endif
#ifdef __SIZE_T_INT
#undef __SIZE_T_INT
#endif

/* to override the LONG vs INT setting, set *one* of the below to 1 */
#define __SIZE_T_LONG       0
#define __SIZE_T_INT        1
#endif
#ifndef WIN32
typedef volatile unsigned long long vuint64;
typedef unsigned long long           uint64;
typedef volatile long long           vint64;
typedef long long                     int64;
#else
typedef volatile unsigned __int64   vuint64;
typedef unsigned __int64             uint64;
typedef volatile __int64             vint64;
#endif

#define PAGE_SIZE 4096
#define SYS_MAX_OS_NAME_LEN   32
enum {
	REGION_WIRING_LAZY = 0,
	REGION_WIRING_WIRED,
	REGION_WIRING_WIRED_ALREADY,
	REGION_WIRING_WIRED_CONTIG
 };

	#define LOCK_RO        0x0
#define LOCK_RW        0x1
#define LOCK_KERNEL    0x2
#define LOCK_MASK      0x3
// note that we only declare a proxy typedef here for size_t, called _newos_size_t
// which stddef.h then retypedefs as the actual size_t. ssize_t is not C++ standard,
// so it belongs here as well as anywhere.

#if __SIZE_T_LONG
typedef unsigned long       _newos_size_t;
//typedef signed long         ssize_t;
#elif __SIZE_T_INT
typedef unsigned int        _newos_size_t;
//typedef signed int          ssize_t;
#else
#error "Don't know what size_t should be (int or long)!"
#endif
typedef volatile unsigned int       vuint32;
typedef unsigned int                 uint32;
typedef volatile int                 vint32;
typedef int                           int32;
typedef volatile unsigned short     vuint16;
typedef unsigned short               uint16;
typedef volatile short               vint16;
typedef short                         int16;
typedef volatile unsigned char       vuint8;
typedef unsigned char                 uint8;
typedef volatile char                 vint8;
typedef char                           int8;
typedef unsigned long addr_t;
//typedef int64               off_t;

//typedef unsigned char       u_char;
//typedef unsigned short      u_short;
//typedef unsigned int        u_int;
//typedef unsigned long       u_long;
/*
typedef unsigned char       uchar;
typedef unsigned short      ushort;
typedef unsigned int        uint;
typedef unsigned long       ulong;
*/
typedef volatile unsigned char       vuchar;
typedef volatile unsigned short      vushort;
typedef volatile unsigned int        vuint;
typedef volatile unsigned long       vulong;

/* system types */
typedef int64 bigtime_t;
typedef uint64 vnode_id;
typedef int region_id;      // vm region id
typedef int aspace_id;      // address space id
typedef int thread_id;      // thread id
typedef int proc_id;        // process id
typedef int pgrp_id;        // process group id
typedef int sess_id;        // session id
typedef int sem_id;         // semaphore id
typedef int port_id;        // ipc port id
typedef int image_id;       // binary image id

# include <stddef.h>

#endif

