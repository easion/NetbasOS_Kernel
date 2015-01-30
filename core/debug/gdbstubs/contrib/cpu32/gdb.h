/*
  Copyright (c) 1999 by      William A. Gatliff
  All rights reserved.          bgat@usa.net

  See the file COPYING for details.

  This file is provided "as-is", and without any express
  or implied warranties, including, without limitation,
  the implied warranties of merchantability and fitness
  for a particular purpose.

  The author welcomes feedback regarding this file.
*/

/* $Id: gdb.h,v 1.1 2000/04/01 04:10:16 bgat Exp $ */

#if !defined( GDB_H_INCLUDED )
#define GDB_H_INCLUDED


/* platform-specific stuff */
void gdb_putc ( char c );
char gdb_getc ( void );
short gdb_peek_register_file ( short id, long *val );
short gdb_poke_register_file ( short id, long val );
void gdb_step ( char *hargs );
void gdb_continue ( char *hargs );
void gdb_kill( void );
void gdb_return_from_exception( void );


/* platform-neutral stuff */
long hex_to_long ( char h );
char lnibble_to_hex ( char i );
long hexbuf_to_long ( short len, const char *hexbuf );
short long_to_hexbuf ( long l, char *hexbuf, short pad );
unsigned char gdb_putstr ( short len, const char *buf );
void gdb_putmsg ( char c, const char *buf, short len );
short gdb_getmsg ( char *rxbuf );
void gdb_last_signal ( short sigval );
void gdb_expedited ( short sigval );
void gdb_read_memory ( const char *hargs );
void gdb_write_memory ( const char *hargs );
void gdb_console_output( short len, const char *buf );
void gdb_write_registers ( char *hargs );
void gdb_read_registers ( char *hargs );
void gdb_write_register ( char *hargs );
void gdb_monitor ( short sigval );
void gdb_handle_exception( long sigval );

#endif
