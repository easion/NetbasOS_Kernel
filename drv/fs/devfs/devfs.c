

#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>

//command
//mount /dev/cfa1 /mnt
//mount devfs /dev
//mount procfs /proc




int devfs_probe(const mount_t *mp )
{
	printf("devfs_probe ...\n");
	if (mp->m_magic != DEVFS_MAGIC)
	{
		return -1;
	}
	printf("devfs_probe ok\n");
	return 0;
}

int devfs_mount(mount_t *mp, void *_data )
{
	printf("devfs_mount ok\n");
	return 0;
}



int devfs_readdir(file_t* filp, vfs_dirent_t *dirent)
{
	printf("devfs_readdir ok\n");
	return 0;
}

int devfs_unmount(mount_t *m, void *pVolume )
{
	printf("devfs_unmount ok\n");
	return 0;
}

int devfs_fread(file_t * filp, char * buf, int count)
{
	int n;
	printf("devfs_read ok\n",count);
	return n;
}

inode_t* devfs_opendir(inode_t * inode, char *filepath)
{
	printf("devfs_opendir ok\n");
	return NULL;
}

