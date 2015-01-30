
// --------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//---------------------------------------------------------------------------------

extern void smp_scan();
extern void cpuinfo_init(void);
void halt(void);
void fill_userdef_call();

extern void tty_init(void);

//used by sys init
extern void process_init();
extern void protect_init(void);
extern void machine_start();
extern int fs_init(int boot_dev);
extern void clock_init(void);
extern int lfb_init(  grub_info_t *multiboot_info );


extern void paging_init(unsigned long useable);
extern u32_t get_bios_mem_size(unsigned long* start, unsigned long* end);
extern void switch_proc(proc_t* p);
extern  void low_mem_init(grub_info_t* info);
extern void thread_init();
void thread_msg_init();
void proc_setup();
void switch_to_idle_thread();

 void system_init(void);
int dbg_init2(void);
int dbg_init(void);
void init_net(void);
void init_compat_bsd(void);
void proc_entry_init(void);
void log_init(void);
int load_krnl_option(void);
void traps_init(void);
int pkt_buf_init(void);








