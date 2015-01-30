
#include <jicama/process.h>

__public int  arch_thread_create2(thread_t* th_new,void (*fn)(void *),
	void *args,  unsigned stack,  proc_t *rp)
{
	th_new->reg->content[0] = (unsigned long)th_new;//(&task[pid+1]);
	th_new->reg->content[1] = 0x5f;   	/*cpsr*/
	th_new->reg->content[2] = 0x100000-1024; /*usr/sys模式堆栈*/
	th_new->reg->content[3] = SVCMODE;	/*svc模式*/
	th_new->reg->content[18]= (unsigned long)fn;	/*pc*/
}


