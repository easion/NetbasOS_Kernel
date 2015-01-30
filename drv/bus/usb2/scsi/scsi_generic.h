

#ifndef SCSI_GENERIC_H_
#define SCSI_GENERIC_H_

#include <scsi.h>

int scsi_generic_open( void *pNode, uint32 nFlags, void **ppCookie );
int scsi_generic_close( void *pNode, void *pCookie );
int scsi_generic_read( void *pNode, void *pCookie, off_t nPos, void *pBuffer, size_t nLen );
int scsi_generic_write( void *pNode, void *pCookie, off_t nPos, const void *pBuffer, size_t nLen );

#endif	/* SCSI_COMMON_H_ */

