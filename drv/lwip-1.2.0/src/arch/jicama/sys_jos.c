#include "lwip/debug.h"
#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/tcpip.h"
#include "netif/loopif.h"

#include "lwip/ip_addr.h"
#include "lwip/sys.h"
#include "lwip/def.h"

typedef struct thread_info
{
	tid_t lwip_tid;
	//char* name;
	struct sys_timeouts timeouts;
}thread_info_t;


struct thread_sem;
void *sem_new( int cnt);
//void sem_free(struct thread_sem *s);
//int down(struct thread_sem *s, unsigned msec);
//int up(struct thread_sem *s);

 void  thread_unready(void *th, unsigned timeout);
 void  thread_ready(void *th);

void *current_thread();
thread_info_t tinfo[10];
static int tcur=0;

//static tid_t lwip_tid;
struct sys_timeouts timeouts;
int dll_args(struct ip_addr *a, struct ip_addr *b, struct ip_addr *c);


/*-----------------------------------------------------------------------------------*/



sys_thread_t
sys_thread_new(void (* function)(void *arg), void *arg, int prio)
{	
	thread_info_t *ti = &tinfo[tcur];

	memset(ti,0,sizeof(thread_info_t));

	LWIP_ASSERT("sys_jos_thread_new(): mbox == NULL", function!=NULL);
	//ti->name = "unknow";
	ti->timeouts.next = NULL;
	ti->lwip_tid = new_kernel_thread(NULL, function, arg);
	++tcur;
	return ti->lwip_tid;
}

/*-----------------------------------------------------------------------------------*/
sys_mbox_t
sys_jos_mbox_new(char * m_name)
{
	sys_mbox_t msg=(void *)mm_malloc(sizeof(struct sys_mbox));

	if (msg == NULL)
	{
		return (sys_mbox_t)SYS_MBOX_NULL;
	}


	msg->msgport = create_msgport(m_name, (void* )current_thread());
	//printf("dpp create_msgport ok\n");

#ifdef JICAMA_DEBUG
	 msg->magic = DEBUG_J_MAGIC;
	if(m_name)
	 msg->name = m_name;
else
	 msg->name = "null";
#endif

	if (msg->msgport < 0)
	{
		kprintf("failed create port for %s\n", m_name);
		goto err1;
	}
  //printf("create_msgport returned msg\n");
  return msg;
err2:
	msgport_destroy(msg->msgport);
err1:
	mm_free(msg, sizeof(struct sys_mbox));
	return  (sys_mbox_t)SYS_MBOX_NULL;

}

sys_mbox_t
sys_mbox_new(void)
{
	return sys_jos_mbox_new(NULL);
}


/*-----------------------------------------------------------------------------------*/
void
sys_mbox_free(sys_mbox_t mbox)
{
	LWIP_ASSERT("sys_mbox_free(): mbox == NULL", mbox!=SYS_MBOX_NULL);
	msgport_destroy(mbox->msgport);
	mm_free(mbox, sizeof(struct sys_mbox));
  return;
}

/*-----------------------------------------------------------------------------------*/
#include <drv/spin.h>

CREATE_SPINLOCK( mdelay_sem );

void
sys_mbox_post(sys_mbox_t mbox, void *data)
{
	unsigned long addr ;
	LWIP_ASSERT("sys_jos_mbox_post(): mbox == NULL", mbox!=SYS_MBOX_NULL);

#ifdef JICAMA_DEBUG
	 if(mbox->magic!=DEBUG_J_MAGIC){
		panic("sys_mbox_post: magic error\n");
		return;
	}
#endif

	if (mbox->msgport < 0){
		panic("msgbox %s: error: mbox->msgport\n", mbox->name);
	}

	if (!data)
	{
		//syslog(4,"sys_mbox_post: data ptr NULL\n");
	}

	spin_lock( &mdelay_sem );
	addr = (unsigned long)data;	
	spin_unlock( &mdelay_sem );
	msgport_send(mbox->msgport, (void*)&addr, 
		sizeof(unsigned long),0);
	return;
}


/*-----------------------------------------------------------------------------------*/
u32_t 
sys_arch_mbox_fetch(sys_mbox_t mbox, void **data, u32_t timeout)
{
	int len;
	int cli;
	unsigned long addr = 0;
 	char *name;

	LWIP_ASSERT("sys_arch_mbox_fetch(): mbox == NULL", mbox!=SYS_MBOX_NULL);
	
#ifdef JICAMA_DEBUG
	 if(mbox->magic!=DEBUG_J_MAGIC){
		panic("magic error\n");
		return SYS_ARCH_TIMEOUT;
	}
#endif

	len =	msgport_pend(mbox->msgport, 
		&addr, sizeof(unsigned long), timeout*100);
	if (len <= 0){		
		//syslog(4,"SYS_ARCH_TIMEOUT returned\n");
		return SYS_ARCH_TIMEOUT;
	}

	if (data){
		*data = addr;
	}
	//syslog(4,"data returned\n");
	return 1;
}

/*-----------------------------------------------------------------------------------*/
void
sys_sem_free(sys_sem_t sem)
{
	sem_free(sem);
  return;
}


/*-----------------------------------------------------------------------------------*/
sys_sem_t
sys_sem_new(u8_t count)
{
	sys_sem_t s = sem_new(count);
	if(!s)kprintf("new sem call: null\n");
  return s;
}

/*-----------------------------------------------------------------------------------*/
u32_t
sys_arch_sem_wait(sys_sem_t sem, u32_t timeout)
{
	int ret;
	LWIP_ASSERT("sys_arch_sem_wait(): sem == NULL", sem!=0);

	if (timeout&&timeout<10)
	{
		timeout=10;
	}

	ret = sem_down(sem, timeout);
	if (ret)
	{
		//超过时间，退出了
		return SYS_ARCH_TIMEOUT;
	}
  return 1;
}
/*-----------------------------------------------------------------------------------*/
void
sys_sem_signal(sys_sem_t sem)
{
	LWIP_ASSERT("sys_arch_sem_wait(): sem == NULL", sem!=0);
	up(sem);
  return;
}

/*-----------------------------------------------------------------------------------*/
void
sys_init(void)
{
	tcur=0;
  timeouts.next = NULL;
  return;
}
/*-----------------------------------------------------------------------------------*/

void default_eth_halder(void (*_isr)());

struct sys_timeouts *
sys_arch_timeouts(void)
{
	thread_info_t *ti = &tinfo[0];
	int i=0;

	while (i<tcur)
	{
		if (ti->lwip_tid == current_thread_id(0))
		{
			return &ti->timeouts;
		}
		ti++;
		i++;
	}
	//kprintf("sys_arch_timeouts default\n");
	return &timeouts;
}
