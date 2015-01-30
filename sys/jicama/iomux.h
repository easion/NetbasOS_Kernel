

#ifndef __IO_MUX_H__
#define __IO_MUX_H__
#include <sys/queue.h>
#define IOEVT_READ     0x0001
#define IOEVT_WRITE    0x0002
#define IOEVT_ERROR    0x0004
#define IOEVT_ACCEPT   0x0008
#define IOEVT_CONNECT  0x0010
#define IOEVT_CLOSE    0x0020

#define POLLIN      0x0001    // Data may be read without blocking
#define POLLPRI     0x0002    // High priority data may be read without blocking
#define POLLOUT     0x0004    // Data may be written without blocking
#define POLLERR     0x0008    // An error has occurred (revents only)
#define POLLHUP     0x0010    // Device has been disconnected (revents only)
#define POLLNVAL    0x0020    // Invalid fd member (revents only)


#define TASK_QUEUED       1
#define TASK_EXECUTING    2

#define TASK_QUEUE_ACTIVE              1
#define TASK_QUEUE_ACTIVE_TASK_INVALID 2

typedef void (*taskproc_t)(void *arg);

struct pollfd 
{
  int fd;                     // File descriptor
  short events;               // Requested events
  short revents;              // Returned events
};

struct task
{
  taskproc_t proc;
  void *arg;
  struct task *next;
  int flags;
};

int poll(struct pollfd fds[], unsigned int nfds, int timeout);

typedef void *object_t;


struct ioobject
{ 
  struct iomux *iomux;

  LIST_ENTRY(ioobject) links;

  unsigned short events_signaled;
  unsigned short events_monitored;
  u32_t type;
  void *data;
 };

struct iomux
{
  //struct object object;
  int sem;
  int flags;
  void *rfdset;
  void *wfdset;
  void *efdset;
  LIST_HEAD(,ioobject) head;

  /*struct ioobject *ready_head;
  struct ioobject *ready_tail;

  struct ioobject *waiting_head;
  struct ioobject *waiting_tail;
  */
};

void clear_io_event(struct ioobject *iob, int events);
int set_io_event(struct ioobject *iob, int events);
void detach_ioobject(struct ioobject *iob);
void init_ioobject(struct ioobject *iob,char type[4]);

int queue_task(struct task *task, taskproc_t proc, void *arg);
void init_task(struct task *task);

#endif
