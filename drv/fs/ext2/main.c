/**
 * main.c - ext2 support 
 *
 * Copyright (c) 2006 Easion Deng
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ext2.h"


int dll_main(char **args)
{
	ext2_hook();
	return 0;
}


int dll_destroy()
{
	remove_ext2_hook();
	return 0;
}



int dll_version()
{
	kprintf("ext2 File system Driver for Jicama OS\n");
	return 0;
}

