/*
**     (R)Jicama OS
**     Main Function
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/utsname.h>
#include <jicama/linux.h>
#include <string.h>


__public int uname(struct utsname *name)
{

    if (name == NULL)
		return EINVAL;

    //memset(name, 0, sizeof(struct utsname));
    strncpy(name->sysname, UTS_SYSNAME,SYS_NMLN);
    strncpy(name->nodename, UTS_NODENAME,SYS_NMLN);
    strncpy(name->release, UTS_RELEASE,SYS_NMLN);
    strncpy(name->version, UTS_VERSION,SYS_NMLN);
    strncpy(name->machine, UTS_MACHINE,SYS_NMLN);
	//kprintf("uname call ok\n");
    return 0;
}

__public dev_t dev_nr(int ma, u8_t mi)
{
	ma &= 0007;
	mi &=  0xff;

	return ((unsigned)(ma << 8) + mi);
}

