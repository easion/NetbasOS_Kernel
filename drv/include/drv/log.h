
#ifndef _KLOG_H_
#define _KLOG_H_


enum{
 LOG_EMERG	=0,
 LOG_ALERT	=1,
 LOG_CRIT	=2,
 LOG_ERR	=	3,
 LOG_WARNING=	4,
 LOG_NOTICE	=5,
 LOG_INFO	=6,
 LOG_DEBUG	=7,
 LOG_TRACE  =8
};

#define LOG_BUF_SIZE 4096

int syslog(int pri, const char *fmt, ...);

#endif

