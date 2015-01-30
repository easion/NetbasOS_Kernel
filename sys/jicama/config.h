
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------
#ifndef POSIX_CONFIG_H
#define POSIX_CONFIG_H

#define __KERNEL__  /*It must on first!*/
#define __netbas__  /*It must on first!*/

#ifdef __IA32__

#define HZ		1000
#include <arch/x86/dev.h>
#include <arch/x86/io.h>
#include <arch/x86/protect.h>

#elif defined(__ARM__)  //arm9

//#define HZ		1000
#include <arch/arm/regs.h>
#include <arch/arm/dev.h>

#else
#error bad platform
#endif

#endif

