
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------

#include <stdarg.h>
#include <jicama/system.h>
#include <string.h>

int hexch2int(char h)
{
	if( ((h) >= 'a' && (h) <= 'f') )
		return h-'a'+10;
	if( ((h) >= 'A' && (h) <= 'F') )
		return h-'A'+10;
	return h-'0';
}

char *int2str(int d, char *str1)
{
	unsigned char i=0;
	u8_t j;
	__local unsigned char str2[16];
	int digit = d;

	memset(str2, 0, 16);
	
	if(digit!=0 && i<16)
	{
		for(i=0; digit!=0; ++i) {
			str2[i]=digit%10+48;
			digit/=10;
		}

		for(j=0; j<i; ++j)
			str1[j]=str2[i-1-j];
			str1[i]='\0';
	}
	else{
		str1[0]='0';
		str1[1]='\0';
	}
	return str1;
}

 unsigned conv2(int norm, int w)
{
/////////// Possibly swap a 16-bit word between 8086 and 68000 byte order. 
/////////// TRUE if no swap, FALSE for byte swap 
/////////// promotion of 16-bit word to be swapped 
  if (norm) return( (unsigned) w & 0xFFFF);
  return( ((w&0377) << 8) | ( (w>>8) & 0377));
}

 long conv4(int norm, long x)
{
/////////// Possibly swap a 32-bit long between 8086 and 68000 byte order. 
/////////// TRUE if no swap, FALSE for byte swap 
/////////// 32-bit long to be byte swapped 

  unsigned lo, hi;
  long l;
  
  if (norm) return(x);			/////////// byte order was already ok 
  lo = conv2(0, (int) x & 0xFFFF);	/////////// low-order half, byte swapped 
  hi = conv2(0, (int) (x>>16) & 0xFFFF);	/////////// high-order half, swapped 
  l = ( (long) lo <<16) | hi;
  return(l);
}

void swap_char(u8_t* toswap)
{
	int i = 0;
	u8_t temp;

	while (toswap[i+1] != 0)
	{
		  temp = toswap[i];
		  toswap[i] = toswap[i+1];
		  toswap[i+1] = temp;

		  i += 2;
    }

	return;
}



