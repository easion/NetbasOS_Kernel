
#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>


//netbas utils

int procfs_probe(const mount_t *mp );
int procfs_mount(mount_t *mp, void *_data );
int procfs_unmount(mount_t *mp, void *pVolume );
int procfs_fread(file_t * filp, char * buf, int count);
int procfs_fwrite(file_t * filp, char * buf, int count);
inode_t* procfs_opendir(inode_t * inode, char *name);
int procfs_readdir(file_t* filp, vfs_dirent_t *dirent);


void procfs_hook();
void remove_procfs_hook();
#ifdef __MODULE__
int dll_main(char **args)
{
	kprintf("procfs Module Running ...\n");
	procfs_hook();
	return 0;
}


int dll_destroy()
{
	remove_procfs_hook();
	return 0;
}
#endif
int procfs_open(file_t* fp);

/********************************************/

static  fs_dev_ops_t _procfs=
{
	fs_name: "procfs",
	fs_copyright:"BSDL",
	fs_author:"Easion",
	fs_bmap: NULL,
	fs_opendir: procfs_opendir,

	fs_readdir: procfs_readdir,
	fs_probe:procfs_probe,
	fs_mount:procfs_mount,
	fs_unmount:procfs_unmount,
	fs_read:procfs_fread,
	fs_write:NULL,
	fs_mkdir:NULL,
	fs_openfile:procfs_open
};

int procfs_open(file_t* fp)
{
	int fd=-1;
	panic("error");
	return fd;
}

void procfs_hook()
{
	install_fs(&_procfs);
}

void remove_procfs_hook()
{
	deinstall_fs(&_procfs);
}


