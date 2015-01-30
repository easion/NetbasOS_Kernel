
/************************************************************
  Copyright (C), 2003-2010, Netbas OS Project.
  FileName: 
  Author:        Version :          Date:
  Description:    
  Version:        
  Function List:   
  History:         
      Easion   2010/2/6     1.0     build this moudle  
***********************************************************/


#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/fnctl.h>
#include <drv/log.h>
#include <assert.h>


static mount_t *root_mount_blk;
static long root_fs_mode=0;
static char root_fs_dev[255];
static unsigned proc_sem_lock;

/*************************************************
  Function:       
  Description:    
  Input:          
  Output:         
  Return:         
  Others:         
*************************************************/

int fs_root_dev(char **ret, long* mode)
{
	if(ret)*ret = root_fs_dev;
	if(mode)*mode = root_fs_mode;
	return 0;
}



int module_exist(const char *str, const char *label, int count, char **ret)
{
	int len1=strlen(label);
	int len2=strlen(str);
	char *end_of_str=str+len2;
	char *begin_of_str=str;

	assert(str != NULL);
	assert(label != NULL);

	if(!len1 || !len2)
		return FALSE;

	if (count>0)
		len1 = MIN(count,len1);

	while((unsigned long)(begin_of_str+len1)<=(unsigned long)end_of_str){
		if(!strnicmp(begin_of_str, label,len1)){
			if(ret)*ret = begin_of_str;
			return TRUE;
		}
		begin_of_str++;
	}

	return FALSE;
}


int init_kernel_option()
{
	char  module_string[1024]; //=get_kernel_command();
	int i,len,retval;
	int err;
	char *string;
	char *ret;
	char match_name[1024]="/fsroot=";

	strncpy(module_string, get_kernel_command(),1024);

	len = strlen(match_name);
	retval = module_exist(module_string, match_name,len,&ret);

	//printf("kernel cmd = %s\n", module_string);

	if (retval==0)
	{
		root_fs_mode = O_RDONLY;
		strcpy(root_fs_dev,"/dev/hdc");
		return 1;
	}
	ret += len;
	//printf("kernel ret = %s\n", ret);

	while (ret[i]!=',' && ret[i]!=0)
	{
		root_fs_dev[i] = ret[i];
		i++;
	}

	root_fs_dev[i]=0;
	//printf("root_fs_dev = %s\n", root_fs_dev);

	if(ret[i]==','){
		i++;
		if(stricmp(&ret[i],"r")==0)
			root_fs_mode = O_RDONLY;
		else if(stricmp(&ret[i],"rw")==0)
			root_fs_mode = O_RDWR;
		else if(stricmp(&ret[i],"w")==0)
			root_fs_mode = O_WRONLY;
	}
	else{
		root_fs_mode = O_RDWR;
	}

	//printf("root_fs_mode = %x\n", root_fs_mode);	
	return 0;
}

int dll_main(char **args)
{
	int i,error;
	char **argv=args;
	int  argc=0;
	struct mount_list*mlist;
	fs_task_t *fp ;

	init_kernel_option();	
	
	install_init();
	mount_list_init();
	buffer_init();
	inode_init();
	superblock_init();

	for (i=0; i<MAX_PROC; i++)
		fd_reset(i);

	init_process();
	register_realfs_hook();

	error=sys_mount(root_fs_dev,"/",NULL,root_fs_mode );

	if (error)
	{
		printf("mount rootfs error\n");
	}

	mlist= find_mount_list("/");

	fp = current_filp();

	fp->pwd=mlist->m_super->m_root;	
	fp->root=mlist->m_super->m_root;
	
	/*error=sys_mount("ramfs","/ramfs","ramfs",0666 );
	if (error)
	{
		printf("mount ramfs error\n");
	}*/
	//init_ramfs();
	return 0;
}

int dll_destroy()
{
	return 0;
}


 static mount_t *superblk[NR_SUPER];
static TAILQ_HEAD(,super_block) g_freelist;
static TAILQ_HEAD(,super_block) g_sblist;

static void superblock_init(void)
{
	int i;

	for (i=0; i<NR_SUPER; i++)
		superblk[i]=NULL;	 
}


mount_t* alloc_superblock(dev_prvi_t *dev_nr)
{
	 int i;

	 LOCK_SCHED(proc_sem_lock);
		
	for (i = 0; i<NR_SUPER; i++)
	{
		if (superblk[i]==NULL)
		{
		superblk[i]=(mount_t*)kmalloc(sizeof(mount_t),0);
	    superblk[i]->m_dev = 	dev_nr;
		superblk[i]->m_magic = NO_MAGIC;
		UNLOCK_SCHED(proc_sem_lock);
	    return superblk[i];
	    }
	}
	UNLOCK_SCHED(proc_sem_lock);
	kprintf("failed alloc_superblock%x\n",dev_nr);
    return NULL;
}

inline dev_prvi_t* get_dev_desc(dev_t dev)
{
	register int i;

	LOCK_SCHED(proc_sem_lock);
		
	for (i = 0; i<NR_SUPER; i++)
	{
		if (superblk[i] == NULL)
			continue;
		if (superblk[i]->m_dev->devno == dev){
			UNLOCK_SCHED(proc_sem_lock);
		    return superblk[i]->m_dev;
		}
	}
	UNLOCK_SCHED(proc_sem_lock);
	//kprintf("get_superblock 0x%x failed!\n",dev);
	return NULL;
}

mount_t* get_superblock(dev_t dev)
{
	register int i;

	LOCK_SCHED(proc_sem_lock);
		
	for (i = 0; i<NR_SUPER; i++)
	{
		if (superblk[i] == NULL)
			continue;
		if (!superblk[i]->m_dev)
		{
			continue;
		}
		if (superblk[i]->m_dev->devno == dev){
			UNLOCK_SCHED(proc_sem_lock);
		    return superblk[i];
		}
	}
	UNLOCK_SCHED(proc_sem_lock);
	//kprintf("get_superblock 0x%x failed!\n",dev);
	return NULL;
}

void show_superblock()
{
	register int i;
		
	kprintf("superblock list\n",superblk[i]->m_dev);
	for (i = 0; i<NR_SUPER; i++)
	{
		if (superblk[i] == NULL)
			continue;
	kprintf("-0x%x- ",superblk[i]->m_dev);
	}
}

fs_task_t *current_filp()
{
#define	__IDLE_PROC		0	//idle的进程号
#define	__INIT_PROC		2	//init的进程号

	LOCK_SCHED(proc_sem_lock);
	int cur_task_no = current_taskno();
	//assert(cur_task_no!=__IDLE_PROC);
	if (cur_task_no < __INIT_PROC || cur_task_no >= MAX_PROC)
	{
		int id;
		char *thread_name;
		UNLOCK_SCHED(proc_sem_lock);

		id=current_thread_id(&thread_name);
		panic("current_filp: current in kernel task %d,tid=%d, tname=%s\n",cur_task_no,id,thread_name);
		t_current = &proc[__INIT_PROC];
	}
	else{
		t_current=&proc[cur_task_no]; /*init proc*/
	}

	UNLOCK_SCHED(proc_sem_lock);
	return t_current;
}


 fs_task_t *t_current =&proc[1];

 fs_task_t proc[MAX_PROC];

