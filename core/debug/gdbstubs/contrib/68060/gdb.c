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

/* $Id: gdb.c,v 1.1 2001/05/18 15:54:05 bgat Exp $ */

#include "gdb.h"

#define GDB_RXBUFLEN 300


const char lnibble_to_hex_table[] = "0123456789abcdef";


/*
  Converts '[0-9,a-f,A-F]'
  to its long integer equivalent.
*/
long hex_to_long ( char h )
{
  if( h >= 'a' && h <= 'f' )
    return h - 'a' + 10;

  if( h >= '0' && h <= '9' )
    return h - '0';

  if( h >= 'A' && h <= 'F' )
    return h - 'A' + 10;

  return 0;
}


/*
  Converts the low nibble of i
  to its hex character equivalent.
*/
char lnibble_to_hex ( char i )
{
  return lnibble_to_hex_table[i & 0xf];
}


/*
  Converts an arbitrary number of hex
  digits to a long integer.
*/
long hexbuf_to_long ( short len, const char *hexbuf )
{
  int retval = 0;


  while( len-- )
    retval = ( retval << 4 ) + hex_to_long( *hexbuf++ );

  return retval;
}


/*
  Converts a long integer into a string of hex bytes.

  If pad is zero, then we only consume the exact number of
  bytes in hexbuf that we need; there will be no leading zeros.

  If pad is nonzero, we consume exactly pad bytes (up to 8),
  of hexbuf, left-padding hexbuf with zeros if necessary,
  and ignoring the most significant bits of l if necessary.

  We return the number of bytes in hexbuf consumed.
*/
short long_to_hexbuf ( long l, char *hexbuf, short pad )
{
  short consumed = 0;
  signed short bits;


  if( pad )
    bits = ( pad * sizeof( long )) - sizeof( long );
  else
    bits = sizeof( long ) * 8 - 4;

  do {

    if(( *hexbuf = lnibble_to_hex( l >> bits )) != '0'
       || pad || consumed ) {

      hexbuf++;
      consumed++;
    }

    bits -= 4;

  } while( bits >= 0 );

  /* always consume at least one byte, even if l was zero */
  return consumed ? consumed : 1;
}


/*
*/
unsigned char gdb_putstr ( short len, const char *buf )
{
  unsigned char sum = 0;


  while( len-- ) {
    sum += *buf;
    gdb_putc( *buf++ );
  }

  return sum;
}


/*
 */
void gdb_putmsg ( char c, const char *buf, short len )
{
  unsigned char sum;


  do {

    /* send the header */
    gdb_putc( '$' );

    /* send the message type, if specified */
    if( c )
      gdb_putc( c );

    /* send the data */
    sum = c + gdb_putstr( len, buf );

    /* send the footer */
    gdb_putc( '#' );
    gdb_putc( lnibble_to_hex( sum >> 4 ));
    gdb_putc( lnibble_to_hex( sum ));

  } while( '+' != gdb_getc() );

  return;
}


/*
*/
short gdb_getmsg ( char *rxbuf )
{
  char c;
  unsigned char sum;
  unsigned char rx_sum;
  char *buf;


 get_msg:

  /* wait around for start character, ignore all others */
  while( gdb_getc() != '$' );

  /* start counting bytes */
  buf = rxbuf;
  sum = 0;

  /* read until we see the '#' at the end of the packet */
  do {

    *buf++ = c = gdb_getc();

    if( c != '#' ) {
      sum += c;
    }

    /* since the buffer is ascii,
       may as well terminate it */
    *buf = 0;

  } while( c != '#' );

  /* receive checksum */
  rx_sum = hex_to_long( gdb_getc());
  rx_sum = ( rx_sum << 4 ) + hex_to_long( gdb_getc() );

  /* if computed checksum doesn't
     match received checksum, then reject */
  if( sum != rx_sum ) {

    gdb_putc( '-' );
    goto get_msg;
  }

  else {
    /* got the message ok */
    gdb_putc( '+' );
  }

  return 1;
}


/*
  "last signal" message

  "Sxx", where:
  xx is the signal number

*/
void gdb_last_signal ( short sigval )
{
  char tx_buf[2];


  gdb_putmsg( 'S', tx_buf,
    long_to_hexbuf( sigval, tx_buf, 2 ));

  return;
}


/*
  "expedited response" message
  "Txx..........."

*/
void gdb_expedited ( short sigval )
{
  char tx_buf[sizeof( long ) * 2];
  long val;
  short id = 0;
  short reglen;
  unsigned char sum;


  do {

    /* send header */
    gdb_putc( '$' );
    sum = gdb_putstr( 1, "T" );

    /* signal number */
    sum += gdb_putstr( long_to_hexbuf( sigval, tx_buf, 2 ), tx_buf );

    /* register values */
    while(( reglen = gdb_peek_register_file( id, &val )) != 0 ) {

      /* register id */
      sum += gdb_putstr( long_to_hexbuf( id, tx_buf, 0 ), tx_buf );
      sum += gdb_putstr( 1, ":" );

      /* register value
        (gdb 4.18 requires all 32 bits in values) */
      sum += gdb_putstr( long_to_hexbuf( val, tx_buf, reglen * 2 ), tx_buf );
      sum += gdb_putstr( 1, ";" );

      /* try the next register */
      id++;
    }

    /* send the message footer */
    gdb_putc( '#' );
    gdb_putc( lnibble_to_hex( sum >> 4 ));
    gdb_putc( lnibble_to_hex( sum ));

  } while( '+' != gdb_getc() );

  return;
}


/*
 */
void gdb_read_memory ( const char *hargs )
{
  char tx_buf[sizeof( long ) * 2];
  long addr = 0;
  long len = 0;
  unsigned char sum;


  /* parse address */
  while( *hargs != ',' )
    addr = ( addr << 4 ) + hex_to_long( *hargs++ );

  /* skip ',' */
  hargs++;

  /* parse length */
  while( *hargs != '#' )
    len = ( len << 4 ) + hex_to_long( *hargs++ );

  /* skip '#' */
  hargs++;

  /* send header */
  gdb_putc( '$' );
  sum = 0;

  while( len ) {

    /* read in long units where we can
       (this is important if the address is
       a peripheral register or other memory
       location that only supports long accesses) */
    while( len >= sizeof( long )
      && ( addr % sizeof( long ) == 0 )) {

      sum += gdb_putstr( long_to_hexbuf( *(long*)addr, tx_buf,
        sizeof( long ) * 2 ), tx_buf );

      addr += sizeof( long );
      len -= sizeof( long );
    }

    /* read in short units where we can
       (same reasons as above) */
    if( len >= sizeof( short )
      && ( addr % sizeof( short ) == 0 )) {

      sum += gdb_putstr( long_to_hexbuf( *(short*)addr, tx_buf,
        sizeof( short ) * 2 ), tx_buf );

      addr += sizeof( short );
      len -= sizeof( short );
    }

    if( len == 1
      || ( len && ( addr % sizeof( short ) != 0 ))) {

      /* request is totally misaligned;
        read a byte, and hope for the best */
      sum += gdb_putstr( long_to_hexbuf( *(char*)addr, tx_buf,
        sizeof( char ) * 2 ), tx_buf );

      addr += sizeof( char );
      len -= sizeof( char );
    }
  }

  /* send the message footer */
  gdb_putc( '#' );
  gdb_putc( lnibble_to_hex( sum >> 4 ));
  gdb_putc( lnibble_to_hex( sum ));

  /* get response from gdb (ignore it for now) */
  /* TODO: check gdb response in gdb_read_memory */
  gdb_getc();

  return;
}


/*
 */
void gdb_write_memory ( const char *hargs )
{
  long addr = 0;
  long len = 0;

  /* parse address */
  while( *hargs != ',' )
    addr = ( addr << 4 ) + hex_to_long( *hargs++ );

  /* skip ',' */
  hargs++;

  /* parse length */
  while( *hargs != ':' )
    len = ( len << 4 ) + hex_to_long( *hargs++ );

  /* skip ':' */
  hargs++;

  while (len)
  {
    /* write in long units where we can
      (this is important if the target is
      a peripheral register or other memory
      location that only supports long accesses) */
    while( len >= sizeof( long )
      && addr % sizeof( long ) == 0 ) {

      *(long*)addr = hexbuf_to_long( sizeof( long ) * 2, hargs );
      hargs += sizeof( long ) * 2;
      addr += sizeof( long );
      len -= sizeof( long );
    }

    /* write in short units where we can (same reasons as above) */
    if( len >= sizeof( short )
        && addr % sizeof( short ) == 0 ) {

      *(short*)addr = hexbuf_to_long( sizeof( short ) * 2, hargs );
      hargs += sizeof( short ) * 2;
      addr += sizeof( short );
      len -= sizeof( short );
    }

    if( len == 1
        || ( len && addr % sizeof( short ) != 0 )) {

      /* request is totally misaligned;
        write a byte, and hope for the best */

      *(char*)addr = hexbuf_to_long( sizeof( char ) * 2, hargs );
      hargs += sizeof( char ) * 2;
      addr += sizeof( char );
      len -= sizeof( char );
    }
  }

  gdb_putmsg( 0, "OK", 2 );
  return;
}


/*
 */
void gdb_console_output( short len, const char *buf )
{
  char tx_buf[2];
  unsigned char sum;


  gdb_putc( '$' );
  sum = gdb_putstr( 1, "O" );

  while( len-- ) {
    tx_buf[0] = lnibble_to_hex( *buf >> 4 );
    tx_buf[1] = lnibble_to_hex( *buf++ );

    sum += gdb_putstr( 2, tx_buf );
  }

  /* send the message footer */
  gdb_putc( '#' );
  gdb_putc( lnibble_to_hex( sum >> 4 ));
  gdb_putc( lnibble_to_hex( sum ));

  /* DON'T wait for response; we don't want to get hung
     up here and halt the application if gdb has gone away! */

  return;
}

/*
 */
void gdb_write_registers ( char *hargs )
{
  short id = 0;
  long val;
  short reglen;


  while( *hargs != '#' ) {

    /* how big is this register? */
    reglen = gdb_peek_register_file( id, &val );

    if( reglen ) {

      /* extract the register's value */
      val = hexbuf_to_long( reglen * 2, hargs );
      hargs += sizeof( long ) * 2;

      /* stuff it into the register file */
      gdb_poke_register_file( id++, val );
    }

    else break;
  }

  gdb_putmsg( 0, "OK", 2 );

  return;
}


/*
 */
void gdb_read_registers ( char *hargs )
{
  char tx_buf[sizeof( long ) * 2];
  long val;
  short id = 0;
  short reglen;
  unsigned char sum;


  do {

    gdb_putc( '$' );
    sum = 0;

    /* send register values */
    while(( reglen = gdb_peek_register_file( id++, &val ) ) != 0 )
      sum += gdb_putstr( long_to_hexbuf( val, tx_buf, reglen * 2 ), tx_buf );

    /* send the message footer */
    gdb_putc( '#' );
    gdb_putc( lnibble_to_hex( sum >> 4 ));
    gdb_putc( lnibble_to_hex( sum ));

  } while( '+' != gdb_getc() );

  return;
}


/*
 */
void gdb_write_register ( char *hargs )
{
  long id = 0;
  long val = 0;


  while( *hargs != '=' )
    id = ( id << 4 ) + hex_to_long( *hargs++ );

  hargs++;

  while( *hargs != '#' )
    val = ( val << 4 ) + hex_to_long( *hargs++ );

  gdb_poke_register_file( id, val );

  gdb_putmsg( 0, "OK", 2 );

  return;
}


/*
  The gdb command processor.
*/
void gdb_monitor ( short sigval )
{
  char rxbuf[GDB_RXBUFLEN];
  char *hargs;


  while( 1 ) {

    gdb_getmsg( rxbuf );

    hargs = rxbuf;
    switch( *hargs++ ) {

    case '?' : /* send last signal */

      gdb_last_signal( sigval );
      break;


    case 'c' : /* continue (address optional) */

      gdb_continue( hargs );

      /* exit back to interrupted code */
      return;


    case 'g' :

      gdb_read_registers( hargs );
      break;


    case 'G' :

      gdb_write_registers( hargs );
      break;


    case 'H' :

      /* set thread---
        unimplemented, but gdb wants it */
      gdb_putmsg( 0, "OK", 2 );
      break;


    case 'k' : /* kill program */

      /* can't put out OK because gdb doesn't respond with + */
      /*gdb_putmsg( 0, "OK", 2 );*/

      gdb_kill();
      break;


    case 'm' :

      gdb_read_memory( hargs );
      break;


    case 'M' :

      gdb_write_memory( hargs );
      break;


    case 'P':

      gdb_write_register( hargs );
      break;


    case 'q' : /* query */

      /* TODO: finish query command in gdb_handle_exception. */

      /* for now, only respond to "Offsets" query */
      gdb_putmsg( 0, "Text=0;Data=0;Bss=0", 19 );

      break;


    case 's' : /* step (address optional) */

      gdb_step( hargs );

      /* exit back to interrupted code */
      return;


    default :

      /* received a command we don't recognize---
        send empty response per gdb spec */
      gdb_putmsg( 0, "", 0 );

    }
  }

  return;
}


/*
*/
void gdb_handle_exception( long sigval )
{
  /* tell the host why we're here */
  gdb_expedited( sigval );

  /* ask gdb what to do next */
  gdb_monitor( sigval );

  /* return to the interrupted code */
  gdb_return_from_exception();
  /* never return to here!! */
}

void gdb_cout(const char *buf)
{
  long len = 0;
  char *pCh;

  for (pCh = (char *)buf; *pCh != 0; pCh++)
  {
    len++;
  }

  gdb_console_output(len, buf);
}
