
/************************************************************
  Copyright (C), 2003-2010, Netbas OS Project.
  FileName: 
  Author:        Version :          Date:
  Description:    
  Version:        
  Function List:   
  History:         
      Easion   2010/2/6     1.0     build this moudle  
***********************************************************/
#include <drv/drv.h>
#include <drv/fs.h>
#include <drv/buffer.h>
#include <assert.h>
#include <errno.h>
#include <net/inet.h>
#include "minixfs.h"


unsigned conv2( w)
int w;				/* promotion of 16-bit word to be swapped */
{
/* Possibly swap a 16-bit word between 8086 and 68000 byte order. */

  //if (norm) 
	  return( (unsigned) w & 0xFFFF);
  //return( ((w&BYTE) << 8) | ( (w>>8) & BYTE));
}

long conv4( x)
long x;				/* 32-bit long to be byte swapped */
{
/* Possibly swap a 32-bit long between 8086 and 68000 byte order. */

  unsigned lo, hi;
  long l;
  
   return(x);			/* byte order was already ok */

#if 0
  lo = conv2( (int) x & 0xFFFF);	/* low-order half, byte swapped */
  hi = conv2( (int) (x>>16) & 0xFFFF);	/* high-order half, swapped */
  l = ( (long) lo <<16) | hi;
  return(l);
#endif
}




int minixfs_readlink(inode_t * inoptr, char *buff, size_t buf_size)
{
	off_t off;
	off_t block;
	buffer_t * bh;

	off = 0;
	while((off < buf_size) && (off < inoptr->i_size)){
		int len = BLOCKSIZ;

		if(len > inoptr->i_size - off)
			len = inoptr->i_size - off;

		if(len > buf_size - off)
			len = buf_size - off;

		block = read_map(inoptr, off);

		if(block == NOBLOCK)
			memset(&buff[off], 0, BLOCKSIZ);
		else{
			bh = bread(inoptr->i_super->m_dev, BLOCK_TO_SECTOR(block));
			if(!bh){
				iput(inoptr);
				return EIO;
			}

			memcpy(&buff[off], bh->b_data, len);
			buf_release(bh);
		}

		off += len;
	}

	return off;
}



