
// ---------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//----------------------------------------------------------------------------------------
#ifndef __SH_MEM__
#define __SH_MEM__

struct shm_info
{
	char name[128];
	int len; //长度
	int count; //长度
	unsigned proc_addr_user; // 在进程中的地址
	unsigned proc_addr; // 在进程中的地址
	unsigned phy_addr; //在系统全局的物理地址
	int id;
	long flags;
	LIST_ENTRY(shm_info) entries;      /* List. */
};

struct vm_region
{
	int id;
	int lock;
	int wiring;

	u32_t vm_base;
	u32_t vm_addr;
	int vm_size;
	char vm_name[32];
};

int shm_init(void);
int shm_get_info(int id, struct shm_info *out_info);
int shm_clone_area(int id, void** proc_addr,char *name);
int shm_create(const char* name, void** addr, int size,long flags);
int shm_release(int id);
int shm_remap_area(int id, void *to);

#endif //__SH_MEM__
