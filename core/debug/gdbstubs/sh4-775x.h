/*
  Copyright (c) 1999 by      William A. Gatliff
  All rights reserved.     bgat@open-widgets.com

  See the file COPYING for details.

  This file is provided "as-is", and without any express
  or implied warranties, including, without limitation,
  the implied warranties of merchantability and fitness
  for a particular purpose.

  The author welcomes feedback regarding this file.

  SH-4 support added by Benoit Miller (fulg@iname.com).
*/

/* $Id: sh4-775x.h,v 1.3 2002/04/08 15:08:20 bgat Exp $ */

#if !defined (GDB_SH4_775X_INCLUDED)
#define GDB_SH4_775X_INCLUDED


extern void gdb_init_register_file ( long initial_r15 );

/* exception handlers */
extern void gdb_exception_dispatch ( void );
extern void gdb_unhandled_isr ( void );
extern void gdb_trapa_isr ( void );
extern void gdb_illegalinst_isr ( void );
extern void gdb_addresserr_isr ( void );
extern void gdb_ubc_isr ( void );

/* exception event table */
extern const void *exception_event_table[];


#endif
