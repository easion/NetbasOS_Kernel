
#include <ansi.h>
#include <jicama/system.h>
#include <string.h>


int isnumber(char c,int base)
{
    __local char *digits = "0123456789ABCDEF";
    if ((c >= '0' && c <= digits[base-1]))
       return(1);
    else return(0);
}

int tonumber(char c)
{
    if (c >= '0' && c <= '9') return(c - '0');
    else if (c >= 'A' && c <= 'F') return(c - 'A' + 10);
    else if (c >= 'a' && c <= 'f') return(c - 'a' + 10);
    else return(c);
}

char todigit(int c)
{
    if (c >= 0 && c <= 9) return(c+'0');
    else if (c >= 10 && c <= 15) return(c + 'A' - 10);
    else return(c);    
}

int hex2num(char *str)
{
	int i, num;

	i=num=0;

	while (isxdigit(str[i]))
	{
		num+=num*16+tonumber(str[i]);
		i++;
	}
	
	return num;
}



unsigned int atoi( const char *str )
{
    unsigned int i, res = 0;

    for ( i = 0; i < strlen( str ) && isdigit( str[ i ] ); i++ )
	   res = res * 10 + str[ i ] - '0';

    return res;
}

char *strcpy(char *src_char, const char *add_char)
{
	char *ret = src_char;

	while (*add_char != '\0')
		*src_char++ = *add_char++;

	*src_char++ = '\0';

	return ret;
}

char *strncpy(char *src_char, const char *add_char, size_t nr)
{
	int i = 0;
	char *ret = src_char;

	while (*add_char != '\0' && i<nr){
		*src_char++ = *add_char++;
		i++;
	}

	*src_char++ = '\0';

	return ret;
}




int strncmp (const char *s1, const char *s2, size_t len)
  {
    int i = 0;

    if (!s1 || !s2)
	    return 1;

    if (len < 0)
	   return 1;

    while (!(s1[i]==0 && s2[i]==0) && i<len)
      {
	if (s1[i] < s2[i])	return -1;
	if (s1[i] > s2[i])	return 1;
	i++;
      }

    return 0;
  }

  int stricmp(const char *_src1, const char *_src2)
{ 
   for ( ; ( *_src1 && *_src2 ) && 
     ((tolower(*_src1)) == (tolower(*_src2)));
     _src1++, _src2++ )
     ;/* nothing here */
 
   return ((tolower(*_src1)) - (tolower(*_src2)));
 }/* int stricmp(const char *_src1, const char *_src2) */

int mystricmp(const char *s1, const char *s2)
{
  char f, l;

  do 
  {
    f = ((*s1 <= 'Z') && (*s1 >= 'A')) ? *s1 + 'a' - 'A' : *s1;
    l = ((*s2 <= 'Z') && (*s2 >= 'A')) ? *s2 + 'a' - 'A' : *s2;
    s1++;
    s2++;
  } while ((f) && (f == l));

  return (int) (f - l);
}
int strnicmp(const char *s1, const char *s2, size_t count)
{
  int f, l;

  do 
  {
      if (((f = (unsigned char)(*(s1++))) >= 'A') && (f <= 'Z')) f -= 'A' - 'a';
      if (((l = (unsigned char)(*(s2++))) >= 'A') && (l <= 'Z')) l -= 'A' - 'a';
  } while (--count && f && (f == l));

  return f - l;
}

void be_zero(unsigned char* s, unsigned int len)
{
	register unsigned int lon;
	if ((lon=len) <= 0)
    return;

	do
		*s++ = 0; //'\0';
     while (--lon);
}

char *strcat(char *src_char, const char *add_char)
{
	char *ret = src_char;

	while (*src_char != '\0')
		src_char++;

	while (*add_char != '\0')
		*src_char++ = *add_char++;

	*src_char++ = '\0';

	return ret;
}

char * strfind(const char *p, int c)
{
   int c2;

   do {
      c2 = *p++;
      if (c2 == c) {
         return((char *)(p-1));
      }
   } while (c2);

   return NIL_PTR;
}

char* basename (char *path)
{
	char *fname;

	fname = path + strlen(path) - 1;
	while (fname >= path) {
		if (*fname == '/') {
			fname++;
			break;
		}
		fname--;
	}
	return fname;
}

char*dirname (char *path)
{
	char *fname;

	fname = basename (path);
	--fname;
	*fname = '\0';
	return path;
}

 void *memset (void *b, int c, unsigned long len)
  {
    unsigned char *p;
    unsigned int i;

    p = (unsigned char *) b;
    for (i=0; i<len; i++)
	p[i] = c;
    return b;
  }


void *memcpy(void *dst_ptr, const void *src_ptr, unsigned long count) 
{
	void *ret_val = dst_ptr;
	const char *src = (const char *)src_ptr;
	char *dst = (char *)dst_ptr;

// copy up /
	for(; count != 0; count--)
		*dst++ = *src++;
	return ret_val;
}


int strlen (const char *s)
  {
    unsigned int count = 0;
    while (s[0])
	s++, count++;
    return count;
  }


int strcmp (const char *s1, const char *s2)
  {
    int i = 0;

    if (!s1 || !s2)  //if one is null.
	   return 1;

    while (!(s1[i]==0 && s2[i]==0))
      {
	if (s1[i] < s2[i])	return -1;
	if (s1[i] > s2[i])	return 1;
	i++;
      }

    return 0;
  }

 
 char *strstr ( const char *_src1, const char *_src2 ) 
 {
   register const size_t st_len = strlen( _src2 );
  
   /* empty! nothing to do here */
   if ( st_len == 0 ) 
     return ( ( char * )_src1 );
 
   while ( ( *_src1 != *_src2 ) || 
           ( strncmp( _src1, _src2, st_len ) ) ) 
   {
     if ( *_src1++ == '\0' )
       return (  NIL_PTR );
   }/* end while */
 
   return ( ( char * ) _src1 );
 }

__local int hex_offset(char c)
{
	int i;
	char arrl[17]="0123456789abcdef";
	char arrh[17]="0123456789ABCDEF";

	for (i=0; i<16; i++)
		if(arrl[i]==c)
		return i;

	for (i=0; i<16; i++)
		if(arrh[i]==c)
		return i;

	return -1;
}


int hex2int(const char *str)
{
	int i, t, num=0;
	int len=strlen(str);

	for (i=0; i<len; i++)
	{
	   t = hex_offset(str[i]);
	   if(t>=0) num=num*0x10+t;
	   else break;
	}

	return num;
}

char *strscn(char *s,char *pattern)
{
    char *scan;
    while (*s != 0) {
    	scan = pattern;
    	while (*scan != 0) {
	    if (*s == *scan) return(s);
	    else scan++;
	}
	s++;
    }
    return(NIL_PTR);
}

long int strtoi(char *s,int base,char **scan_end)
{
#define INT_MIN   -2147483648
#define INT_MAX    2147483647
#define UINT_MAX   4294967295

    int sign,value,overflow = 0;
    long int result = 0,oldresult;
    /* Evaluate sign */
    if (*s == '-') {
	sign = -1;
	s++;
    } else if (*s == '+') {
	sign = 1;
	s++;
    }
    else sign = 1;
    /* Skip trailing zeros */
    while (*s == '0') s++;
    /* Convert number */
    while (isnumber(*s,base)) {
	value = tonumber(*s++);
	if (value > base || value < 0) return(0);
	oldresult = result;
	result *= base;
	result += value;
	/* Detect overflow */
	if (oldresult > result) overflow = 1;
    }
    if (scan_end != 0L) *scan_end = s;
    if (overflow) result = INT_MAX;
    result *= sign;
    return(result);
}

int memcmp(const void *left_p, const void *right_p, size_t count)
{
	const unsigned char *left = (const unsigned char *)left_p;
	const unsigned char *right = (const unsigned char *)right_p;

	for(; count != 0; count--)
	{
		if(*left != *right)
			return *left -  *right;
		left++;
		right++;
	}
	return 0;
}

void	bzero(u8_t *buf,size_t sz)
{
	size_t	i;

	for(i=0;i<sz;i++)
		buf[i] = 0;
}

/*
 * Copyright (c) 1996 Andy Valencia
 *
 * strchr()
 * Return pointer to first occurence, or 0
 */
char *
strchr(const char *p, int c)
{
   int c2;

   do {
      c2 = *p++;
      if (c2 == c) {
         return((char *)(p-1));
      }
   } while (c2);
   return(0);
}
