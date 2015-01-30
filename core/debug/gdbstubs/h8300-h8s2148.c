
/* 
  Copyright (c) 2001 by      William A. Gatliff
  All rights reserved.      bgat@billgatliff.com

  See the file COPYING for details.

  This file is provided "as-is", and without any express or implied
  warranties, including, without limitation, the implied warranties of
  merchantability and fitness for a particular purpose.

  The author welcomes feedback regarding this file.

  **

  $Id: h8300-h8s2148.c,v 1.1 2003/03/26 16:20:13 bgat Exp $

arm-elf-gcc -DCRT0 -DKS32C50100 -nostartfiles -nodefaultlibs -Wall -g -o arm7tdmi arm7tdmi.[cS] arm7tdmi-ks32c50100.c gdb.c
*/

#include "gdb.h"
#include "h8300-h8s2148.h"


int gdb_putc (int c)
{
  return c;
}

int gdb_getc (void)
{
  return -1;
}


void h8300_h8s2148edk_startup(void)
{
}

void gdb_kill (void)
{
}

void gdb_detach (void)
{
}


