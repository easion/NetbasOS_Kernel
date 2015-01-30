
#include <jicama/system.h>
#include <jicama/fs.h>
#include <jicama/paging.h>
#include <errno.h>
#include <string.h>

#define CLIBRARY_NAME "/system/libc.so"
#define CLIBRARY_LOADPOINT (0xff800000)

struct share_library
{
	struct share_library *next;
	unsigned addr;
	//unsigned base;
	int len;
	unsigned bbs;
	char *name;
};

struct share_library lib_head;
#define VIS_ADDR 0x1000000

void load_user_library()
{
	struct filp* filp;
	char *buff = (void*)CLIBRARY_LOADPOINT;

	//ÆðÓÃÄÚ´æ
	if (mem_writeable(CLIBRARY_LOADPOINT, 0x400000)!=0)
	{
		map_high_memory(CLIBRARY_LOADPOINT,0x400000,"user");
	}

	memset(&lib_head,0,sizeof(lib_head));

	filp = server_open(CLIBRARY_NAME, "r");

	if (!filp)
	{
		return;
	}

	//lib_head.base = VIS_ADDR;
	lib_head.name = CLIBRARY_NAME;
	lib_head.addr = CLIBRARY_LOADPOINT;
	lib_head.len = flength(filp);

	if (lib_head.len > 0x400000)
	{
		fclose(filp);
		return;
	}

	kprintf("try loading %s ...\n", CLIBRARY_NAME);
	fread( buff, 512, 1, filp);
	fclose(filp);

}

