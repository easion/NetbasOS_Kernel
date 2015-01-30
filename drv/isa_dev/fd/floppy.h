/* 
*Jicama OS  
 * Copyright (C) 2001-2003  DengPingPing      All rights reserved.   
 */
#ifndef FLOPPY_H
#define FLOPPY_H

#define FDC_DOR  0x3f2  /* Digital Output Register */
#define FDC_MSR  0x3f4   /* Main Status Register (input) */
#define FDC_DRS  0x3f4   /* Data Rate Select Register (output) */
#define FDC_DATA 0x3f5   /* Data Register */
#define FDC_DIR  0x3f7  /* Digital Input Register (input) */
#define FDC_CCR  0x3f7   /* Configuration Control Register (output) */

/* command bytes (these are 765 commands + options such as MFM, etc) */
#define CMD_SPECIFY (0x03)  /* specify drive timings */
#define CMD_WRITE   (0xc5)  /* write data (+ MT,MFM) */
#define CMD_READ    (0xe6)  /* read data (+ MT,MFM,SK) */
#define CMD_RECAL   (0x07)  /* recalibrate */
#define CMD_SENSEI  (0x08)  /* sense interrupt status */
#define CMD_FORMAT  (0x4d)  /* format track (+ MFM) */
#define CMD_SEEK    (0x0f)  /* seek track */
#define CMD_VERSION (0x10)  /* FDC version */

struct floppy_struct
{
    unsigned char detect;
    unsigned char tracks;
    unsigned char heads;
    unsigned char sectors;

	unsigned char request_sector;
	unsigned char* fd_buffer;
} fd[2];

extern int sendbyte( int value );
extern int getbyte( void );
extern void motor_on( void );
extern void fp_dma(int command, int secs, unsigned int dma_addr);
extern int floppy_read( void );


//int floppy_init( void );
#endif


