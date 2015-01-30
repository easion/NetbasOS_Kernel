
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/ipc.h>
#include <string.h>


static int last_shm_entry_id;
volatile static LIST_HEAD(, shm_info) g_shm_list;
static sem_t thread_sock_sem;


//进程之间的共享内存实现
int shm_init()
{
	LIST_INIT(&g_shm_list);
	last_shm_entry_id=0x56;
	return 0;
}

struct shm_info*shm_find(int shm_id)
{
	struct shm_info		*item;

	LOCK_SCHED(thread_sock_sem);	

	LIST_FOREACH (item,&g_shm_list, entries)
	{
		if (item->id == shm_id)
		{
			UNLOCK_SCHED(thread_sock_sem);	
			return item;
		}
	}
	
	UNLOCK_SCHED(thread_sock_sem);	
	return NULL;
}

int shm_get_info(int id, struct shm_info *out_info)
{
	struct shm_info *item = shm_find(id);

	if (!item)
	{
		return EINVAL;
	}

	memcpy(out_info, item,sizeof(struct shm_info));	
	return 0;
}

static struct shm_info * shm_new(const char* name, unsigned user_addr, int size,long flags)
{
	int error;
	struct shm_info *shm_entry;
	

	shm_entry = kcalloc(sizeof(struct shm_info));

	if (!shm_entry)
	{
		return NULL;
	}
	shm_entry->id = last_shm_entry_id++;
	shm_entry->count=1;
	shm_entry->phy_addr=0;
	shm_entry->len=size;
	shm_entry->flags=flags;
	shm_entry->proc_addr=current_proc_vir2phys(user_addr);
	shm_entry->proc_addr_user = user_addr;
	strncpy(shm_entry->name,name,128);
	LOCK_SCHED(thread_sock_sem);	
	LIST_INSERT_HEAD(&g_shm_list,shm_entry, entries);
	UNLOCK_SCHED(thread_sock_sem);	
	//kprintf("shm_new:%s,user_addr=%x,shm_entry->id=%d\n", name,user_addr, shm_entry->id);

	return shm_entry;
}


//from如果为0,表示从内存中分配页面 
int shm_create(const char* name, void** addr, int size,long flags)
{
	int error;
	struct shm_info *shm_entry;
	void *user_addr;
	proc_t *rp = current_proc();

	user_addr = user_malloc(rp,size,0);
	if (!user_addr)
	{
		return ENOMEM;
	}
	

	shm_entry = shm_new(name,user_addr,size,flags);

	if (!shm_entry)
	{
		return ENOMEM;
	}

	*addr = user_addr;
	return shm_entry->id;
}

int shm_clone_area(int id, void** addr,char *name)
{
	int error;
	struct shm_info *out_info;
	struct shm_info *new_info;
	int pages;
	int size;
	void *user_addr;
	proc_t *rp = current_proc();

	out_info = shm_find(id);

	if (!out_info)
	{
		kprintf("shm_clone_area id %d not found\n",id);
		return EINVAL;
	}

	size=out_info->len;
	pages=(size+PAGE_SIZE-1)/PAGE_SIZE;


	user_addr = user_malloc(rp,size,0);
	if (!user_addr)
	{
		return ENOMEM;
	}

	new_info = shm_new(name,user_addr,size,0);
	if (!new_info)
	{
		return ENOMEM;
	}
	

	kprintf("shm_clone_area:user_addr=0x%x,out_info->user_addr=0x%x,phy_addr=%x, pages=%d \n",
		user_addr,out_info->proc_addr,out_info->phy_addr,pages);

	error = map_to(out_info->proc_addr,new_info->proc_addr,pages);


	if (error)
	{
		shm_release(new_info->id);
		return EINVAL;
	}

	if(addr)
	*addr = user_addr;

	return new_info->id;
}



int shm_remap_area(int id, void *to)
{
	struct shm_info *item = shm_find(id);
	int pages,error;


	if (!item)
	{
		kprintf("shm_remap_area error %d\n", id);
		return EINVAL;
	}

	item->phy_addr = to;

	pages=(item->len+PAGE_SIZE-1)/PAGE_SIZE;

	

	error=map_share_page(to,item->proc_addr, pages);
	//kprintf("shm_remap_area succ to %x %d\n", to,error);

	if (error)
	{
		return EINVAL;
	}


	//unmap_user_pages(proc_addr, pages);
	return 0;
}


int shm_release(int id)
{
	struct shm_info *out_info;
	int pages;
	proc_t *rp = current_proc();

	out_info = shm_find(id);

	if (!out_info)
	{
		return EINVAL;
	}
	
	pages=(out_info->len+PAGE_SIZE-1)/PAGE_SIZE;

	if (out_info->count)
	{
		out_info->count--;
	}

	if (out_info->count)
	{
		return EINVAL;
	}

	user_free(rp, out_info->proc_addr_user);
	LOCK_SCHED(thread_sock_sem);	
	LIST_REMOVE(out_info,entries);
	UNLOCK_SCHED(thread_sock_sem);	
	unmap_user_pages(out_info->proc_addr, pages);
	kfree(out_info);
	return 0;
}

