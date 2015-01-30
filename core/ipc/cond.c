/*
**     (R)Jicama OS
**     messaging subsystem
**     Copyright (C) 2005 DengPingPing
*/
#include <jicama/process.h>
#include <jicama/trace.h>
#include <jicama/msgport.h>
#include <string.h>
#include <errno.h>

struct thread_cond
{
	thread_t *t;
	int count;
};

__public int cond_init(struct thread_cond *lock)
{
	return 0;
}


__public int cond_broadcast(struct thread_cond *lock)
{
	return 0;
}

__public int cond_signal(struct thread_cond *lock)
{
	return 0;
}

__public int cond_wait(struct thread_cond *lock)
{
	return 0;
}

__public int cond_timewait(struct thread_cond *lock)
{
	return 0;
}

