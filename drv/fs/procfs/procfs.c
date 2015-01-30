

#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>




int procfs_probe(const mount_t *mp )
{
	printf("procfs_probe ...\n");
	if (mp->m_magic != PROC_MAGIC)
	{
		return -1;
	}
	printf("procfs_probe ok\n");
	return 0;
}

int procfs_mount(mount_t *mp, void *_data )
{
	printf("procfs_mount ok\n");
	return 0;
}



int procfs_readdir(file_t* filp, vfs_dirent_t *dirent)
{
	panic("procfs_readdir ok\n");
	return -1;
}

int procfs_unmount(mount_t *m, void *pVolume )
{
	printf("procfs_unmount ok\n");
	return 0;
}

int procfs_fread(file_t * filp, char * buf, int count)
{
	int n=-1;
	panic("procfs_read ok\n",count);
	return n;
}

inode_t* procfs_opendir(inode_t * inode, char *filepath)
{
	panic("procfs_opendir ok\n");
	return NULL;
}

