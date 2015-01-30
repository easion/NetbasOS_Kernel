
#ifndef STRING_H
#define STRING_H

#ifdef __cplusplus
extern "C" {
#endif


#include <drv/defs.h>

  size_t snprintf (char *str, size_t len, const char *fmt, ...);
 int sprintf(char *buf, const char *fmt, ...);


 int stricmp(const char *_src1, const char *_src2);
int strnicmp(const char *s1, const char *s2, size_t count);
int hex2num(char *str);
void	bzero(unsigned char *buf,size_t sz);
char *strscn(char *s,char *pattern);
int isnumber(char c,int base);
int tonumber(char c);
long int strtoi(char *s,int base,char **scan_end);

char*    strcpy(char*, const char*);
char*    strncpy(char*, const char*, size_t);
char *	strchr(const char *_s, int _c);
int      strlen(const char*);
int      strcmp(const char*, const char*);
int	strncmp(const char *_s1, const char *_s2, size_t _n);
unsigned long    strtoul(const char*, char**, int);
long     strtol(const char*, char**, int);
size_t   strcspn(const char*, const char*);
int      strcoll(const char*, const char*);
char*    strpbrk(const char*, const char*);
size_t   strspn(const char*, const char*);
char*    strstr(const char*, const char*);
char*    strtok(char*, const char*);
size_t   strxfrm(char*, const char*, size_t);

void*    memchr(const void* s, int c, size_t n);
int      memcmp(const void* s1, const void* s2, size_t n);
void*    memset(void* dest, int c, size_t count);
void*    memcpy(void* dest, const void* src, size_t count);

/**/ char *strcat(char *src_char, const char *add_char);
/**/ char * strfind(const char *p, int c);
/**/ void swap_char(unsigned char* toswap);
double cos( double angle );
double fabs( double __x );
double sin( double angle );
double sqrt( double x );
double tan( double __x );
unsigned short wswap( unsigned short x );


#ifdef __cplusplus
}
#endif

#endif

