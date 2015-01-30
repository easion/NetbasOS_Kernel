
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------
#ifndef JICAMA_KERNEL_H
#define JICAMA_KERNEL_H



#define ROOT_DEV   (boot_parameters.bp_rootdev)
#define IMAGE_DEV  (boot_parameters.bp_ramimagedev)

/* Device numbers of RAM, floppy and hard disk devices.
 * h/com.h defines RAM_DEV but only as the minor number.
 */
#define DEV_FD0   0x200
#define DEV_HD0   0x300
#define DEV_RAM   0x100
#define DEV_SCSI  0x700	/* Atari TT only */
#define DEV_DSP 0x800

#define NODEV -1

/* form minix, major device*/

#define MEM 1
#define FD      2
#define HD     3
#define TTYX 4
#define TTY 5
#define LP 6
#define PIPE 7

#define MAJOR(dev_nr) (((dev_t)(dev_nr))>>8)

/*minjor device*/
#define MINOR(dev_nr) ((dev_nr)&0xff)


/* Structure to hold boot parameters. */
struct bparam_s;

volatile extern struct bparam_s boot_parameters;

/*TIME STRUCT*/

struct tm
{
  int sec;			// Seconds after the minute [0, 59]
  int min;			// Minutes after the hour [0, 59]
  int hour;			// Hours since midnight [0, 23]

  int day;			// Day of the month [1, 31]
  int month;			// Months since January [0, 11]
  int year;			// Years since 1900
  int dayofweek;			// Days since Sunday [0, 6]
  int yday;			// Days since January 1 [0, 365]

  int isdst;			// Daylight Saving Time flag
  char *__tm_zone;
  int __tm_gmtoff;
};

typedef struct  {
  time_t tms_utime;		// 用户使用的CPU 时间。
  time_t tms_stime;		// 系统（内核）CPU 时间。
  time_t tms_cutime;		// 已终止的子进程使用的用户CPU 时间。
  time_t tms_cstime;		// 已终止的子进程使用的系统CPU 时间。
}tms_t;

struct timeval 
{
  long tv_sec;		        // Seconds
  long tv_usec;		        // Microseconds
};
//timezone 结构定义为:
struct timezone{
int tz_minuteswest; /*和Greenwich 时间差了多少分钟*/
int tz_dsttime; /*日光节约时间的状态*/
};
extern time_t volatile upsince;

/* System calls. */
#define SEND		   1	/* function code for sending messages */
#define RECEIVE		   2	/* function code for receiving messages */
#define BOTH		   3	/* function code for SEND + RECEIVE */
#define REPLY		   4	/* function code for SEND + RECEIVE */

#define KDEBUGMSG "\x1B[32mDEBUG:\x1B[34;0m"
#define KINFOMSG "\x1B[32mINFO:\x1B[34;0m"
#define KFAILEDMSG "\x1B[32mFAILED:\x1B[34;0m"
#define KOKMSG "\x1B[32mOK:\x1B[34;0m"
#define KWARNMSG "\x1B[32mwarn:\x1B[34;0m"

time_t startup_ticks(void);

#define insertQ(node_t) \
node_t *prev; \
 node_t *rest; \
node_t *next; 




__asmlink void enable(void);
__asmlink void disable(void);


__asmlink  int key_cook(	int value,	func_t p);


/*device driver module support!*/
__asmlink int puts(const unsigned char *);
__asmlink  int hex2num(char *str);
__asmlink void flush_all(dev_t dev);
 __asmlink void mm_free(u32_t base, size_t clicks);
__asmlink  unsigned long get_page(void);


__asmlink  int strnicmp(const char *s1, const char *s2, size_t count);
 __asmlink void do_gettime(struct tm*  toget);
__asmlink  int put_syscall(int nr, void(*fn)(void));
__asmlink  int put_interrupt(int nr, void(*fn)(void));
__asmlink void swap_char(u8_t* toswap);
__asmlink unsigned rand(void);
__asmlink void srand(unsigned new_seed);
__asmlink void chinese_area(u32_t *addr, size_t *sz);
__asmlink unsigned long get_unix_time(void);
__asmlink  time_t time(time_t *t);
__asmlink  time_t get_unix_sec(void);
__asmlink  time_t startup_ticks(void);
unsigned long to_unix_time(struct tm * time);


int memcmp(const void *left_p, const void *right_p, size_t count);
void* memcpy_from_user( void *dst_ptr, const void *src_ptr, unsigned long count );
void* memcpy_to_user( void *dst_ptr, const void *src_ptr, unsigned long count );
void write_user_dword(unsigned long val,unsigned long * addr);
void write_user_word(unsigned short val,unsigned short * addr);
void write_user_byte(char val,char *addr);
unsigned char read_user_byte(const char *addr);
unsigned short read_user_word(const u16_t *addr);
unsigned long read_user_dword(const u32_t *addr);

#define UnixCall(name) UNIX_##name
#define SysCall(name) SYS_##name
#define EmptyCall(name)   UNIX_nosys
#define SYSCALL_NOT_SUPPORT do\
	{\
	syslog(3,"%s not support yet!\n", __FUNCTION__);\
	return ENOSYS;\
	}\
	while (0);\


#endif

