
// ---------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//-----------------------------------------------------------------------------------
#ifndef UTSNAME_H
#define UTSNAME_H

/*jicama os version*/

#define EMAIL		"<atcn563@yahoo.com.cn>"
#define COPYRIGHT 		"(C)copyright 2003-2006 Easion.Deng"

#define UTS_SYSNAME "Netbas"
#define UTS_MACHINE "i386"
#define UTS_NODENAME "NetbasPC"
#define UTS_DOMAINNAME "(none)"
#define UTS_RELEASE "0.31"
#define UTS_VERSION  "Alpha"

#define SYS_NMLN        9

typedef struct utsname {
     char machine[SYS_NMLN];
    char nodename[32];
    char release[SYS_NMLN];
   char sysname[SYS_NMLN];
    char version[SYS_NMLN];
} *utsname_t;

int uname(utsname_t name);
#endif

