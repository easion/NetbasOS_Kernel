/* 
**  Jicama OS  
* * Copyright (C) 2003  DengPingPing     
**  All rights reserved.   
*/

#ifndef __JICAMA_TIMER_H__
#define __JICAMA_TIMER_H__
#ifdef __cplusplus
extern "C" {
#endif
struct slink
{
  struct slink *next;
  struct slink *prev;
};

typedef struct callout
{
  struct slink link;
  bool active;
  time_t expires;
  void (*handler)(void *arg);
  void *arg;
}krnl_timer_t;

void init_timer(krnl_timer_t *timer, void (*handler)(void *arg), void *arg);
void install_timer(krnl_timer_t *timer,time_t t);
int remove_timer(krnl_timer_t *timer);
int restart_timer(krnl_timer_t *timer, unsigned int expires);

#ifdef __cplusplus
}
#endif

#endif
