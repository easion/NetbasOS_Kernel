
#ifndef __RLIMIT_H__
#define __RLIMIT_H__

#define RLIMIT_CPU	0		/* CPU time in ms */
#define RLIMIT_FSIZE	1		/* Maximum filesize */
#define RLIMIT_DATA	2		/* max data size */
#define RLIMIT_STACK	3		/* max stack size */
#define RLIMIT_CORE	4		/* max core file size */
#define RLIMIT_RSS	5		/* max resident set size */

#define RLIM_NLIMITS	6

#define RLIM_INFINITY	0x7fffffff

struct rlimit {
	int	rlim_cur;
	int	rlim_max;
};

#endif

