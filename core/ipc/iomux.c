
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/ipc.h>
#include <jicama/devices.h>
#include <jicama/msgport.h>
#include <jicama/iomux.h>
#include <string.h>
#include <assert.h>

static sem_t ioobj_sem;


// Release lock on object
//

static int orel(void* hobj)
{
  //struct object *o = (struct object *) hobj;

  /*if (--o->lock_count == 0 && o->handle_count == 0) 
    return destroy_object(o);
  else
    return 0;
	*/
}



static void release_waiting_threads(struct iomux *iomux)
{
	ASSERT(iomux->sem>=0);

#if 0

 // struct waitblock *wb;
  //struct waitblock *wb_next;
  struct ioobject *iob;

  // Dispatch all ready I/O objects to all ready waiting threads
  //wb = iomux->object.waitlist_head;
  iob = iomux->ready_head;
 // while (iob)
  {
    //wb_next = wb->next_wait;

    //if (thread_ready_to_run(wb->thread))
    //{
      // Overwrite waitkey for thread with context for object
      //wb->thread->waitkey = iob->context;

      // Remove object from iomux
      detach_ioobject(iob);

      // Mark thread ready
      //release_thread(wb->thread);

      iob = iomux->ready_head;
    //}

   // wb = wb_next;
  }
#endif
	//kprintf("release_waiting_threads called\n");
	unlock_semaphore(iomux->sem);
}


static int init_iomux(struct iomux *iomux, int flags)
{
  iomux->sem = create_semaphore("iomux",0,0);
  if (iomux->sem<=0)
  {
	  return -1;
  }
  iomux->flags = flags;
  LIST_INIT(&iomux->head);
  return 0;
}

void detach_ioobject(struct ioobject *iob);

static int close_iomux(struct iomux *iomux)
{
  struct ioobject *iob;
  void *nxt;

  //kprintf("close_iomux called\n");
	ASSERT(iomux->sem>=0);

	LOCK_SCHED(ioobj_sem);

  LIST_FOREACH_SAFE(iob,&iomux->head,links,nxt){
  //LIST_FOREACH(iob,&iomux->head,links){
	  detach_ioobject(iob);
  }
  UNLOCK_SCHED(ioobj_sem);
  destroy_semaphore(iomux->sem);
  iomux->sem = -1;
  return 0;
}

static int queue_ioobject(struct iomux *iomux, struct ioobject *iob, int events)
{
	int found=0;
  if (!events) return EINVAL;

  if (iob->iomux) 
  {
    // Do not allow already attached object to attach to another iomux
    if (iob->iomux != iomux) {
		kprintf("queue_ioobject  error(%p,%p,type %s)\n",iob->iomux, iomux,iob->type);
		return EPERM;
	}

    // Update the event monitoring mask
    events |= iob->events_monitored;

    // Detach object, it will be inserted in the appropriate queue further down 
    detach_ioobject(iob);
  }

  LOCK_SCHED(ioobj_sem);

  iob->iomux = iomux;
  iob->events_monitored = events;
  //iob->context = context;
  
  // If some signaled event is monitored insert in ready queue else in waiting queue
  if (iob->events_monitored & iob->events_signaled)
  {
	  kprintf("queue_ioobject succ\n");
    //iomux->object.signaled = 1;
	//LIST_INSERT_HEAD(&iomux->head, iob, links);  
	found++;
  }
  else
  {
	  LIST_INSERT_HEAD(&iomux->head, iob, links);   
  }

  UNLOCK_SCHED(ioobj_sem);

  // If iomux is signaled try to dispatch ready objects to waiting threads
  //if (iob->object.signaled) 
	//  release_waiting_threads(iomux);

  return found;
}


/***************************************/

//for closesocket()
void detach_ioobject(struct ioobject *iob)
{

  if (iob->iomux)
  {
	  LOCK_SCHED(ioobj_sem);

    //iob->next = iob->prev = NULL;

	iob->events_monitored = 0;  
  LIST_REMOVE(iob, links);
    iob->iomux = NULL;
	UNLOCK_SCHED(ioobj_sem);
  }
}

/***************************************/

int set_io_event(struct ioobject *iob, int events)
{
	int err=0;
  struct iomux *iomux;

  if (!iob)
	  return err;

 // kprintf("set_io_event %d %p\n",events, iob);
	LOCK_SCHED(ioobj_sem);
  iomux = iob->iomux;

  //
  // If the following is true we must move the object to the ready queue and signal it:
  //  1) object is attached to an iomux
  //  2) object is on the waiting queue
  //  3) the new event(s) are being monitored.

  //kprintf("set_io_event() %d\n",events);

  if ( (iob->events_monitored & iob->events_signaled) == 0 &&(iob->events_monitored & events) != 0)
  {
    // Update signaled events
    iob->events_signaled |= events;

    // Remove object from waiting queue
	

    // Signal iomux
    //iomux->object.signaled = 1;

	//kprintf("[ PID:%X ]called release_waiting_threads\n",current_thread()->tid);

    // Try to dispatch ready objects to waiting threads
	if(iomux)
	  release_waiting_threads(iomux);
	else{
		kprintf("[ PID:%X ]called error release_waiting_threads\n",current_thread()->tid);
	}
	err = 1;
  }
  else
  {
	  if(!iomux){
	 trace("[ PID:%X ]error update  event %x events_signaled %x events_monitored %x\n",
		  events,iob->events_signaled,iob->events_monitored);
	  }
    // Just update the signaled event(s) for object
    iob->events_signaled |= events;
	//release_waiting_threads(iomux);
  }

  UNLOCK_SCHED(ioobj_sem);
  return err;
}

void clear_io_event(struct ioobject *iob, int events)
{
	LOCK_SCHED(ioobj_sem);
  //kprintf("clear_io_event called %d %p\n",events,iob);
  // Clear events
  iob->events_signaled &= ~events;
  UNLOCK_SCHED(ioobj_sem);
}



void init_ioobject(struct ioobject *iob,char type[4])
{
  //init_object(&iob->object, type);
  memset(iob,0,sizeof(struct ioobject));
  memcpy(iob->type, type,4);
  iob->data = NULL;
  iob->iomux = NULL;
  //iob->context = 0;
 // iob->next = iob->prev = NULL;
  iob->events_signaled = iob->events_monitored = 0;
}

static int check_fds(int maxfdp1,fd_set *fds, int eventmask)
{
	unsigned int n;
	int matches;
	struct ioobject *iob;
	int i;
	fd_set lreadset;

	if (!fds) {
		//kprintf("check_fds: empty,return\n");
		return 0;
	}

    /* Go through each socket in each list to count number of sockets which
       currently match */

	FD_ZERO(&lreadset);
	matches = 0;
    for(i = 0; i < maxfdp1; i++)
    {
		
        if (FD_ISSET(i, fds))
        {
			//if (i==9)
			//{
			//	kprintf("found mouse dev\n");
			//}
		    iob = (struct ioobject *) fs_get_prvi_data(i,NULL);
			if (!iob){
				kprintf("check_fds:check fd %d  empty\n",i);
				return EBADF;
			}


			if (iob->events_signaled & eventmask){
			//kprintf("check_fds: go fd %d %p iob->events_signaled=%x succ\n",i,iob,iob->events_signaled);
				FD_SET(i, &lreadset);
				matches++;
			}
		}
	}
 
  *fds = lreadset;
  return matches;
}

static int add_fds_to_iomux(struct iomux *iomux,int nfd,const fd_set *fds, int eventmask)
{
  unsigned int n;
  struct ioobject *iob;
  int rc;
  int count=0;

  if (!fds) return 0;

  for (n = 0; n < nfd; n++)
  {
	  if (!FD_ISSET(n, fds))continue;
    iob = (struct ioobject *) fs_get_prvi_data(n,NULL);
    if (!iob){
		kprintf("add_fds_to_iomux:bad fd %d\n",n);
		return EBADF;
	}

    rc = queue_ioobject(iomux, iob, eventmask);
    if (rc < 0){
		return rc;
	}
	else{
		count+=rc;
	}

    orel(iob);
  }

  return count;
}

static int check_select(int nfds,fd_set *readset, fd_set *writeset, fd_set *exceptset)
{
  int numread;
  int numwrite;
  int numexcept;

  numread = check_fds(nfds,readset, IOEVT_READ | IOEVT_ACCEPT | IOEVT_CLOSE);
  if (numread < 0) return numread;

  numwrite = check_fds(nfds,writeset, IOEVT_WRITE | IOEVT_CONNECT);
  if (numwrite < 0) return numwrite;

  numexcept = check_fds(nfds,exceptset, IOEVT_ERROR);
  if (numexcept < 0) return numexcept;

  if (numread != 0 || numwrite != 0 || numexcept != 0)
  {
    return numread + numwrite + numexcept;
  }

  return 0;
}

inline int msleep(int msec)
{
	if (msec==0)
	{
		return 0;
	}
	thread_wait(current_thread(),msec);
	return 0;
}


int vfs_select(int nfds, fd_set *readset, fd_set *writeset, 
	fd_set *exceptset, struct timeval *timeout)
{
	int rc;
	unsigned int tmo;
	struct iomux s_iomux;
	fd_set lreadset, lwriteset, lexceptset;

   if (readset)
        lreadset = *readset;
    else
        FD_ZERO(&lreadset);
    if (writeset)
        lwriteset = *writeset;
    else
        FD_ZERO(&lwriteset);
    if (exceptset)
        lexceptset = *exceptset;
    else
        FD_ZERO(&lexceptset);

  rc = check_select(nfds,&lreadset, &lwriteset, &lexceptset);

  //kprintf("check_select %d ok\n",rc);
  if (rc > 0){
	  //kprintf("select(): %d fds is ready\n",rc);
	  goto got_result;
  }
  else if (rc<0)
  {
	  return rc;
  }

  if (!timeout) {
    tmo = INFINITE;
  }
  else{
    tmo = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
	  if (tmo == 0) return 0;
  }


  if (timeout && !readset && !writeset && !exceptset) {
	  kprintf("init_iomux error\n");
	  return msleep(tmo);
  }

  rc = init_iomux(&s_iomux, 0);
  if (rc<0)
  {
	  kprintf("init_iomux error\n");
	  return -1;
  }

   //²»»áÐÞ¸Äreadfds
  rc = add_fds_to_iomux(&s_iomux,nfds, readset, IOEVT_READ | IOEVT_ACCEPT | IOEVT_CLOSE);
  if (rc < 0)
  {
    close_iomux(&s_iomux);
    return EBADF;
  }
  else  if (rc>0){
	  goto got_result0;
	}

  rc = add_fds_to_iomux(&s_iomux,nfds, writeset, IOEVT_WRITE | IOEVT_CONNECT);
  if (rc < 0)
  {
    close_iomux(&s_iomux);
    return EBADF;
  }
  else  if (rc>0){
	  goto got_result0;
	}


  rc = add_fds_to_iomux(&s_iomux,nfds, exceptset, IOEVT_ERROR);
  if (rc < 0)
  {
    close_iomux(&s_iomux);
    return EBADF;
  }
  else  if (rc>0){
	  goto got_result0;
	}

  ASSERT(s_iomux.sem);

  //kprintf("select go sleep %d\n",tmo);



  rc = lock_semaphore_timeout(s_iomux.sem,tmo);
  if (rc < 0)
  {
	//kprintf("%s() lock_semaphore_timeout %d %d\n",__FUNCTION__,tmo,rc);

    //if (rc == -ETIMEOUT) rc = 0;
	if (readset)
		FD_ZERO(readset);
	if (writeset)
		FD_ZERO(writeset);
	if (exceptset)
		FD_ZERO(exceptset);

	close_iomux(&s_iomux);

    return 0;
  }

got_result0:
   if (readset)
		lreadset = *readset;
	else
		FD_ZERO(&lreadset);

	if (writeset)
		lwriteset = *writeset;
	else
		FD_ZERO(&lwriteset);

	if (exceptset)
		lexceptset = *exceptset;
	else
		FD_ZERO(&lexceptset);

  rc = check_select(nfds,&lreadset, &lwriteset, &lexceptset);  
  close_iomux(&s_iomux);
  

got_result:
   if (readset)
        *readset = lreadset;
    if (writeset)
        *writeset = lwriteset;
    if (exceptset)
        *exceptset = lexceptset;

  return rc;
}


static int check_poll(struct pollfd fds[], unsigned int nfds)
{
  struct ioobject *iob;
  unsigned int n;
  int ready;
  int revents;
  int mask;

  ready = 0;
  for (n = 0; n < nfds; n++)
  {
    revents = 0;
    if (fds[n].fd >= 0)
    {
      iob = (struct ioobject *) fs_get_prvi_data(fds[n].fd,NULL);
      if (!iob)
      {
	revents = POLLNVAL;
	if (iob) orel(iob);
      }
      else
      {
	mask = IOEVT_ERROR | IOEVT_CLOSE;
	if (fds[n].events & POLLIN) mask |= IOEVT_READ | IOEVT_ACCEPT;
	if (fds[n].events & POLLOUT) mask |= IOEVT_WRITE | IOEVT_CONNECT;

	mask &= iob->events_signaled;
	if (mask != 0)
	{
          if (mask & (IOEVT_READ | IOEVT_ACCEPT)) revents |= POLLIN;
          if (mask & (IOEVT_WRITE | IOEVT_CONNECT)) revents |= POLLOUT;
	  if (mask & IOEVT_ERROR) revents |= POLLERR;
	  if (mask & IOEVT_CLOSE) revents |= POLLHUP;
	}
      }

      orel(iob);
    }
    fds[n].revents = revents;
    if (revents != 0) ready++;
  }

  return ready;
}

static int add_fd_to_iomux(struct iomux *iomux, int fd, int events)
{
  struct ioobject *iob;
  int rc;
  int mask;

  if (fd < 0) return 0;
  iob = (struct ioobject *) fs_get_prvi_data(fd,NULL);
  if (!iob) return EBADF;

  mask = IOEVT_ERROR | IOEVT_CLOSE;
  if (events & POLLIN) mask |= IOEVT_READ | IOEVT_ACCEPT;
  if (events & POLLOUT) mask |= IOEVT_WRITE | IOEVT_CONNECT;

  rc = queue_ioobject(iomux, iob, mask);
  if (rc < 0) return rc;

  orel(iob);
  return 0;
}

int poll(struct pollfd fds[], unsigned int nfds, int timeout)
{
  struct iomux iomux;
  int rc;
  unsigned int n;

  if (nfds == 0) 
	  return msleep(timeout);

  if (!fds)
	  return EINVAL;

  rc = check_poll(fds, nfds);
  if (rc > 0) return rc;
  if (timeout == 0) return 0;

  rc = init_iomux(&iomux, 0);
  if (rc<0)
  {
	  kprintf("init_iomux error\n");
	  return -1;
  }

  for (n = 0; n < nfds; n++)
  {
    rc = add_fd_to_iomux(&iomux, fds[n].fd, fds[n].events);
    if (rc < 0)
    {
      close_iomux(&iomux);
      return rc;
    }
  }

  //kprintf("down on %s\n",__FUNCTION__);
  rc = lock_semaphore_timeout(iomux.sem,timeout);
  if (rc < 0)
  {
    close_iomux(&iomux);
    //if (rc == -ETIMEOUT) rc = 0;
    return 0;
  }

  rc = check_poll(fds, nfds);
  close_iomux(&iomux);
  return rc;
}
#if 0

static void dump_iomux(struct iomux *iomux)
{
  struct ioobject *iob;

  //kprintf("iomux %c", iomux->object.signaled ? '+' : '-');
  if (iomux->ready_head)
  {
    kprintf(" r:");
    iob = iomux->ready_head;
    while (iob)
    {
      kprintf("%d(%x,%x) ", ((((unsigned long) iob) >> 2) & 0xFF), 
		  iob->events_monitored, iob->events_signaled);
      iob = iob->next;
    }
  }

  if (iomux->waiting_head)
  {
    kprintf(" w:");
    iob = iomux->waiting_head;
    while (iob)
    {
      kprintf("%d(%x,%x) ", ((((unsigned long) iob) >> 2) & 0xFF), 
		  iob->events_monitored, iob->events_signaled);
      iob = iob->next;
    }
  }
  kprintf("\n");
}
#endif


