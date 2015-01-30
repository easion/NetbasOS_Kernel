#include <jicama/process.h>

#define	__NR_exit	0
#define	__NR_sleep	1
#define	__NR_write 	2

typedef int (*fn_ptr)(void);
extern int sys_exit(void);
extern int sys_sleep(void);
extern int sys_write(void);

//fn_ptr swi_table[] = {  sys_exit,sys_sleep,sys_write };


int uexit(int error_code)
{
 	int ret;
 	__asm__(
 		"mov	r0, %1\n"
 		"swi	%2\n"
 		"mov	%0, r0\n"
 		:"=r"(ret)
 		:"r"(error_code),"M"(__NR_exit)
 		:"r0"
 		);
 	return ret;
}


int usleep(int time)
{
 	int ret;
 	__asm__(
 		"mov	r0, %1\n"
 		"swi	%2\n"
 		"mov	%0, r0\n"
 		:"=r"(ret)
 		:"r"(time),"M"(__NR_sleep)
		:"r0"
		);
	return ret;
}

/**************************************************************
*  返回输出的字节数
***************************************************************/
int uwrite(char *str)
{
 	int ret;
 	__asm__(
 		"mov	r0, %1\n"
 		"swi	%2\n"
 		"mov	%0, r0\n"
 		:"=r"(ret)
 		:"r"(str),"M"(__NR_write)
		:"r0"
		);
	return ret;
}
