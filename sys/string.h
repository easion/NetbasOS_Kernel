
#ifndef STRING_H
#define STRING_H



#include <type.h>

__asmlink  int sscanf(char *buf,char *fmt,...);
__asmlink  size_t snprintf (char *str, size_t len, const char *fmt, ...);
__asmlink int sprintf(char *buf, const char *fmt, ...);
__asmlink int tty_putchar(const unsigned char c);


 int stricmp(const char *_src1, const char *_src2);
int strnicmp(const char *s1, const char *s2, size_t count);
int hex2num(char *str);
void	bzero(u8_t *buf,size_t sz);
char *strscn(char *s,char *pattern);
int isnumber(char c,int base);
int tonumber(char c);
long int strtoi(char *s,int base,char **scan_end);
void *memmove(void *dst, const void *src, size_t count);

char*    strcpy(char*, const char*);
char*    strncpy(char*, const char*, size_t);
char *	strchr(const char *_s, int _c);
int      strlen(const char*);
int      strcmp(const char*, const char*);
int	strncmp(const char *_s1, const char *_s2, size_t _n);
u32_t    strtoul(const char*, char**, int);
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
unsigned int atoi( const char *str );
char*dirname (char *path);
char* basename (char *path);


/**/ char *strcat(char *src_char, const char *add_char);
/**/ char * strfind(const char *p, int c);
/**/ void swap_char(unsigned char* toswap);
double cos( double angle );
double fabs( double __x );
double sin( double angle );
double sqrt( double x );
double tan( double __x );
u16_t wswap( u16_t x );



#endif

