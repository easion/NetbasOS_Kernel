
#ifndef __DRV_DEFS_H__
#define __DRV_DEFS_H__
#ifdef __cplusplus
extern "C" {
#ifndef NULL
#define NULL		0
#endif
#else
#ifndef NULL
#define NULL		(void*)0
#endif
#endif

#define __attr_packet __attribute__((packed))

#define __asmlink extern
#define  __public
#define __local static
#define INFINITE  (time_t)-1


#define FAILED        1
#define SUCCESS   0
#define   UNUSED   0
#define ALLOC_MAJOR_NUMBER -1
#define offsetof(structure,field) ((int)&((structure*)0)->field)

typedef unsigned char gid_t;
typedef unsigned short dev_t;
typedef unsigned short ino_t;
typedef unsigned short mode_t;
typedef unsigned long time_t;
typedef unsigned long off_t;
typedef unsigned reg_t;
typedef unsigned short prot_t;
#define OK		0

typedef enum {FALSE=0,TRUE=1} BOOL ;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

typedef	short	uid_t;
typedef	int	    pid_t;
typedef	unsigned long size_t;	/*  32 bits == 4 GB address space  */


typedef unsigned char   u8_t;  //×Ö½Ú
typedef unsigned short u16_t;  //×Ö
typedef unsigned long  u32_t;  //Ë«×Ö
typedef signed char   s8_t;  //×Ö½Ú
typedef signed short s16_t;  //×Ö
typedef signed long  s32_t;  //Ë«×Ö

typedef unsigned long long int u64_t;
typedef  long long int s64_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;
#include <sys/queue.h>
//#include <assert.h>
typedef struct thread_wait
{
	TAILQ_HEAD(, thread_wait) head;
}wait_queue_head_t;
#define printf kprintf

#ifdef __cplusplus
}
#else
typedef enum { false, true } bool;
#endif

#endif


