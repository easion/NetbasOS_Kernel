/* 
** Jicama OS Loadable Kernel Modules Test
** 2005-3-5
*/
#include <drv/drv.h>
static	tid_t id1;
static	tid_t id2;

void thread1(void *arg)
{
	kprintf("thread1 run  ...\n");

	while (1)
	{
		//kprintf("thread1 run loops  ...\n");
		kprintf("%s sleep 2 sec...\n",arg);
		thread_wait(current_thread(), 2000);
		schedule();
	}
}


void thread_test()
{
	id = new_kernel_thread("thread1", thread1,"[thread1 args]");
	kprintf("create thread %d ok\n",id);

	id = new_kernel_thread("thread2", thread1,"[thread2 args]");
	kprintf("create thread %d ok\n",id);
}


/*dll entry*/
int dll_main(char **argv)
{
	puts("Hello world, module Runing!\n");
	thread_test();	

	return 0;
}


int dll_destroy()
{
	kprintf("dll_destroy called!\n");

	thread_exit_byid(id1,NULL);
	thread_exit_byid(id2,NULL);
	return 0;
}

