
// ---------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//----------------------------------------------------------------------------------

#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <jicama/syscall.h>
#include <jicama/spin.h>
#include <assert.h>

__asmlink int 		entry_proc (u16_t tss_selector);
__asmlink time_t startup_ticks();
__asmlink time_t get_unix_sec();

CREATE_SPINLOCK( schedule_sem_lock );


//
 __local int volatile isneed_schdule=0;

/*
**
*/
__public void lock_schedule(int *val)
{
	unsigned cur_flag;

	save_eflags(&cur_flag);
	if(val){
		/*save it*/
		*val = isneed_schdule;
	}
	isneed_schdule = FALSE;
	restore_eflags(cur_flag);
}

__public void set_schedule(int val)
{
	unsigned cur_flag;

	save_eflags(&cur_flag);
	isneed_schdule=val;
	restore_eflags(cur_flag);
}

/*
**
*/
__public void unlock_schedule()
{
	set_schedule(TRUE);	
}

/*
**
*/
__public bool scheduleable()
{
	bool ret;
	unsigned cur_flag;

	save_eflags(&cur_flag);
	ret=isneed_schdule;
	restore_eflags(cur_flag);

	/*if (ret)
	{
		kprintf("scheduleable state ok\n");
	}*/
	return ret;
}

