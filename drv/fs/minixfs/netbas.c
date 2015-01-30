
#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>
#include "minixfs.h"


void minixfs_hook();
void remove_minixfs_hook();

int dll_main(char **args)
{
	int i=0;
	int now_mnt = 0;
	//char mnt_dir[255];
	//char mnt_dev[255];
	char *mnt_dev=NULL;
	long mode;
	int error=0;

	kprintf("minixfs Module Running ...\n");

	minixfs_hook();

	while (args[i])
	{
		//kprintf("arg %d %s\n",i,args[i]);
		if (stricmp(args[i],"--remount") == 0)
		{
			now_mnt = 1;
		}

		/*if (strnicmp(args[i],"-mount=", 7) == 0)
		{
			if(strcmp (&args[i][7], "root") ==0)
				kprintf("mount rootfs\n");
			fs_root_dev(&mnt_dev, &mode);
			now_mnt = 1;
		}*/

		i++;
	}

	if (now_mnt)
	{
		fs_root_dev(&mnt_dev, &mode);
		error=sys_mount(mnt_dev,"/",NULL, mode );
	}

	kprintf("error = %d\n", error);
	return 0;
}


int dll_destroy()
{
	remove_minixfs_hook();
	return 0;
}


int dll_version()
{
	kprintf("minixfs File system Driver for Netbas OS\n");
	return 0;
}


