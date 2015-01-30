/* 
**  Jicama OS  
* * Copyright (C) 2003  DengPingPing     
**  All rights reserved.   
*/

#ifndef __JICAMA_TIMER_H__
#define __JICAMA_TIMER_H__

struct slink
{
  struct slink *next;
  struct slink *prev;
};

struct timer
{
  struct slink link;
  bool active;
  time_t expires;
  void (*handler)(void *arg);
  void *arg;
};

void init_timer(struct timer *timer, void (*handler)(void *arg), void *arg);
void install_timer(struct timer *timer,time_t );
int remove_timer(struct timer *timer);
int restart_timer(struct timer *timer, unsigned int expires);
int mod_timer(struct timer *timer, unsigned expires);

#define time_after(a, b)     ((long) (b) - (long) (a) < 0)
#define time_before(a, b)    time_after(b, a)

#define time_after_eq(a, b)  ((long) (a) - (long) (b) >= 0)
#define time_before_eq(a ,b) time_after_eq(b, a)

#endif
