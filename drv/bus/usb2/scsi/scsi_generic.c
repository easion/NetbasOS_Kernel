
#include <scsi_common.h>
#include <scsi_generic.h>

//#undef DEBUG_LIMIT
//#define DEBUG_LIMIT   KERN_DEBUG_LOW

int scsi_generic_open( void *pNode, uint32 nFlags, void **ppCookie )
{
	SCSI_device_s *psDevice = pNode;
	int nError = 0;

	LOCK( psDevice->hLock );

	DEBUG_PRINT( "scsi_generic_open() called\n" );

	/* Discover if the drive is ready */
	nError = scsi_test_ready( psDevice );
	if( nError < 0 )
	{
		DEBUG_PRINT( "An error occured waiting for the device, return code 0x%02x\n", nError );
	}
	else
	{
		/* No harm in allowing the drive firmware a little time */
		snooze( 10000 );

		nError = scsi_read_capacity( psDevice, NULL );
		if( nError < 0 )
		{
			DEBUG_PRINT( "An error occured reading from the device, return code 0x%02x\n", nError );
		}
		else
			atomic_inc( &psDevice->nOpenCount );
	}

	UNLOCK( psDevice->hLock );

	return nError;
}

int scsi_generic_close( void *pNode, void *pCookie )
{
	SCSI_device_s *psDevice = pNode;

	DEBUG_PRINT( "scsi_generic_close() called\n" );

	LOCK( psDevice->hLock );
	atomic_dec( &psDevice->nOpenCount );
	UNLOCK( psDevice->hLock );
	return 0;
}

static int scsi_generic_do_read( SCSI_device_s * psDevice, off_t nPos, void *pBuffer, int nLen )
{
	uint64 nBlock;
	uint64 nBlockCount;
	SCSI_cmd sCmd;
	int nError;
	size_t nToRead = nLen;
	size_t nReadNow;

	/* The iso9660 fs requires this workaround... */
	if( nToRead < psDevice->nSectorSize )
		nToRead = psDevice->nSectorSize;

	while( nToRead > 0 )
	{
		if( nToRead > 0xffff )
			nReadNow = 0xffff;
		else
			nReadNow = nToRead;

		nBlock = nPos / psDevice->nSectorSize;
		nBlockCount = nReadNow / psDevice->nSectorSize;

		/* Build SCSI_READ_10 command */
		scsi_init_cmd( &sCmd, psDevice );

		sCmd.nDirection = SCSI_DATA_READ;
		sCmd.pRequestBuffer = psDevice->pDataBuffer;
		sCmd.nRequestSize = nReadNow;

		sCmd.nCmd[0] = SCSI_READ_10;
		if( psDevice->nSCSILevel <= SCSI_2 )
			sCmd.nCmd[1] = ( psDevice->nLun << 5 ) & 0xe0;
		else
			sCmd.nCmd[1] = 0;
		sCmd.nCmd[2] = ( unsigned char )( nBlock >> 24 ) & 0xff;
		sCmd.nCmd[3] = ( unsigned char )( nBlock >> 16 ) & 0xff;
		sCmd.nCmd[4] = ( unsigned char )( nBlock >> 8 ) & 0xff;
		sCmd.nCmd[5] = ( unsigned char )nBlock & 0xff;
		sCmd.nCmd[6] = sCmd.nCmd[9] = 0;
		sCmd.nCmd[7] = ( unsigned char )( nBlockCount >> 8 ) & 0xff;
		sCmd.nCmd[8] = ( unsigned char )nBlockCount & 0xff;

		sCmd.nCmdLen = scsi_get_command_size( SCSI_READ_10 );

		DEBUG_PRINT( "Reading block %i (%i)\n", ( int )nBlock, ( int )nBlockCount );

		/* Send command */
		nError = psDevice->psHost->queue_command( &sCmd );

		DEBUG_PRINT( "nError=%d\tsCmd.nResult=%d\n", nError, sCmd.nResult );

		if( nError != 0 || sCmd.nResult != 0 )
		{
			DEBUG_PRINT( "SCSI: Error while reading!\n" );
			return -EIO;
		}

		/* Copy data */
		if( nLen < psDevice->nSectorSize )
			memcpy( pBuffer, psDevice->pDataBuffer, nLen );
		else
			memcpy( pBuffer, psDevice->pDataBuffer, nReadNow );

		nPos += nReadNow;
		pBuffer += nReadNow;
		nToRead -= nReadNow;
	}

	return nLen;
}

int scsi_generic_read( void *pNode, void *pCookie, off_t nPos, void *pBuffer, size_t nLen )
{
	SCSI_device_s *psDevice = pNode;
	int nRet;

	DEBUG_PRINT( "scsi_generic_read() called\n" );

	LOCK( psDevice->hLock );

	nPos += psDevice->nStart;
	nRet = scsi_generic_do_read( psDevice, nPos, pBuffer, nLen );

	UNLOCK( psDevice->hLock );

	DEBUG_PRINT( "scsi_generic_read() completed: %d\n", nRet );

	return nRet;
}

static int scsi_generic_do_write( SCSI_device_s * psDevice, off_t nPos, const void *pBuffer, int nLen )
{
	int nToWrite = nLen;
	int nWriteNow = 0;
	uint64 nBlock;
	uint64 nBlockCount;
	SCSI_cmd sCmd;
	int nError = 0;

	while( nToWrite > 0 )
	{
		if( nToWrite > 0xffff )
			nWriteNow = 0xffff;
		else
			nWriteNow = nToWrite;

		nBlock = nPos / psDevice->nSectorSize;
		nBlockCount = nWriteNow / psDevice->nSectorSize;

		/* Build SCSI_WRITE_10 command */
		scsi_init_cmd( &sCmd, psDevice );

		sCmd.nDirection = SCSI_DATA_WRITE;
		sCmd.pRequestBuffer = (void*)pBuffer;
		sCmd.nRequestSize = nWriteNow;

		sCmd.nCmd[0] = SCSI_WRITE_10;
		if( psDevice->nSCSILevel <= SCSI_2 )
			sCmd.nCmd[1] = ( psDevice->nLun << 5 ) & 0xe0;
		else
			sCmd.nCmd[1] = 0;
		sCmd.nCmd[2] = ( unsigned char )( nBlock >> 24 ) & 0xff;
		sCmd.nCmd[3] = ( unsigned char )( nBlock >> 16 ) & 0xff;
		sCmd.nCmd[4] = ( unsigned char )( nBlock >> 8 ) & 0xff;
		sCmd.nCmd[5] = ( unsigned char )nBlock & 0xff;
		sCmd.nCmd[6] = sCmd.nCmd[9] = 0;
		sCmd.nCmd[7] = ( unsigned char )( nBlockCount >> 8 ) & 0xff;
		sCmd.nCmd[8] = ( unsigned char )nBlockCount & 0xff;

		sCmd.nCmdLen = scsi_get_command_size( SCSI_WRITE_10 );

		DEBUG_PRINT( "Writing block %i (%i)\n", ( int )nBlock, ( int )nBlockCount );

		/* Send command */
		nError = psDevice->psHost->queue_command( &sCmd );

		DEBUG_PRINT( "Result: %i\n", sCmd.nResult );

		if( nError != 0 || sCmd.nResult != 0 )
		{
			printk( "SCSI: Error while writing!\n" );
			return -EIO;
		}

		pBuffer = ( ( uint8 * )pBuffer ) + nWriteNow;
		nPos += nWriteNow;
		nToWrite -= nWriteNow;
	}
	return nLen;
}


int scsi_generic_write( void *pNode, void *pCookie, off_t nPos, const void *pBuffer, size_t nLen )
{
	SCSI_device_s *psDevice = pNode;
	int nRet;

	DEBUG_PRINT( "scsi_generic_write() called\n" );

	if( ( nPos & ( psDevice->nSectorSize - 1 ) ) || ( nLen & ( psDevice->nSectorSize - 1 ) ) )
	{
		printk( "SCSI: Invalid position requested\n" );
		return -EIO;
	}

	LOCK( psDevice->hLock );

	nPos += psDevice->nStart;
	nRet = scsi_generic_do_write( psDevice, nPos, pBuffer, nLen );

	UNLOCK( psDevice->hLock );

	DEBUG_PRINT( "scsi_generic_write() completed: %d\n", nRet );

	return nRet;
}

#if 0

LOCK(){}
UNLOCK(){}
create_semaphore(){}
//write_user_byte(){}
udelay(){}
atomic_inc(){}
#endif