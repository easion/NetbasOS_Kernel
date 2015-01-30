
/************************************************************
  Copyright (C), 2003-2010, Netbas OS Project.
  FileName: 
  Author:        Version :          Date:
  Description:    
  Version:        
  Function List:   
  History:         
      Easion   2010/2/6     1.0     build this moudle  
***********************************************************/

#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/errno.h>


#if 0
// If divisible by 4, but not divisible by 100, but divisible by 400, it's a leap year
// 1996 is leap, 1900 is not, 2000 is, 2100 is not
#define IS_LEAP_YEAR(y) ((((y) % 4) == 0) && (((y) % 100) || ((((y)) % 400) == 0)))
int tzoffset=8;
/* returns leap days since 1970 */
static int leaps(int yr, int mon)
{
	// yr is 1970-based, mon 0-based
	int result = (yr+2)/4 - (yr + 70) / 100;
	if((yr+70) >= 100) result++; // correct for 2000
	if (IS_LEAP_YEAR(yr + 1970))
		if (mon < 2) result--;
	return result;
}

static int daze[] = { 0,0,31,59,90,120,151,181,212,243,273,304,334,0,0,0 };

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

time_t dos2time_t(u32_t t)
{
	time_t days;

	//get_tzoffset();

	//dprintf("%d/%d/%d %d:%2.2d:%2.2d\n",
	//	(t>>25)+1980,((t>>21)&15),((t>>16)&31),
	//	(t>>11)&31,(t>>5)&63,2*(t&31));

	days = daze[(t>>21)&15] + ((t>>25)+10)*365 + leaps((t>>25)+10,((t>>21)&15)-1)+((t>>16)&31)-1;

	return (((days * 24) + ((t>>11)&31)) * 60 + ((t>>5)&63) + tzoffset) * 60 + 2*(t&31);
}

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

u32_t time_t2dos(time_t s)
{
	u32_t t, d, y;
	int days;

	//get_tzoffset();

	t = (s % 60) / 2;	s /= 60; s -= tzoffset;
	t += (s % 60) << 5;	s /= 60;
	t += (s % 24) << 11;s /= 24;

	s -= 10*365 + 2; // convert from 1970-based year to 1980-based year

	for (y=0;;y++) {
		days = IS_LEAP_YEAR(1980+y) ? 366 : 365;
		if (s < days) break;
		s -= days;
	}

	if (IS_LEAP_YEAR(1980+y)) {
		if (s == 59) {
			d = (1 << 5) + 28; /* 2/29, 0 based */
			goto bi;
		} else if (s > 59)
			s--;
	}

	for (d=0;d<11;d++)
		if (daze[d+2] > s)
			break;
	d = (d << 5) + (s - daze[d+1]);

bi:
	d += (1 << 5) + 1; // make date 1-based

	return t + (d << 16) + (y << 25);
}
#endif

