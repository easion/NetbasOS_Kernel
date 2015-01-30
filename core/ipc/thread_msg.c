/*
**     (R)Jicama OS
**     messaging subsystem
**     Copyright (C) 2005 DengPingPing
*/
#include <jicama/process.h>
#include <jicama/trace.h>
#include <jicama/spin.h>
#include <jicama/msgport.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


/*
 * send message
 * used by client
 */
__public int post_thread_message(thread_t *pthread,  void* va, u32_t count)
{	
	if (pthread->msgport_id<0)
	{
		kprintf("post_thread_message error\n");
		return -1;
	}
	return msgport_send(pthread->msgport_id,0,va,count);
}

__public int get_thread_msgport(thread_t *pthread)
{
	return pthread->msgport_id;
}


/*
 * receive a message, if null return 0, else return counts
 */
__public int 
get_thread_message(thread_t *pthread, void* va,	u32_t count, time_t timeout)
{
	if (pthread->msgport_id<0)
	{
		kprintf("get_thread_message: empty on %s tid %d\n",pthread->name,pthread->tid);
		return -1;
	}	
	return msgport_pend(pthread->msgport_id,NULL,va,count,timeout);
}	



