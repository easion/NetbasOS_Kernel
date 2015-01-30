/*
**     (R)Jicama OS
**     Main Function
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <jicama/grub.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <string.h>
#include <errno.h>

#include "core.h"
char *init_cmd=(char *)0;

void arch_init3();
void arch_init2();
static void task_loops();


/*
**
*/
void ps_data_type()
{
	kprintf("long size:%d\t", sizeof(long));
	kprintf("long long size:%d\t", sizeof(long long));
	kprintf("unsigned long size:%d\t", sizeof(unsigned long));
	kprintf("unsigned size:%d\t", sizeof(unsigned));
	kprintf("int size:%d\t", sizeof(int));
	kprintf("short size:%d\t", sizeof(short));
	kprintf("char size:%d\t", sizeof(char));
}

/*
**systen main entry
*/
__public int core_start( void* args)
{
	unsigned long mem_size,base_mm;

	log_init();
	tty_init();  //must be first on console_init

	memset(&boot_parameters, 0, sizeof(struct bparam_s));
	console_init(); 
	trace ("console init ok\n");

	if(sizeof(proc_t)>PAGE_SIZE){
		trace ("warn : tcb(size:%d) too big\n", sizeof(proc_t));
	}

	grub_mem_useable(&base_mm, &boot_parameters.bp_ramsize);
	trace ("ram size is %d M\n", boot_parameters.bp_ramsize/0x100000);

	load_krnl_option();
	
	memory_init(boot_parameters.bp_ramsize);
	trace ("memory_init init ok\n");
	system_init(); //init gdt and idt table
	traps_init();
	trace ("traps_init init ok\n");

	//kprintf ("init_compat_bsd ...\n");
	init_compat_bsd();
	//kprintf ("device_init ...\n");
	device_init();
	proc_entry_init();
	setup_unix_socketcall(NULL);

	trace ("kthread init\n");

	thread_init();
	trace ("process init\n");
	process_init();


	shm_init();

	trace ("arch_init2 init ...\n");
	arch_init2();

	trace ("Enable Timer ...\n");	

	enable();  //enable interrupt, 

	trace ("init clock ...\n");	
	clock_init();  //it realy schedule

	trace("proc setup...\n");
	proc_setup();

	lock_schedule(NULL);
	
	//gzip_test();

	pkt_buf_init();
	trace("pkt_buf_init...\n");

	arch_init3();
	//load_kernel_dlls("/device");

	init_net();
	dbg_init();
	dbg_init2();

	trace("wakeup init task ...\n");	

	//switch to a new process.
	thread_ready((uinit_task));
	thread_ready((daemon_task));
	unlock_schedule();
	//enable();  //enable interrupt, 

	trace("start schedule ...\n");
	switch_to_idle_thread();
	kprintf("lauch idle ok\n");
	while(1);

	//task_loops();
    return 1;
}

static int switch_flag = 0;

static bool _switch()
{
	bool ret= false;
	unsigned cur_flag;

	save_eflags(&cur_flag);

	if (switch_flag){
		ret = true;
	}

	restore_eflags(cur_flag);
	return ret;
}

void switch_enable()
{
	switch_flag = 1;
}

static void task_loops()
{
	//switch_enable();
	//kprintf("entry task_loops %d\n", _switch());

	for (; ; )
	{
		if (_switch())
		{
			switch_flag = 0;
			schedule();
		}
	}
}

/*
**stop used by debug
*/
void stop(void)
{
	while(1);
}

/*
** g++ utils
*/
__public void __gxx_personality_v0()
{
}

__public void _Unwind_Resume()
{
	kprintf("%s call\n", __FUNCTION__);
}

/*
**
*/
__public void load_amd64_kernel()
{
#ifdef __AMD64__
	//k
#else
	tty_init();  //must be first on console_init
	console_init();
	panic("amd64_start_kernel");
#endif
}

/*
**printf tast , take from internet
*/
void justatest()
{
	int i=0;
#define	SCN_WID		15
#define	SCN_HT		20
	kprintf("\x1B[2J""\x1B[1;%dH""TETRIS by Alexei Pazhitnov",		SCN_WID * 2 + 2);
	/* display banner */
	kprintf("\x1B[31m""B"	"\x1B[32m""o"	"\x1B[33m""r"
		"\x1B[34m""e"	"\x1B[35m""a"	"\x1B[36m""l"
		"\x1B[31;1m""i"	"\x1B[37;0m""s"
		" OS release 11, by Chris Giese "
		"<geezer@execpc.com>\n");

	kprintf("\x1B[2;%dH""Software by Chris Giese", SCN_WID * 2 + 2);
	kprintf("\x1B[4;%dH""'1' and '2' rotate shape", SCN_WID * 2 + 2);
	kprintf("\x1B[5;%dH""Arrow keys move shape", SCN_WID * 2 + 2);
	kprintf("\x1B[6;%dH""Esc or Q quits", SCN_WID * 2 + 2);

	kprintf("\x1B[11;%dH""Press any key to begin", SCN_WID * 2 + 2);
	kprintf("\x1B[10;%dH""\x1B[37;40;1m""       GAME OVER""\x1B[0m",SCN_WID * 2 + 2);
	kprintf("\x1B[8;%dH""Lines: %u   ",SCN_WID * 2 + 2, 5);
	kprintf("\x1B[s""Lines:ffffffffffffffffffffff    ");

#if 1
	poke32(0xfa004500, 99991);
	kprintf("value :%d\n", peek32(0xfa004500));
#else
	poke32(0xc0000000, 99991);
	kprintf("value :%d\n", peek32(0xc0000000));
#endif
		while (1)
		{
	kprintf("\x1B[s count  %d\n", i+=50);
	kprintf("\x1B[u");
		//mdelay(5);
		}
}

void dump_bochs_vbe_modelist()
{
	int i;
	u16_t*	pnTmpList=( u16_t*)0xc6a86;/*test for bochs, just work on bochs 2.1.1*/

	kprintf("mode list:\n");

	for ( i = 0 ; pnTmpList[i] != 0xffff && i < 1024 ; ++i ) {
	   if(pnTmpList[i]==0xffff)break;
		//pnModeList[i] = pnTmpList[i];
		kprintf("%X-", pnTmpList[i]);
    }
}


