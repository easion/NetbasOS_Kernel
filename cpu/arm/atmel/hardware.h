/*
 * linux/include/asm-arm/arch-at91/hardware.h
 *
 *  Copyright (C) 2003 SAN People
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H


#include "AT91RM9200.h"

#define AT91C_MAIN_CLOCK  179712000  /* from 18.432 MHz crystal (18432000 / 4 * 39) */
#define AT91C_MASTER_CLOCK  59904000  /* peripheral clock (AT91C_MASTER_CLOCK / 3) */
/* Virtual and Physical base address for system peripherals */
#define AT91_SYS_BASE		0xFFFFF000 /*4K */

/* Virtual and Physical base addresses of user peripherals */
#define AT91_SPI_BASE		0xFFFE0000 /*16K */
#define AT91_SSC2_BASE		0xFFFD8000 /*16K */
#define AT91_SSC1_BASE		0xFFFD4000 /*16K */
#define AT91_SSC0_BASE		0xFFFD0000 /*16K */
#define AT91_USART3_BASE	0xFFFCC000 /*16K */
#define AT91_USART2_BASE	0xFFFC8000 /*16K */
#define AT91_USART1_BASE	0xFFFC4000 /*16K */
#define AT91_USART0_BASE	0xFFFC0000 /*16K */
#define AT91_EMAC_BASE		0xFFFBC000 /*16K */
#define AT91_TWI_BASE		0xFFFB8000 /*16K */
#define AT91_MCI_BASE		0xFFFB4000 /*16K */
#define AT91_UDP_BASE		0xFFFB0000 /*16K */
#define AT91_TCB1_BASE		0xFFFA4000 /*16K */
#define AT91_TCB0_BASE		0xFFFA0000 /*16K */

/*
 * Where in virtual memory the IO devices (timers, system controllers
 * and so on)
 */
#define AT91_IO_BASE		0xF0000000	/* Virt/Phys Address of IO */

/* FLASH */
#define AT91_FLASH_BASE		0x10000000	/* NCS0 */

/* SDRAM */
#define AT91_SDRAM_BASE		0x20000000	/* NCS1 */

/* SmartMedia */
#define AT91_SMARTMEDIA_BASE	0x40000000	/* NCS3 */

/* Definition of interrupt priority levels */
#define AT91C_AIC_PRIOR_0 AT91C_AIC_PRIOR_LOWEST
#define AT91C_AIC_PRIOR_1 ((unsigned int) 0x1)
#define AT91C_AIC_PRIOR_2 ((unsigned int) 0x2)
#define AT91C_AIC_PRIOR_3 ((unsigned int) 0x3)
#define AT91C_AIC_PRIOR_4 ((unsigned int) 0x4)
#define AT91C_AIC_PRIOR_5 ((unsigned int) 0x5)
#define AT91C_AIC_PRIOR_6 ((unsigned int) 0x6)
#define AT91C_AIC_PRIOR_7 AT91C_AIC_PRIOR_HIGEST
#define DECLARE_GLOBAL_DATA_PTR     register volatile void *gd asm ("r8")
#define CFG_AT91C_BRGR_DIVISOR	33	/* hardcode so no __divsi3 : AT91C_MASTER_CLOCK / baudrate / 16 */

#define CFG_SPI_WRITE_TOUT	CFG_HZ
#define CFG_MAX_DATAFLASH_BANKS 2
#define CFG_MAX_DATAFLASH_PAGES 16384
#define CFG_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000	/* Logical adress for CS0 */
#define CFG_DATAFLASH_LOGIC_ADDR_CS3	0xD0000000	/* Logical adress for CS3 */


typedef struct {
	unsigned long base;		/* logical base address for a bank */
	unsigned long size;		/* total bank size */
	unsigned long page_count;
	unsigned long page_size;
	unsigned long id;		/* device id */
	unsigned char protect[CFG_MAX_DATAFLASH_PAGES]; /* page protection status */
} dataflash_info_t;


typedef unsigned int AT91S_DataFlashStatus;

/*----------------------------------------------------------------------*/
/* DataFlash Structures							*/
/*----------------------------------------------------------------------*/

/*---------------------------------------------*/
/* DataFlash Descriptor Structure Definition   */
/*---------------------------------------------*/
typedef struct _AT91S_DataflashDesc {
	unsigned char *tx_cmd_pt;
	unsigned int tx_cmd_size;
	unsigned char *rx_cmd_pt;
	unsigned int rx_cmd_size;
	unsigned char *tx_data_pt;
	unsigned int tx_data_size;
	unsigned char *rx_data_pt;
	unsigned int rx_data_size;
	volatile unsigned char state;
	volatile unsigned char DataFlash_state;
	unsigned char command[8];
} AT91S_DataflashDesc, *AT91PS_DataflashDesc;

/*---------------------------------------------*/
/* DataFlash device definition structure       */
/*---------------------------------------------*/
typedef struct _AT91S_Dataflash {
	int pages_number;			/* dataflash page number */
	int pages_size;				/* dataflash page size */
	int page_offset;			/* page offset in command */
	int byte_mask;				/* byte mask in command */
	int cs;
} AT91S_DataflashFeatures, *AT91PS_DataflashFeatures;

/*---------------------------------------------*/
/* DataFlash Structure Definition	       */
/*---------------------------------------------*/
typedef struct _AT91S_DataFlash {
	AT91PS_DataflashDesc pDataFlashDesc;	/* dataflash descriptor */
	AT91PS_DataflashFeatures pDevice;	/* Pointer on a dataflash features array */
} AT91S_DataFlash, *AT91PS_DataFlash;


typedef struct _AT91S_DATAFLASH_INFO {

	AT91S_DataflashDesc Desc;
	AT91S_DataflashFeatures Device; /* Pointer on a dataflash features array */
	unsigned long logical_address;
	unsigned int id;			/* device id */
	unsigned char protect[CFG_MAX_DATAFLASH_PAGES]; /* page protection status */
} AT91S_DATAFLASH_INFO, *AT91PS_DATAFLASH_INFO;


/*-------------------------------------------------------------------------------------------------*/

#define AT45DB161	0x2c
#define AT45DB321	0x34
#define AT45DB642	0x3c

#define AT91C_DATAFLASH_TIMEOUT		10000	/* For AT91F_DataFlashWaitReady */

/* DataFlash return value */
#define DATAFLASH_BUSY			0x00
#define DATAFLASH_OK			0x01
#define DATAFLASH_ERROR			0x02
#define DATAFLASH_MEMORY_OVERFLOW	0x03
#define DATAFLASH_BAD_COMMAND		0x04
#define DATAFLASH_BAD_ADDRESS		0x05


/* Driver State */
#define IDLE		0x0
#define BUSY		0x1
#define ERROR		0x2

/* DataFlash Driver State */
#define GET_STATUS	0x0F

/*-------------------------------------------------------------------------------------------------*/
/* Command Definition										   */
/*-------------------------------------------------------------------------------------------------*/

/* READ COMMANDS */
#define DB_CONTINUOUS_ARRAY_READ	0xE8	/* Continuous array read */
#define DB_BURST_ARRAY_READ		0xE8	/* Burst array read */
#define DB_PAGE_READ			0xD2	/* Main memory page read */
#define DB_BUF1_READ			0xD4	/* Buffer 1 read */
#define DB_BUF2_READ			0xD6	/* Buffer 2 read */
#define DB_STATUS			0xD7	/* Status Register */

/* PROGRAM and ERASE COMMANDS */
#define DB_BUF1_WRITE			0x84	/* Buffer 1 write */
#define DB_BUF2_WRITE			0x87	/* Buffer 2 write */
#define DB_BUF1_PAGE_ERASE_PGM		0x83	/* Buffer 1 to main memory page program with built-In erase */
#define DB_BUF1_PAGE_ERASE_FASTPGM	0x93	/* Buffer 1 to main memory page program with built-In erase, Fast program */
#define DB_BUF2_PAGE_ERASE_PGM		0x86	/* Buffer 2 to main memory page program with built-In erase */
#define DB_BUF2_PAGE_ERASE_FASTPGM	0x96	/* Buffer 1 to main memory page program with built-In erase, Fast program */
#define DB_BUF1_PAGE_PGM		0x88	/* Buffer 1 to main memory page program without built-In erase */
#define DB_BUF1_PAGE_FASTPGM		0x98	/* Buffer 1 to main memory page program without built-In erase, Fast program */
#define DB_BUF2_PAGE_PGM		0x89	/* Buffer 2 to main memory page program without built-In erase */
#define DB_BUF2_PAGE_FASTPGM		0x99	/* Buffer 1 to main memory page program without built-In erase, Fast program */
#define DB_PAGE_ERASE			0x81	/* Page Erase */
#define DB_BLOCK_ERASE			0x50	/* Block Erase */
#define DB_PAGE_PGM_BUF1		0x82	/* Main memory page through buffer 1 */
#define DB_PAGE_FASTPGM_BUF1		0x92	/* Main memory page through buffer 1, Fast program */
#define DB_PAGE_PGM_BUF2		0x85	/* Main memory page through buffer 2 */
#define DB_PAGE_FastPGM_BUF2		0x95	/* Main memory page through buffer 2, Fast program */

/* ADDITIONAL COMMANDS */
#define DB_PAGE_2_BUF1_TRF		0x53	/* Main memory page to buffer 1 transfert */
#define DB_PAGE_2_BUF2_TRF		0x55	/* Main memory page to buffer 2 transfert */
#define DB_PAGE_2_BUF1_CMP		0x60	/* Main memory page to buffer 1 compare */
#define DB_PAGE_2_BUF2_CMP		0x61	/* Main memory page to buffer 2 compare */
#define DB_AUTO_PAGE_PGM_BUF1		0x58	/* Auto page rewrite throught buffer 1 */
#define DB_AUTO_PAGE_PGM_BUF2		0x59	/* Auto page rewrite throught buffer 2 */

/*-------------------------------------------------------------------------------------------------*/

extern int addr_dataflash (unsigned long addr);
extern int read_dataflash (unsigned long addr, unsigned long size, char *result);
extern int write_dataflash (unsigned long addr, unsigned long dest, unsigned long size);
extern void dataflash_print_info (void);

#endif
