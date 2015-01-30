/*
**     (R)Jicama OS
**     Real Time Controller
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/timer.h>

struct alarm_timer
{
	thread_t *thread;
	struct timer timer;
	void *arg;
};

__local void alarm_handler(void *arg)
{
	struct alarm_timer *at = arg;
	thread_t *t = at->thread;	

	(int)at->arg++;
	remove_timer(&at->timer);
	kfree((u32_t)at);
	sendsig(t, SIGALRM);
}


int set_alarm(time_t msecs)
{
	struct alarm_timer *at;
	int rc;

	at = (void*)kmalloc(sizeof (struct alarm_timer),0);

	if (!at)
	{
		return -1;
	}

	at->thread = current_thread();
	at->arg = (void*)0;

	if (msecs == 0)
	{
	//yield();
	rc = 0;
	}
	else
	{
	init_timer(&at->timer, alarm_handler, at);
	//at->timer.expires = startup_ticks() + msec2ticks(msecs);
	install_timer(&at->timer,msecs);
	//rc = enter_alertable_wait(THREAD_WAIT_SLEEP);
	//del_timer(&timer);
	}

	return rc;
}

