
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/ipc.h>

/*
**�ź�����ʼ������
*/
__public void Sinit(struct semaphore_s *s, int cnt)
{
	//��ʼ���������
	thread_waitq_init(&s->sem_q);
	//�ж��ٿ��õ���Դ
	s->count = cnt;	
}

