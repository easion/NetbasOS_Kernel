

#ifndef _LINUX_MODULE_DEFS_H
#define _LINUX_MODULE_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#define __KERNEL__

/* Linux pre-C99 types */
typedef signed char		s8;
typedef unsigned char		u8;
typedef signed short		s16;
typedef unsigned short		u16;
typedef signed long		s32;
typedef unsigned long		u32;
typedef long long		s64;
typedef unsigned long long		u64;

/* Linux pre-C99 types */
typedef signed char		__s8;
typedef unsigned char		__u8;
typedef signed short		__s16;
typedef unsigned short		__u16;
typedef signed long		__s32;
typedef unsigned long		__u32;
typedef long long		__s64;
typedef unsigned long long		__u64;

typedef unsigned char		dma_addr_t;
typedef long loff_t ;
typedef long ssize_t ;

#define ARRAY_SIZE(x) \
	(sizeof(x) / sizeof((x)[0]))

/* There is no EISA PnP support, some drivers need to know about it */
#define EISA_bus 0

/* Old timing code may use these */
#define jiffies startup_ticks()
#define JIFF2U(a)	((a)*1000LL)    /* Convert time in JIFFIES to micro sec. */




/* I/O */
#define FMODE_READ		1	
#define FMODE_WRITE		2

/* Memory */
#define ALIGN( p, m) \
	((typeof(p))(((u32_t)(p)+m)&~m))
#define ALIGN8( p ) \
	ALIGN(p,7)
#define ALIGN16( p ) \
	ALIGN(p,15)
#define ALIGN32( p ) \
	ALIGN(p,31)

#define get_unaligned(ptr) \
	(*(ptr))
#define put_unaligned(val, ptr) \
	((void)( *(ptr) = (val) ))

#define virt_to_phys(p)	((uint32)(p))
#define virt_to_bus		virt_to_phys
#define phys_to_virt(p) ((void*)(p))
#define bus_to_virt 	phys_to_virt

#define cpu_to_le32(n)	((uint32)(n))
#define le32_to_cpu(n)	((uint32)(n))
#define cpu_to_le16(x)	((uint16)(x))
#define le16_to_cpu(x)	((uint16)(x))

#define virt_to_le32desc(addr) \
	cpu_to_le32(virt_to_bus(addr))
#define le32desc_to_virt(addr) \
	bus_to_virt(le32_to_cpu(addr))

#ifndef NULL
#define NULL (void*)0
#endif





enum{
	GFP_KERNEL,
};


enum{
	EV_KEY,
	EV_SYN,
	EV_REL,
	KEY_ESC,
	KEY_COMPOSE
};

/*Êó±êÊÂ¼þ*/




#define	MEMF_KERNEL	    0x00000010 ///< Kernel memory, must be supervisor
#define MEMF_OKTOFAILHACK   0x00000020 ///< Ugly hack to let new fail-safe code
				       ///< avoid block forever when out of mem
//#define MEMF_PRI_MASK	    0x000000ff
#define	MEMF_NOBLOCK	    0x00000100 ///< make kmalloc fail rather than wait
				       ///< for pages to be swapped out
#define MEMF_CLEAR   	    0x00010000 ///< kmalloc: return a zero filled area
#define	MEMF_LOCKED	    0x10000000 ///< memory is non-pageable


#ifdef __cplusplus
}
#endif

#endif /* _LINUX_MODULE_DEFS_H */
