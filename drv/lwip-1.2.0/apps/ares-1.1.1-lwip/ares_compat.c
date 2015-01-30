/*******************************************************************************/
/* This file has been modified by Sergio Perez Alcañiz <serpeal@upvnet.upv.es> */
/*            Departamento de Informática de Sistemas y Computadores           */
/*            Universidad Politécnica de Valencia                              */
/*            Valencia (Spain)                                                 */
/*            April 2003                                                       */
/*******************************************************************************/

#include "ares_private.h"
#include <rtl_time.h>

inline int gettimeofday(struct timeval *tv, struct timezone *tz){
  //struct timespec now;
#define MICSEC (10000000/HZ)

	if (!tv)
	{
		return 0;
	}

  tv->tv_sec = get_unix_time();
  tv->tv_usec = (startup_ticks()%HZ)*MICSEC;;
  return 0;
} 

/* Compare S1 and S2, ignoring case, returning less than, equal to or
   greater than zero if S1 is lexicographically less than,
   equal to or greater than S2.  */
int
strcasecmp (const char *s1, const char *s2)
{
  register const unsigned char *p1 = (const unsigned char *) s1;
  register const unsigned char *p2 = (const unsigned char *) s2;
  unsigned char c1, c2;

  if (p1 == p2)
    return 0;




  do
    {
      c1 = tolower (*p1++);
      c2 = tolower (*p2++);
      if (c1 == '\0')
	break;
    }
  while (c1 == c2);

  return c1 - c2;
}

char *strdup(const char *s)
{
	char *ns;

	if(!s)
	    return NULL;

	ns = malloc(strlen(s)+1);
	if(ns)
		strcpy(ns, s);

	return ns;
}


time_t time(time_t *tp)
{

  struct timeval time;

//	long ticks = RetrieveClock();
//	long secs;

//	secs = (ticks * 4 + ticks * 8 / 13) / 1000;


  gettimeofday(&time, NULL);

  if(tp)
    *tp = (long) time.tv_sec;



	return (time_t) time.tv_sec;
}
