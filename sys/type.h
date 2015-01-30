
#ifndef COMMON_TYPE_H
#define COMMON_TYPE_H
#if __GNUC__ < 3
#define __expect(foo,bar) (foo)
#else
#define __expect(foo,bar) __builtin_expect((long)(foo),bar)
#endif // __GNUC__

#define __likely(foo) __expect((foo),1)
#define __unlikely(foo) __expect((foo),0)
#define offsetof(structure,field) ((int)&((structure*)0)->field)

typedef unsigned long pte_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
typedef long tick_t;

typedef unsigned long uLongf;
#define Z_OK 0


#define NULL ((void *)0)
#define NIL_PTR ((char *)0)

#ifdef __cplusplus
#define __asmlink extern "C" 
#define  __public extern "C" 
#else
#define __asmlink extern
#define  __public
typedef enum {FALSE=0,TRUE=1} BOOL ;
typedef enum { false=0, true=1 } bool;
#endif

#define __local static

/*For gcc*/
#define __attr_packet __attribute__((packed))
#define __noreturn __attribute__((noreturn))  
#define __irq // __attribute__((interrupt)) //如果被定义，函数可能不再返回被调用函数


#define FAILED        1
#define SUCCESS   0
#define   UNUSED   0



typedef  void (*func_t) (void);
typedef  int (*fun_t) (char*);
typedef void thread_entry_t (void*);

typedef int semaphore_t;

typedef unsigned char   u8_t;  //字节
typedef unsigned short u16_t;  //字
typedef unsigned long  u32_t;  //双字
typedef unsigned long long int u64_t;

typedef  char   s8_t;  //字节
typedef  short s16_t;  //字
typedef  long  s32_t;  //双字
typedef  long long int s64_t;

typedef	 long	tid_t;
typedef	short	uid_t;
typedef	int	    pid_t;
typedef	unsigned long size_t;	/*  32 bits == 4 GB address space  */

typedef unsigned char gid_t;
typedef unsigned short dev_t;
typedef unsigned short ino_t;
typedef unsigned short mode_t;
typedef unsigned long time_t;
typedef long off_t;
typedef unsigned reg_t;
typedef unsigned short prot_t;

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned long u_long;
typedef unsigned short u_short;
typedef unsigned char u_char;
typedef unsigned int u_int;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define	peek8(x)	(*(u8_t *)(x))
#define	peek16(x)	(*(u16_t *)(x))
#define	peek32(x)	(*(u32_t *)(x))
#define	peek64(x)	(*(u64_t *)(x))

#define	poke8(x,n)	(*(u8_t *)(x)=((u8_t)n))
#define	poke16(x,n)	(*(u16_t *)(x)=((u16_t)n))
#define	poke32(x,n)	(*(u32_t *)(x)=((u32_t)n))
#define	poke64(x,n)	(*(u64_t *)(x)=((u64_t)n))

typedef void *linear_t;

#ifndef _MSC_VER
#define	__int64 long long
#endif
typedef	unsigned char		uint8;
typedef	signed char			sbyte,int8;
typedef	unsigned short		uint16;
typedef	signed short		int16;
typedef	unsigned int		uint,uint32;
typedef	signed int			int32;
typedef	unsigned __int64	qword,uint64;
typedef	signed __int64		int64;
typedef	void				*pvoid;
typedef	int32				HFILE;
typedef	int8				DOSRESULT;
typedef	const char			*string;

#define KATOMIC_INC(data, type) (*(type*)(&data))++
#define KATOMIC_DEC(data, type) (*(type*)(&data))--
#define KATOMIC_READ(data, type) (*(type*)(&data))

typedef	unsigned long addr_t;
#endif 
