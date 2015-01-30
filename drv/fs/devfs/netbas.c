
#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>


//netbas utils

int devfs_probe(const mount_t *mp );
int devfs_mount(mount_t *mp, void *_data );
int devfs_unmount(mount_t *mp, void *pVolume );
int devfs_fread(file_t * filp, char * buf, int count);
int devfs_fwrite(file_t * filp, char * buf, int count);
inode_t* devfs_opendir(inode_t * inode, char *name);
int devfs_readdir(file_t* filp, vfs_dirent_t *dirent);


static void devfs_hook();
static void remove_devfs_hook();

int devfs_open(file_t* fp);

/********************************************/

static  fs_dev_ops_t _devfs=
{
	fs_name: "devfs",
	fs_copyright:"BSDL",
	fs_author:"Easion",
	fs_bmap: NULL,
	fs_opendir: devfs_opendir,

	fs_readdir: devfs_readdir,
	fs_probe:devfs_probe,
	fs_mount:devfs_mount,
	fs_unmount:devfs_unmount,
	fs_read:devfs_fread,
	fs_write:NULL,
	fs_mkdir:NULL,
	fs_openfile:devfs_open
};

int devfs_open(file_t* fp)
{
	int fd=-1;
	return fd;
}

static void devfs_hook()
{
	install_fs(&_devfs);
}

static void remove_devfs_hook()
{
	deinstall_fs(&_devfs);
}

#ifdef __DEVFS_MODULE__

int dll_main(char **args)
{
	kprintf("devfs Module Running ...\n");
	devfs_hook();
	return 0;
}

int dll_destroy()
{
	remove_devfs_hook();
	return 0;
}

int dll_version()
{
	kprintf("devfs File system Driver for Netbas OS\n");
	return 0;
}

#endif

