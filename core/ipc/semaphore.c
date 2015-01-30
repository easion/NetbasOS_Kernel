/*
**     (R)Jicama OS
**     Program Args Setup
**     Copyright (C) 2003,2004 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/process.h>

__public void semaphore_up(semaphore_t* sem) 
{
    (*sem)++;
    return;
}


__public int semaphore_down(semaphore_t* sem)
{
    unsigned eflags;
	int sem_count;

	save_eflags(&eflags);    
    (*sem)--; /*not need wait*/
	sem_count = *sem;
	restore_eflags(eflags);
	return sem_count;
}

