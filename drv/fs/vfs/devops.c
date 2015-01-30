
#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <drv/errno.h>
#include <drv/log.h>
#include <assert.h>
#include "sys.h"

#define MAX_FS 8
static fs_dev_ops_t *fsdev[MAX_FS];


static int from_root_node(const char *dirname)
{
	char *s=NULL;

	s=strfind(dirname,':');

	if (dirname[0]=='/' || s){
		//kprintf("from root dir %s %s\n",s,dirname);
		return 1;
	}

	return 0;
}

int install_init()
{
	memset(fsdev, 0, sizeof(fsdev));
	return 0;
}

int install_fs(fs_dev_ops_t *ops)
{
	int i;

	for (i = 0; i<MAX_FS; i++)
	{
		if (fsdev[i] == NULL)
		{
			fsdev[i] = ops;
			return 0;
		}
	}

	return -1;
}

int deinstall_fs(fs_dev_ops_t *ops)
{
	int i;
	for (i = 0; i<MAX_FS; i++)
	{
		if (fsdev[i] == ops)
		{
			fsdev[i] = NULL;
			return 0;
		}
	}

	return -1;
}

inline fs_dev_ops_t* get_inode_ops(inode_t * inode)
{
	mount_t *mp;

	mp = inode->i_super;
	if (!mp)
	{
		return NULL;
	}
	return mp->m_ops;
}


fs_dev_ops_t* get_ops_byname(char* fsname)
{
	int i;

	for (i = 0; i<MAX_FS; i++)
	{
		if (!fsdev[i])
		{
			continue;
		}
		if (strcmp(fsdev[i]->fs_name,fsname) == 0)
		{
			return fsdev[i];
		}
	}
	return NULL;
}

off_t vfs_bmap(inode_t * inode, off_t pos, int create)
{
	fs_dev_ops_t *op;


	op = get_inode_ops(inode);

	if (!op || !op->fs_bmap)
	{
		return (off_t)-1;
	}

	return op->fs_bmap(inode,pos,create);
}

inode_t* vfs_opendir(inode_t * inode, char *name)
{
	fs_dev_ops_t *op;

	op = get_inode_ops(inode);

	if (!op || !op->fs_opendir)
	{
	printf("vfs_opendir %s @ %x -addr 0x%x\n", name, op);
		return NULL;
	}


	return op->fs_opendir(inode,name);
}


int vfs_readlink(inode_t * inode, char *buff, size_t buf_size)
{
	fs_dev_ops_t *op;

	op = get_inode_ops(inode);

	if (!op || !op->fs_readlink)
	{
		return -ENOENT;
	}


	return op->fs_readlink(inode,buff,buf_size);
}


int vfs_closedir(inode_t * inode)
{
	fs_dev_ops_t *op;

	op = get_inode_ops(inode);

	if (!op || !op->fs_closedir)
	{
	//printf("vfs_closedir  @ %x -addr 0x%x\n",  device,op);
		return 0;
	}


	return op->fs_closedir(inode);
}

int vfs_openfile(file_t * filp)
{
	fs_dev_ops_t *op;

	op = get_inode_ops(filp->f_inode);

	if (!op || !op->fs_openfile)
	{
	//printf("vfs_openfile @ %x -addr 0x%x\n",  device,op);
		return -1;
	}


	return op->fs_openfile(filp);
}




inode_t* vfs_get_inode_name(inode_t * inode,ino_t ino, char *name)
{
	fs_dev_ops_t *op;

	op = get_inode_ops(inode);

	if (!op || !op->fs_get_inode_name)
	{
		return NULL;
	}

	return op->fs_get_inode_name(inode,ino,name);
}

void vfs_release_buttom(file_t * filp)
{
	inode_t* inode = filp->f_inode;
	fs_dev_ops_t *op;
	dev_t device=inode->i_dev;

	
	//iput(inode);


	if (S_ISFIFO(inode->i_mode))
	{	
		close_pipe(filp);
		return 0;
	}
}

int vfs_release(file_t * filp)
{
	inode_t* inode = filp->f_inode;
	fs_dev_ops_t *op;
	dev_t device=inode->i_dev;

	
	//iput(inode);


	if (S_ISFIFO(inode->i_mode))
	{
		//if (filp->f_inode->i_count>=2)
		{
			//close_pipe(filp);
		}
		return 0;
	}
	else if (S_ISSOCK(inode->i_mode))
	{
		//kprintf("vfs_release():release socket start%p.\n",filp);
		if (filp->f_iob&&1>=inode->i_count){
		closesocket(filp->f_iob);
		//free_fiob(filp);
		filp->f_iob=NULL;
		//kprintf("vfs_release():release socket%p ok.\n",filp);
		}
		else{
			//kprintf("vfs_release():release busy %d.\n",inode->i_count);
		}
		return 0;
	}
	else if (S_ISBLK(inode->i_mode) || S_ISCHR(inode->i_mode))
	{
		//kprintf("vfs_release():dev_close ok.\n");
		if (filp->f_iob&&1==inode->i_count)
		{
		dev_close(filp->f_iob);
		free_fiob(filp);		
		}
		return 0;
	}
	else if (S_ISWHT(inode->i_mode)&&1==inode->i_count){
		if (filp->f_iob){
		proc_file_close(filp->f_iob);
		free_fiob(filp);
		}
	}	
	else{
	}

	op = get_inode_ops(inode);

	if (!op || !op->fs_releasefile)
	{
		//printf("vfs_release()  release file on device 0x%x\n", inode->i_dev);
		return 0;
	}

	return op->fs_releasefile(filp);
}

int vfs_read(file_t * fp,char* buffer, int count, int *exist_val)
{
	fs_dev_ops_t *op;
	int ret;
	inode_t* inop = fp->f_inode;

	assert(fp != NULL);
	assert(inop != NULL);
	//printf("vfs_read ino is %d\n", inop->i_number);

	 if (!buffer) {
		return -1;
	 }

	op = get_inode_ops(inop);

	if (!op || !op->fs_read)
	{
		*exist_val = 0;
		return -1;
	}

	ret = op->fs_read(fp,buffer, count);
	*exist_val = 1;
	return ret;
}

int vfs_write(file_t * fp,char* buffer, int count, int *exist_val)
{
	fs_dev_ops_t *op;

	assert(fp != NULL);
	assert(fp->f_inode != NULL);

	 if (!buffer)
	 {
		 return -1;
	 }

	op = get_inode_ops(fp->f_inode);

	if (!op || !op->fs_write)
	{
		*exist_val = 0;
		return -1;
	}

	*exist_val = 1;
	return op->fs_write(fp,buffer, count);
}

int vfs_readdir(file_t * fp, vfs_dirent_t *vd)
{
	int err;
	fs_dev_ops_t *op;
	inode_t *inode = fp->f_inode;

	 if (!fp || !vd)
	 {
		printf("vfs_readdir error ino is %d\n", inode->i_number);
		 return -1;
	 }


	//snooze(2);
	op = get_inode_ops(inode);

	if (!op || !op->fs_readdir)
	{
		printf( "vfs_readdir error fs_readdir null\n");
		return -1;
	}

	err = op->fs_readdir(fp,vd);
	//vd->d.l_ino = inode->i_number?inode->i_number:1;
	vd->d.l_off = fp->f_pos;
	return err;
}



int vfs_mount(mount_t *mp, void *arg)
{
	fs_dev_ops_t *op;

	if (!mp){
		return -1;
	}
	op = (mp->m_ops);
	if (!op || !op->fs_mount)
	{
		return -1;
	}
	return op->fs_mount(mp, arg);
}


int vfs_unmount(mount_t *mp)
{
	fs_dev_ops_t *op;
	if (!mp){
		return -1;
	}

	op = (mp->m_ops);
	if (!op || !op->fs_unmount)
	{
		return -1;
	}
	return op->fs_unmount(mp,NULL);
}

int vfs_ino_rw(mount_t *mp, inode_t *inode, int rw)
{
	fs_dev_ops_t *op;

	if (!mp){
		return -1;
	}

	op = get_inode_ops(inode);
	if (!op || !op->fs_inorw)
	{
		return -1;
	}
	return op->fs_inorw(mp, inode,rw);
}


//struct mount_list *mount_list;
static TAILQ_HEAD(, mount_list) mount_list_head;
static unsigned proc_sem_lock;

int mount_list_init()
{
	TAILQ_INIT(&mount_list_head);
	return 0;
}

struct mount_list* add_mount_list(char *devname, char *mntdir)
{
	struct mount_list *list;


	if (!from_root_node(mntdir))
	{
		printf("add_mount_list() format error for %s, must mount as D:\n",mntdir);
		return NULL;
	}

	if (find_mount_list(mntdir))
	{
		return NULL;
	}

	list=kmalloc(sizeof(struct mount_list),0);
	

	strncpy(list->m_dev_name, devname,255);
	strncpy(list->m_mnt_dir, mntdir,255);
	LOCK_SCHED(proc_sem_lock);

	TAILQ_INSERT_TAIL(&mount_list_head,list,next);
	UNLOCK_SCHED(proc_sem_lock);
	

	return list;
}

char* get_mount_dir(char *mntdir,const char *dirname, int *pos)
{
	int i=1;
	int c=0;
	char *pdir=dirname;

	if (dirname[0]!='/')
	{
		*pos=0;
		return NULL;
	}

	mntdir[0]='/';

	while (*pdir=='/'){
		c++;
		pdir++;
	}

	while (*pdir!='/'&&*pdir){
		mntdir[i]=*pdir;
		c++;
		i++;
		pdir++;
	}

	mntdir[i]=0;
	*pos=c;
	return mntdir;
}


struct mount_list* find_mount_list(const char *mntdir)
{
	struct mount_list *list;
	void *nxt;

	if (!mntdir)
	{
		return NULL;
	}

	TAILQ_FOREACH_SAFE(list,&mount_list_head,next,nxt){
		if (stricmp(list->m_mnt_dir,mntdir)==0)
		{			
			return (list);
		}
	}
	return 0;
}

struct mount_list* get_mount_list(const char *mntdir)
{
	struct mount_list*ret=find_mount_list(mntdir);
	if (!ret)
	{
		ret=find_mount_list("/");
	}
	return ret;
}

int remove_mount_list(const char *mntdir)
{
	struct mount_list *list;
	void *nxt;

	LOCK_SCHED(proc_sem_lock);

	TAILQ_FOREACH_SAFE(list,&mount_list_head,next,nxt){
		if (stricmp(list->m_mnt_dir,mntdir)==0)
		{
			TAILQ_REMOVE(&mount_list_head,list,next);
			kfree(list);
			UNLOCK_SCHED(proc_sem_lock);
			return 1;
		}
	}
	UNLOCK_SCHED(proc_sem_lock);
	return 0;
}


static int mount_fs(mount_t *mp)
{
	int error;

	error = try_probe_fs(mp);
	if (error)
	{
		goto err;
	}

	//printf("try_probe_fs succ\n");

	error = vfs_mount(mp,NULL);

	if(error)
		goto err1;

	//printf("vfs_mount AT dev 0x%x done\n", mp->m_dev->devno);
	return 0;
err:
	printf("vfs_probe: unknow filesystem type\n");
err1:
	printf("no file system mounted\n");
	return -1;
}

struct devices_param
{
	int size;
	int dev;
};

struct devices_param devices_param[1];


int dev_block_size(int dev)
{
	mount_t *mp;

	if (dev==devices_param[0].dev)
	{
		return devices_param[0].size;
	}

	mp = get_superblock(dev);

	if (!mp)
	{
		return 0;
	}

	//printf("dev_block_size %x %s %d\n", dev,op->fs_name,op->fs_blksz);

	return mp->m_blk_size;
}




int sys_mount(const char *devname, char *mntdir, const char *fstype, long flags)
{
	int err;
	struct mount_list*mlist;
	int dev_fd;
	m_dev_type_t t = D_DEV;
	inode_t *inode;
	dev_prvi_t *dm;
	mount_t *mp;
	fs_dev_ops_t*ops;
	static int ramdev=0x17a9;

	dm=kmalloc(sizeof(dev_prvi_t),0);

	if (strcmp(devname,"procfs")==0)
	{
		printf("proc fs found\n");
		t = D_PROC;
	}

	mlist=add_mount_list(devname, mntdir);
	if (!mlist){
		kprintf("sys_mount() mount %s at %s error\n", devname,mntdir);
		return -1;//已经挂上了
	}


	if(strncmp(devname,"/dev",4)==0){
		t = D_DEV;
		dev_fd = dev_open(devname,0,dm);

		devices_param[0].dev = dm->devno;
		devices_param[0].size = 2048;

		if (dev_fd<0){
			printf("device %s not exist\n", devname);
			return -1;
		}
	}
	else{
		dm->devno = ramdev++;
	}

	mp = alloc_superblock(dm);

	if (!mp)
	{
		printf("alloc_superblock() %s error\n", devname);
		return -1;
	}

	if (t == D_PROC)
	{
		mp->m_magic = PROC_MAGIC;
	}



	if (fstype)
	{
		ops=get_ops_byname(fstype);
		if (!ops)
		{
		printf("init_ramfs got line%d\n",__LINE__);
			return -1;
		}
		mp->m_ops = ops;
		err = ops->fs_mount(mp, NULL);
	}
	else{
		err = mount_fs(mp);
	}

	if (err<0){
		kprintf("sys_mount() try_probe_fs %s at %s error\n", devname,mntdir);
		remove_mount_list(devname);
		return -1;
	}

	mlist->m_super = mp;
	return err;
error:
	remove_mount_list(devname);
	dev_close(dm);
	//re(mp);
	return -1;
}

int sys_unmount(char *dname)
{
	mount_t *mp = find_mount_list(dname);
	if(!mp)return -1;
	vfs_unmount(mp);
	invalidate(mp->m_dev->devno);
	remove_mount_list(dname);
	return OK;
}

/*
int vfs_probe(mount_t *mp)
{
	fs_dev_ops_t *op;

	op = get_ops(mp->m_dev->devno);
	if (!op || !op->fs_probe)
	{
		return -1;
	}
	return op->fs_probe(mp);
}
*/

int vfs_write_inode(inode_t *inode)
{
	fs_dev_ops_t *op;

	op = get_inode_ops(inode);
	if (!op || !op->fs_write_inode)
	{
		return -1;
	}
	return op->fs_write_inode(inode);
}

int vfs_unlink (inode_t* inode, unsigned char* file)
{
	fs_dev_ops_t *op;

	op = get_inode_ops(inode);
	if (!op || !op->fs_unlink)
	{
		return -1;
	}
	return op->fs_unlink(inode, file);
}

int vfs_link (inode_t* inode, const char* oldpath,const char *newpath)
{
	fs_dev_ops_t *op;

	op = get_inode_ops(inode);
	if (!op || !op->fs_symlink)
	{
		return -1;
	}
	return op->fs_symlink(inode, oldpath,newpath);
}


int vfs_rmdir (inode_t* inode, unsigned char* file)
{
	fs_dev_ops_t *op;

	op = get_inode_ops(inode);
	if (!op || !op->fs_rmdir)
	{
		return -1;
	}
	return op->fs_rmdir(inode, file);
}

int vfs_mkdir(inode_t* inode, char* dname,int nPerms)
{
	fs_dev_ops_t *op;

	op = get_inode_ops(inode);
	if (!op || !op->fs_mkdir)
	{
		return -1;
	}
	return op->fs_mkdir(inode, dname,nPerms);
}

int vfs_create_file(inode_t* inode, unsigned char* file,int mode)
{
	fs_dev_ops_t *op;

	op = get_inode_ops(inode);
	if (!op || !op->fs_creat)
		return -1;
	return op->fs_creat(inode,file,mode);
}


int vfs_truncate(mount_t *mp, unsigned long cluster )
{
	fs_dev_ops_t *op;

	op = (mp->m_ops);
	if (!op || !op->fs_truncate)
	{
		return -1;
	}
	return op->fs_truncate(mp,cluster);
}

void register_realfs_hook()
{
#ifdef __ENABLE_ROMFS__
	romfs_hook(); //必须是在最先,在后面初始化会存在已知的问题
#endif
#ifdef __ENABLE_FATFS__
	fat_hook();
#endif
#ifdef __ENABLE_CDFS__
	cdfs_hook();
#endif

	ramfs_hook();

#ifdef __ENABLE_MINIXFS__
	minixfs_hook();
#endif
	procfs_hook();
}

int try_probe_fs(mount_t *mp)
{
	int i;
	fs_dev_ops_t *op=NULL;

	//if (mp->m_magic != NO_MAGIC)
	//	return 0;

	for (i = 0; i<MAX_FS; i++)
	{
		op = fsdev[i];
		if (op == NULL)
		{
			continue;
		}

		if (!op->fs_probe)
		{
			continue;
		}

		//printf("fs_name = %s\n",op->fs_name);

		if( op->fs_probe(mp)== 0){
			mp->m_ops = op;
			return 0;
		}
	}

	return -1;
}

