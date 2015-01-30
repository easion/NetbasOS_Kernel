#include <stdarg.h>
#include <jicama/system.h>
#include <ansi.h>
#include <string.h>

/* Flag bit settings */
#define RESPECT_WIDTH	1  /* Fixed width wanted 	*/
#define ADD_PLUS	2  /* Add + for positive/floats */
#define SPACE_PAD	4  /* Padding possibility	*/
#define ZERO_PAD	8
#define LEFT_PAD 	16
#define INT_MIN   -2147483648
#define INT_MAX    2147483647


#define STD_SIZE	0
#define SHORT_SIZE	1
#define LONG_SIZE	2

#ifndef __ARM__

long unsigned strtou(char *s,int base,char **scan_end)
{
    int value,overflow = 0;
    long unsigned result = 0,oldresult;
    /* Skip trailing zeros */
    while (*s == '0') s++;
    if (*s == 'x' && base == 16) {
	s++;
	while (*s == '0') s++;
    }
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
    return(result);
}

#ifdef NO_KERNEL
double strtod(char *s,char **scan_end)
{
    int sign,i;
    double result = 0;
    double value;
    double mantissa = 0,divisor = 1;
    unsigned short power = 0;
    /* Evaluate sign */
    if (*s == '-') {
	sign = -1;
	s++;
    }
    else sign = 1;
    /* Skip trailing zeros */
    while (*s == '0') s++;
    /* Convert integer part */
    while (*s <= '9' && *s >= '0') {
	value = *s++ - '0';
	result *= 10.0;
	result += value;
    }
    /* Detect floating point & mantissa */
    if (*s == '.') {
	s++;
    	while (*s <= '9' && *s >= '0') {
	    value = *s++ - '0';
	    mantissa *= 10.0;
	    mantissa += value;
	    divisor *= 10.0;
	}
    }
    mantissa /= divisor;
    /* Adjust result */
    result += mantissa;
    /* Adjust sign */
    result *= sign;
    /* Detect exponent */
    if (*s == 'e' || *s == 'E') {
	s++;
	if (*s == '-') {
	    sign = -1;
	    s++;
	} else if (*s == '+') {
	    sign = 1;
	    s++;
	}
	else sign = 1;
    	while (*s <= '9' && *s >= '0') {
	    value = *s++ - '0';
	    power *= 10.0;
	    power += value;
	}
    }
    /* Adjust result on exponent sign */
    if (sign > 0) for (i = 0; i < power; i++) result *= 10.0;
    else for (i = 0; i < power; i++) result /= 10.0;
    if (scan_end != 0L) *scan_end = s;
    return(result);
}

#endif

int vsscanf(char *buf,char *fmt,va_list parms)
{
    int scanned = 0,size = 0,suppress = 0;
    int w = 0,flag = 0,l = 0;
    char c,*c_ptr;
    long int n1,*n1l;    
    int *n1b;
    short int *n1s;
    long unsigned n2,*n2l,parsing = 0;
    unsigned *n2b;
    short unsigned *n2s;
    //double n3,*n3l;
    float *n3s;
    char *base = buf;
    while (*fmt != 0) {
	if (*fmt != '%' && !parsing) {
	    /* No token detected */
	    fmt++;
	}
	else {
	    /* We need to make a conversion */
	    if (*fmt == '%') {
		fmt++;
		parsing = 1;
		size = STD_SIZE;
		suppress = 0;
		w = 0;
		flag = 0;
		l = 0;
	    }
	    /* Parse token */
	    switch(*fmt) {
		case '1' :
		case '2' :
		case '3' :
		case '4' :
		case '5' :
		case '6' :
		case '7' :
		case '8' :
		case '9' :
		case '0' : if (parsing == 1) {
		    	      w = strtou(fmt,10,&base);
			      /* We use SPACE_PAD to parse %10s
			         commands where the number is the
				 maximum number of char to store!
			      */
			      flag |= SPACE_PAD;
			      fmt = base-1;
			   }
			   break;
		case 'c' : c = *buf++;
			   c_ptr = va_arg(parms, char *);
			   *c_ptr = c;
			   scanned++;
			   parsing = 0;
			   break;
		case 's' : c_ptr = va_arg(parms, char *);
			   while (*buf != 0 && isspace(*buf)) buf++;
			   l = 0;
			   while (*buf != 0 && !isspace(*buf)) {
			       if (!(flag & SPACE_PAD)) *c_ptr++ = *buf;
			       else if (l < w) {
				   *c_ptr++ = *buf;
				   l++;
			       }
			       buf++;	     
			   }
			   *c_ptr = 0;
			   scanned++;
			   parsing = 0;
			   break;
		case 'i' :
		case 'd' : buf = strscn(buf,"1234567890-+");
			   n1 = strtoi(buf,10,&base);
			   buf = base;
			   if (!suppress) {
			       switch (size) {
				   case STD_SIZE : n1b = va_arg(parms, int *);
						   *n1b = (int)n1;
						   break;
				   case LONG_SIZE : n1l = va_arg(parms, long int *);
				   		    *n1l = n1;
						    break;
				   case SHORT_SIZE : n1s = va_arg(parms, short int *);
						     *n1s = (short)(n1);
						     break;
			       }
			       scanned++;
			   }
			   parsing = 0;
			   break;
		case 'u' : buf = strscn(buf,"1234567890");
			   n2 = strtou(buf,10,&base);
			   buf = base;
			   if (!suppress) {
			      switch (size) {
				  case STD_SIZE : n2b = va_arg(parms, unsigned *);
				  		  *n2b = (unsigned)n2;
						  break;
				  case LONG_SIZE : n2l = va_arg(parms, long unsigned *);
						   *n2l = n2;
						   break;
				  case SHORT_SIZE : n2s = va_arg(parms, short unsigned *);
						    *n2s = (short)(n2);
						    break;
			      }
			      scanned++;
			   }
			   parsing = 0;
			   break;
		case 'x' : buf = strscn(buf,"1234567890xabcdefABCDEF");
			   n2 = strtou(buf,16,&base);
			   buf = base;
			   if (!suppress) {
			      switch (size) {				  
			          case STD_SIZE : n2b = va_arg(parms, unsigned *);
				  		  *n2b = (unsigned)n2;
						  break;
				  case LONG_SIZE : n2l = va_arg(parms, long unsigned *);
						   *n2l = n2;
						   break;
				  case SHORT_SIZE : n2s = va_arg(parms, short unsigned *);
						    *n2s = (short)(n2);
						    break;
			      }
			      scanned++;
			   }
			   parsing = 0;
			   break;

		#ifdef NO_KERNEL
		case 'f' :
		case 'g' :
		case 'e' : buf = strscn(buf,"1234567890.e+-");
			   n3 = strtod(buf,&base);
			   buf = base;
			   if (!suppress) {			       
			       switch (size) {				   				 
			       	   case STD_SIZE : n3l = va_arg(parms, double *);
						   *n3l = n3;
						   break;
				   case LONG_SIZE : n3l = va_arg(parms, double *);
						    *n3l = n3;
						    break;
				   case SHORT_SIZE : n3s = va_arg(parms, float *);
						     *n3s = (float)(n3);
						     break;
			       }
			       scanned++;
			   }
			   parsing = 0;
			   break;
		 #endif
		case 'l' : size = LONG_SIZE;
			   break;
		case 'h' :
		case 'n' : size = SHORT_SIZE;
			   break;
		case '*' : suppress = 1;
			   break;
		default :  parsing = 0;
			   break;
	    }
	    fmt++;
	}
    }
    return(scanned);
}

int sscanf(char *buf,char *fmt,...)
{
    va_list parms;
    int result;
    va_start(parms,fmt);
    result = vsscanf(buf,fmt,parms);
    va_end(parms);
    return(result);
}
#endif

