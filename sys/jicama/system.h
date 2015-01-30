
// ---------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//-----------------------------------------------------------------------------------
#ifndef JICAMA_SYSTEM_H
#define JICAMA_SYSTEM_H

#include <jicama/config.h>
#include <type.h>
#include <errno.h>
#include <jicama/kernel.h>
#include <jicama/trace.h>
#include <sys/queue.h>
#include <jicama/log.h>

#define barrier() __asm__ __volatile__("": : :"memory")

#define USEC_IN_SEC	1000000 
#define USECS_PER_TICK  (USEC_IN_SEC / HZ)/*Ã¿ÃëµÎ´ðÊý£º10000*/
#define MSECS_PER_TICK  (1000 / HZ) /*Ã¿µÎ´ðÎ¢ÃëÊý£º10*/
#define INFINITE  (time_t)-1


int backtrace(void **buffer, int size);
char** backtrace_symbols(void *const *buffer, int size);



__asmlink char* get_kernel_command(void);
void dump_devicesinfo(char *buf, int len);
int check_debug(void);
int check_kmem(void);

int create_sys_task(void);
time_t msec2ticks(time_t msec);


__asmlink void swap_char(u8_t* toswap);
__asmlink unsigned rand(void);
__asmlink void srand(unsigned new_seed);
__asmlink void chinese_area(u32_t *addr, size_t *sz);
__asmlink unsigned long get_unix_time(void);
__asmlink  time_t get_unixtime(time_t *t);
__asmlink  time_t get_unix_sec();
__asmlink  time_t startup_ticks();

int load_kernel_dlls(char *arg);

u32_t _sbrk(size_t size);
int set_alarm(time_t msecs);

int sig_invalid(unsigned long bits);
void set_time(struct timeval *tv);

int register_module_handler(void *mod,void *arg);
int unregister_module_handler(void *mod,void *arg);

void arch_sym_setup(void);


/*device driver module support!*/
__asmlink int puts(const unsigned char *);
__asmlink  int hex2num(char *str);
__asmlink void flush_all(dev_t dev);
__asmlink  unsigned long get_page();
 __asmlink void * low_alloc(u16_t len);
__asmlink   int low_free(void* p,u32_t len);
int tty_putchar(const unsigned char c);
int vt100_conv (int ch, char *buf);
int ioport_init(void);
int ramdisk_init(void);
void gzip_ran_init(void);


 void dis_irq (const unsigned int nr) ;
 void en_irq (const unsigned int nr);


void get_time( struct tm *time);














#endif


