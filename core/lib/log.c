
#include <type.h>
#include <string.h>
#include <stdarg.h> 
#include <jicama/system.h> 
#include <jicama/fs.h> 
#include <jicama/log.h> 
#include <jicama/process.h> 

typedef int (*fnptr_t)(unsigned c, void **helper);
int do_printf(const char *fmt, va_list args, fnptr_t fn,int max, void *ptr);
int vprintf(int pri, const char *fmt,int max, va_list args);

__local int sys_log_level=LOG_NOTICE;
__local char log_buffer[LOG_BUF_SIZE];
__local int log_chars;
__local char *pri_msg[] = {
 "EMERG",
 "ALERT",
 "CRIT",
 "ERR",
 "WARNING",
 "NOTICE",
 "INFO",
 "DEBUG",
 "TRACE"
};

void log_init(void)
{
	log_chars = 0;
	sys_log_level = LOG_DEBUG;
	memset(log_buffer,0, sizeof(log_buffer));
}

__local void log_putchar(char ch)
{
	if (log_chars<LOG_BUF_SIZE)
	{
		log_buffer[log_chars] = ch;
		log_chars++;
		return ;
	}
	memcpy(log_buffer,&log_buffer[1],LOG_BUF_SIZE-1);
	log_buffer[LOG_BUF_SIZE-2] = ch;
	log_buffer[LOG_BUF_SIZE-1] = 0;
}


__local int log_help(unsigned c, void **ptr)
{
	log_putchar(c);
	return 0 ;
}

int log_vprintf(int pri, const char *fmt, int max, va_list args)
{
	return do_printf(fmt, args, log_help,max, NULL);
}

__local void log_putstr(char *str)
{
	while (*str)
	{
		log_putchar(*str++);
	}
}

int read_log(char *buf, int size)
{
	int len;

	log_chars = MIN(log_chars,LOG_BUF_SIZE);
	len = MIN(log_chars,size);
	memcpy(buf, log_buffer,len);
	return len;
}


void set_log_level(int val)
{
	sys_log_level = val;
}

int get_log_level()
{
	return sys_log_level ;
}



__local char *make_header(int pri)
{
	static char buf[60];
	 struct tm time;
	 thread_t *pthread=NULL;

	(void)get_time(&time);

	if (pri<LOG_USER)
	{
		sprintf(buf,"[%-08s%02d:%02d:%02d]:",pri_msg[pri%9], time.hour, time.min,time.sec);
	}
	else{
		pthread=current_thread();
		sprintf(buf,"[%-08s%02d:%02d:%02d]:",pthread->name, time.hour, time.min,time.sec);
	}

	return buf;
}



int syslog(int pri, const char *fmt, ...)
{
	va_list args;
	int rv;
	char *head = make_header(pri);

	if (pri>sys_log_level && pri<LOG_USER)	{
		return 0;
	}

	log_putstr(head);
	va_start(args, fmt);
	if (pri<=LOG_ERR){
		vprintf(pri, fmt, 4096,args);
	}
	rv = log_vprintf(pri, fmt, 4096,args);
	va_end(args);
	return rv;
}


