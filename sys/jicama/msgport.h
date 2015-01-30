#ifndef __msg_port_h__
#define __msg_port_h__

#include <jicama/spin.h>
enum{
	MSGPORT_CREATE_NO=1,
	MSGPORT_CONNECT_NO=2,
	MSGPORT_DESTROY_NO=3,
	MSGPORT_WAT_NO=4,
	MSGPORT_PEND_NO=5,
	MSGPORT_SEND_NO=6,
	MSGPORT_MDELAY=7,
	MSGPORT_SLEEP=8,
	MSGPORT_WAKEUP=9,
	MSGPORT_NOTIFY_SEND_NO=10,
	MSGPORT_SET_PUBLIC,
	MSGPORT_GET_PUBLIC,
};

#define MSG_BUFSZ	(8092)
#define SEM_MAX_THREAD 10

#define insertQ(node_t) \
node_t *prev; \
 node_t *rest; \
node_t *next; 

typedef struct msg_waitbuf
{
	//insertQ(struct msg_waitbuf); 
	//int port;//
	int code;
	int size;
	u8_t buf[MSG_BUFSZ];
	TAILQ_ENTRY(msg_waitbuf) buf_lists;
}msg_buf_t;


struct retmsginfo
{
	int port;
	unsigned long addr;
};

//syscall
int api_msgport(regs_t *reg);

//utils
int create_msgport(const char *ptname, struct tcb* t);
int connect_msgport(const char *pname);
int    msgport_send(int port, int code, void* va, u32_t count);
int msgport_pend(int port, int *code,void* kbuf, u32_t count, unsigned long timeout);
int msgport_set_limit(int port,   size_t  limit);

int msgport_destroy(int port);
int msgport_wait(int port,   unsigned timeout);

/*********************************************/

struct spinlock_t;

struct thread_sem
{
	int count;
	spinlock_t spin;
	//thread_t *pthread[SEM_MAX_THREAD];
	thread_wait_t threadq;
};

/*void sem_init(struct thread_sem *s, int cnt);
void *sem_new( int cnt);
void sem_free(struct thread_sem *s);
int sem_down(struct thread_sem *s, time_t msec);
int sem_up(struct thread_sem *s);

*/

void* create_semaphore(const char *name, int flags,int init_val);
int lock_semaphore(void* semaphore);
int trylock_semaphore(void* semaphore);
int lock_semaphore_timeout(void* semaphore,time_t timeout);
int unlock_semaphore(void* semaphore);
int destroy_semaphore(void* semaphore);
int unlock_semaphore_ex(void* semaphore,int count);

void thread_msg_init(void);

#endif


