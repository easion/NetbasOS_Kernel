/*
**     (R)Jicama OS
**     messaging subsystem
**     Copyright (C) 2005 DengPingPing
*/
#include <jicama/process.h>
#include <jicama/trace.h>
#include <jicama/msgport.h>
#include <jicama/spin.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

//#define MAX_MSG		(100)
#define MAX_PORTNAME_LEN		(20)

#define MSGPORT_LIMIT_NUM 1024
static sem_t thread_sock_sem;

static int last_port=0x12;
typedef struct msgwait {
	size_t limit,current;
	thread_wait_t threadq;
	char port_name[MAX_PORTNAME_LEN];
	//queue_t *port_que; //
	TAILQ_HEAD(tailhead, msg_waitbuf) qh;
	LIST_ENTRY(msgwait) lists;
	port_id_t port_no;
}msg_cb_t;
__inline msg_cb_t* find_msgport_byid(port_id_t no);

static volatile  LIST_HEAD(, msgwait) mcb_qh;


/*
 * protos
 */
//__local msg_cb_t* wait_pool = (msg_cb_t* )0;

int msg_proc(char *buf, int len)
{
	int cnt=0,c, idx=0;
	msg_cb_t* mcb;

	c = sprintf(buf+cnt,"Msgport dump:\n", 
		"111");
	cnt += c;

	LOCK_SCHED(thread_sock_sem);	

	LIST_FOREACH(mcb,&mcb_qh,lists)
	{
		idx++;
		c = snprintf(buf+cnt,MAX_PORTNAME_LEN,"%d\t%s\t%03d\n\r",
			idx,mcb->port_name,mcb->port_no);
		cnt += c;
	}

	UNLOCK_SCHED(thread_sock_sem);	

	//int ifree = 0;
/*
	idx = 0;
	while(idx < MAX_MSG) {
		if(mcb->port_que == NULL) {
			ifree++;
			mcb++;
			idx++;
			continue;
		}
		c = snprintf(buf+cnt,MAX_PORTNAME_LEN,"%s\t\tPort%d\n",
			mcb->port_name,idx);
		cnt += c;
		mcb++;
		idx++;
	}

	c = sprintf(buf+cnt,"Msgport: Num%d,Free%d,NameLength%d\n", 
		MAX_MSG, ifree,MAX_PORTNAME_LEN);
	cnt += c;*/
	return cnt;
}


#define  PORT_COUNT 8

static port_id_t g_anPortTable[ PORT_COUNT ];

/*
 * msgport init
 */
__public void thread_msg_init(void)
{
  int i;
	LIST_INIT(&mcb_qh);
	last_port=0x12;

  for ( i = 0; i < PORT_COUNT; i++ ) {
    g_anPortTable[ i ] = -1;
  }
}

__inline msg_cb_t* find_msgport_byid(port_id_t no)
{
	msg_cb_t*mcb;

	LOCK_SCHED(thread_sock_sem);	

	LIST_FOREACH(mcb,&mcb_qh,lists)
	{
		if (mcb->port_no == no)
		{
			UNLOCK_SCHED(thread_sock_sem);	
			return mcb;
		}
	}

	UNLOCK_SCHED(thread_sock_sem);	
	return NULL;
}


__public int msgport_set_limit(port_id_t port,   size_t  limit)
{
	msg_cb_t* mcb=find_msgport_byid(port);
	if (!mcb)
	{
		return -1;
	}
	mcb->limit=limit;
	return 0;
}


/*
 * msgport exist
 */
__public port_id_t find_msgport_byname(const char *name)
{
	msg_cb_t*mcb;

	LOCK_SCHED(thread_sock_sem);	

	LIST_FOREACH(mcb,&mcb_qh,lists)
	{
		if (strcmp(mcb->port_name,name) == 0)
		{
			UNLOCK_SCHED(thread_sock_sem);	
			return mcb;
		}
	}

	UNLOCK_SCHED(thread_sock_sem);	
	return NULL;	
}

/*
 * msgport destroy
 */
__public port_id_t create_msgport(const char *pname, thread_t *pthread)
{
	msg_cb_t* mcb;
	char *name=pname;
	char tmpbuf[MAX_PORTNAME_LEN];

	if (!pthread)
	{
		return -2;
	}

	if (!name)
	{
		name = tmpbuf;
		snprintf(tmpbuf,MAX_PORTNAME_LEN, "%s_%d", 
					pthread->name,last_port);
	}

	if (find_msgport_byname(name))
	{
		kprintf("msgport pot exist here\n");
		return -1;
	}

	mcb = kcalloc(sizeof(msg_cb_t));

	if (!mcb)
	{
		return -3;
	}

	mcb->port_no = last_port++;
	mcb->current=0;
	mcb->limit=MSGPORT_LIMIT_NUM;
	strncpy(mcb->port_name, name, MAX_PORTNAME_LEN);
	thread_waitq_init(&(mcb->threadq));

	LOCK_SCHED(thread_sock_sem);	

	TAILQ_INIT(&mcb->qh);

	LIST_INSERT_HEAD(&mcb_qh,mcb,lists);
	UNLOCK_SCHED(thread_sock_sem);	
	
	return mcb->port_no;
}

/*
 * msgport destroy
 */

__public int msgport_destroy(port_id_t port)
{
	int queue_is_null;
	msg_cb_t* mcb = find_msgport_byid(port);
	msg_buf_t *vptr;

	CREATE_SPINLOCK( des_msg_sem );

	if (!mcb)
	{
		return -1;
	}
	

	spin_lock(&des_msg_sem);

	LIST_REMOVE(mcb,lists);

	for (vptr = TAILQ_FIRST(&mcb->qh); vptr; vptr = TAILQ_FIRST(&mcb->qh)) {
                TAILQ_REMOVE(&mcb->qh, vptr, buf_lists);
				kfree(vptr);
	}

	kfree(mcb);

	/*queue_is_null = TAILQ_EMPTY(&mcb->qh);

	if (!queue_is_null)	{
		//如果还存在没有取出的数据
		msg_buf_t *mbuf;
		mbuf =(msg_buf_t *)queue_remove(mcb->port_que);
		syslog(4, "msgport_destroy():oops, msgport %d has %d msgs\n",
			port, queue_is_null, mbuf->size);
	}
	queue_release(mcb->port_que);
	mcb->port_que = NULL;
	memset(mcb->port_name,0, MAX_PORTNAME_LEN);*/
	spin_unlock(&des_msg_sem);
	return 0;
}



/*
 * connects to message, returns port number
 * used by client
 */
__public int connect_msgport(const char *pname)
{
	msg_cb_t* mcb = find_msgport_byname(pname);

	//记录连接口
	return(mcb->port_no);
}



__public port_id_t msgport_wait(port_id_t port,   unsigned timeout)
{
	unsigned t = timeout;
	msg_cb_t* mcb = find_msgport_byid(port);
	
	/*
	 * block server and mark it wait status
	 */
	thread_sleep_on(&(mcb->threadq));
	return 0;
}

/*
 * send message
 * used by client
 */
__public int msgport_send(port_id_t port, int code, void* va, u32_t count)
{	
	msg_cb_t* mcb;
	msg_buf_t *mbptr; 
	unsigned cur_flag;
	CREATE_SPINLOCK( send_sem );

	if (count>MSG_BUFSZ)
	{
		syslog(3,"too big buffer\n");
		return -1;
	}

	mcb =  find_msgport_byid(port);

	/*
	 * some sanity checks
	 */
	if(!mcb ) {
		kprintf("msgport_send(): invalid port number %d\n",port);
		return (0);
	}

	if(mcb->current>=mcb->limit){
		kprintf("msgport_send: error limit %d %d\n",mcb->current,mcb->limit);
		thread_wakeup(&(mcb->threadq));
		return -1;
	}

	mbptr =   kcalloc(sizeof(msg_buf_t));

	if (mbptr == NULL){
		syslog(3,"%s send: no buffer found @ port%d\n", current_thread()->name, port);
		//没有内存空间了
		return -1;
	}


	save_eflags(&cur_flag);	

	spin_lock(&send_sem);

	mbptr->code = code;
	mbptr->size = count;
	mcb->current++;

	memcpy(mbptr->buf, va, count);
	TAILQ_INSERT_TAIL(&mcb->qh, mbptr, buf_lists);

	
	spin_unlock(&send_sem);
	restore_eflags(cur_flag);

	/*
	 * unblock server, give it message id
	 */
	// kprintf("thread_wakeup %p port=%d\n",&(mcb->threadq),port);
	thread_wakeup(&(mcb->threadq));
	return (count);
}


__local int msgport_pend2(msg_cb_t* mcb,  time_t timeout)
{
	int ret;


	do{

		if(!TAILQ_EMPTY(&mcb->qh)){
			break;
		}


		if (timeout == (unsigned long)0)
		{
		//syslog(3,"queue wait %d returned, no wait\n",t);
			return -1;
		}

		
		//kprintf("thread_sleep_on go %p %d\n",&(mcb->threadq),mcb->port_no);
		ret = thread_sleep_on_timeout(&(mcb->threadq),timeout);

		//kprintf("thread_sleep_on succ\n");

		
		if(ret){
		//kprintf("msg timeout, queue null\n");
			return -1;
		}
			
		
		
	}while(0);

	if(!TAILQ_EMPTY(&mcb->qh)){
			return 1;
	}
	
	return (0);
}	



/*
 * receive a message, if null return 0, else return counts
 */
__public int msgport_pend(port_id_t port, int *code,void* kbuf,	u32_t count, unsigned long timeout)
{
	int nbytes;
	msg_buf_t *mbptr = NULL ;
	msg_cb_t* mcb;
	unsigned cur_flag;

	if(kbuf == NULL) {
		kprintf("wait_msg(): invalid buffer\nbytes");
		return (0);
	}


	mcb =  find_msgport_byid(port);

	if(!mcb) {
		kprintf("wait_msg(): invalid port number\n");
		return (0);
	}

	nbytes = msgport_pend2(mcb, timeout);		
	if(nbytes <= 0) {	
		//syslog(4,"time out\n");
		return -1;
	}

	save_eflags(&cur_flag);	

	//kprintf("msgport_pend2 succ...\n");
	mbptr = TAILQ_FIRST(&mcb->qh);
	if (!mbptr)
	{
		restore_eflags(cur_flag);
	kprintf("msgport_pend2 error...\n");
	return -1;
	}

	mcb->current--;
	TAILQ_REMOVE(&mcb->qh, mbptr, buf_lists);	
	ASSERT(mbptr);
	nbytes = MIN( mbptr->size ,count);

	
	/*
	 * copy to dest address
	 */
	if(nbytes>0){
		memcpy((char*)kbuf, mbptr->buf, nbytes);
	}

	if(code)
	*code = mbptr->code;
	restore_eflags(cur_flag);

	/*
	 * free receiving buffer params
	 */	
	kfree(mbptr);

	return (nbytes);
}	


port_id_t msgport_get_public_port( int nId, int nFlags,time_t waitsecs )
	{
  port_id_t nPort;
  int has_timeout = waitsecs?1:0;

  if ( __unlikely( nId < 0 ) || __unlikely( nId >= PORT_COUNT ) ) {
    return -EINVAL;
  }

  //LOCK( g_nPortTableLock );

  if ( nFlags ) {
    while ( g_anPortTable[ nId ] == -1 ) {
      //UNLOCK( g_nPortTableLock );
      thread_wait( current_thread(),1000 );

	  if (has_timeout  && !waitsecs--)
	  {
		  break;
	  }
     // LOCK( g_nPortTableLock );
    }
  }

  nPort = g_anPortTable[ nId ];

  //UNLOCK( g_nPortTableLock );

  return nPort;
}

int msgport_set_public_port( int nId, port_id_t nPort ) 
{
  if ( __unlikely( nId < 0 ) || __unlikely( nId >= PORT_COUNT ) ) {
    return EINVAL;
  }

 // LOCK( g_nPortTableLock );
  g_anPortTable[ nId ] = nPort;
  //UNLOCK( g_nPortTableLock );

  return 0;
}
