
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/ipc.h>

/*
**信号量初始化函数
*/
__public void Sinit(struct semaphore_s *s, int cnt)
{
	//初始化任务队列
	thread_waitq_init(&s->sem_q);
	//有多少可用的资源
	s->count = cnt;	
}

