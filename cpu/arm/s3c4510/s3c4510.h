/*
 * File      : s3c4510.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-03-13     Bernard      first version
 */

#ifndef __S3C4510_H__
#define __S3C4510_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup S3C4510
 */
/*@{*/

/*------------------------------------------------------------------------
 *	  ASIC Address Definition
 *----------------------------------------------------------------------*/

#define S3C_REG    *(volatile unsigned int *)

/* Special Register Start Address After System Reset */
#define _SPSTR_RESET    (S3C_REG(0x1000000))

#define S3C4510_BASE_ADDR		0x3ff0000
#define S3C4510_SRAM_ADDR		0x3fe0000
#define INTADDR 		(Reset_Addr+0x20)		
#define SPSTR      		(S3C_REG(S3C4510_BASE_ADDR))

/* *********************** */
/* System Manager Register */
/* *********************** */
#define SYSCFG			(S3C_REG(S3C4510_BASE_ADDR+0x0000))

#define CLKCON			(S3C_REG(S3C4510_BASE_ADDR+0x3000))
#define EXTACON0		(S3C_REG(S3C4510_BASE_ADDR+0x3008))
#define EXTACON1		(S3C_REG(S3C4510_BASE_ADDR+0x300c))
#define EXTDBWTH		(S3C_REG(S3C4510_BASE_ADDR+0x3010))
#define ROMCON0			(S3C_REG(S3C4510_BASE_ADDR+0x3014))
#define ROMCON1			(S3C_REG(S3C4510_BASE_ADDR+0x3018))
#define ROMCON2			(S3C_REG(S3C4510_BASE_ADDR+0x301c))
#define ROMCON3			(S3C_REG(S3C4510_BASE_ADDR+0x3020))
#define ROMCON4			(S3C_REG(S3C4510_BASE_ADDR+0x3024))
#define ROMCON5			(S3C_REG(S3C4510_BASE_ADDR+0x3028))
#define DRAMCON0		(S3C_REG(S3C4510_BASE_ADDR+0x302c))
#define DRAMCON1		(S3C_REG(S3C4510_BASE_ADDR+0x3030))
#define DRAMCON2		(S3C_REG(S3C4510_BASE_ADDR+0x3034))
#define DRAMCON3		(S3C_REG(S3C4510_BASE_ADDR+0x3038))
#define REFEXTCON		(S3C_REG(S3C4510_BASE_ADDR+0x303c))

/* *********************** */
/* Ethernet BDMA Register  */
/* *********************** */
#define BDMATXCON		(S3C_REG(S3C4510_BASE_ADDR+0x9000))
#define BDMARXCON		(S3C_REG(S3C4510_BASE_ADDR+0x9004))
#define BDMATXPTR		(S3C_REG(S3C4510_BASE_ADDR+0x9008))
#define BDMARXPTR		(S3C_REG(S3C4510_BASE_ADDR+0x900c))
#define BDMARXLSZ		(S3C_REG(S3C4510_BASE_ADDR+0x9010))
#define BDMASTAT		(S3C_REG(S3C4510_BASE_ADDR+0x9014))

/* Content Address Memory */
#define CAM_BaseAddr	(S3C4510_BASE_ADDR + 0x9100)
#define CAM_BASE		(S3C_REG(S3C4510_BASE_ADDR+0x9100))
#define CAM_Reg(x)		(S3C_REG(CAM_BaseAddr+(x*0x4)))

#define BDMATXBUF		(S3C_REG(S3C4510_BASE_ADDR+0x9200))
#define BDMARXBUF		(S3C_REG(S3C4510_BASE_ADDR+0x9800))

/* *********************** */
/* Ethernet MAC Register   */
/* *********************** */
#define MACCON			(S3C_REG(S3C4510_BASE_ADDR+0xa000))
#define CAMCON			(S3C_REG(S3C4510_BASE_ADDR+0xa004))
#define MACTXCON		(S3C_REG(S3C4510_BASE_ADDR+0xa008))
#define MACTXSTAT		(S3C_REG(S3C4510_BASE_ADDR+0xa00c))
#define MACRXCON		(S3C_REG(S3C4510_BASE_ADDR+0xa010))
#define MACRXSTAT		(S3C_REG(S3C4510_BASE_ADDR+0xa014))
#define STADATA			(S3C_REG(S3C4510_BASE_ADDR+0xa018))
#define STACON			(S3C_REG(S3C4510_BASE_ADDR+0xa01c))
#define CAMEN			(S3C_REG(S3C4510_BASE_ADDR+0xa028))
#define EMISSCNT		(S3C_REG(S3C4510_BASE_ADDR+0xa03c))
#define EPZCNT			(S3C_REG(S3C4510_BASE_ADDR+0xa040))
#define ERMPZCNT		(S3C_REG(S3C4510_BASE_ADDR+0xa044))
#define ETXSTAT			(S3C_REG(S3C4510_BASE_ADDR+0x9040))
#define MACRXDESTR		(S3C_REG(S3C4510_BASE_ADDR+0xa064))
#define MACRXSTATEM		(S3C_REG(S3C4510_BASE_ADDR+0xa090))
#define MACRXFIFO		(S3C_REG(S3C4510_BASE_ADDR+0xa200))

/**************************************************/
/* KS32C50100 : HDLC Channel A                    */
/**************************************************/
#define HMODEA 			(S3C_REG(S3C4510_BASE_ADDR+0x7000))
#define HCONA 			(S3C_REG(S3C4510_BASE_ADDR+0x7004))
#define HSTATA  		(S3C_REG(S3C4510_BASE_ADDR+0x7008))
#define HINTENA 		(S3C_REG(S3C4510_BASE_ADDR+0x700c))
#define HTXFIFOCA 		(S3C_REG(S3C4510_BASE_ADDR+0x7010))
#define HTXFIFOTA 		(S3C_REG(S3C4510_BASE_ADDR+0x7014))
#define HRXFIFOA 		(S3C_REG(S3C4510_BASE_ADDR+0x7018))
#define HBRGTCA			(S3C_REG(S3C4510_BASE_ADDR+0x701c))
#define HPRMBA	 		(S3C_REG(S3C4510_BASE_ADDR+0x7020))
#define HSAR0A 			(S3C_REG(S3C4510_BASE_ADDR+0x7024))
#define HSAR1A	 		(S3C_REG(S3C4510_BASE_ADDR+0x7028))
#define HSAR2A	 		(S3C_REG(S3C4510_BASE_ADDR+0x702c))
#define HSAR3A	 		(S3C_REG(S3C4510_BASE_ADDR+0x7030))
#define HMASKA 			(S3C_REG(S3C4510_BASE_ADDR+0x7034))
#define HDMATXPTRA 		(S3C_REG(S3C4510_BASE_ADDR+0x7038))
#define HDMARXPTRA 		(S3C_REG(S3C4510_BASE_ADDR+0x703c))
#define HMFLRA 			(S3C_REG(S3C4510_BASE_ADDR+0x7040))
#define HRBSRA 			(S3C_REG(S3C4510_BASE_ADDR+0x7044))
#define HDLCBaseAddr	(S3C4510_BASE_ADDR+0x7000)
	
/**************************************************/
/* KS32C50100 : HDLC Channel B                    */
/**************************************************/
#define HMODEB 			(S3C_REG(S3C4510_BASE_ADDR+0x8000))
#define HCONB 			(S3C_REG(S3C4510_BASE_ADDR+0x8004))
#define HSTATB  		(S3C_REG(S3C4510_BASE_ADDR+0x8008))
#define HINTENB 		(S3C_REG(S3C4510_BASE_ADDR+0x800c))
#define HTXFIFOCB 		(S3C_REG(S3C4510_BASE_ADDR+0x8010))
#define HTXFIFOTB 		(S3C_REG(S3C4510_BASE_ADDR+0x8014))
#define HRXFIFOB 		(S3C_REG(S3C4510_BASE_ADDR+0x8018))
#define HBRGTCB			(S3C_REG(S3C4510_BASE_ADDR+0x801c))
#define HPRMBB	 		(S3C_REG(S3C4510_BASE_ADDR+0x8020))
#define HSAR0B 			(S3C_REG(S3C4510_BASE_ADDR+0x8024))
#define HSAR1B	 		(S3C_REG(S3C4510_BASE_ADDR+0x8028))
#define HSAR2B	 		(S3C_REG(S3C4510_BASE_ADDR+0x802c))
#define HSAR3B	 		(S3C_REG(S3C4510_BASE_ADDR+0x8030))
#define HMASKB 			(S3C_REG(S3C4510_BASE_ADDR+0x8034))
#define HDMATXPTRB 		(S3C_REG(S3C4510_BASE_ADDR+0x8038))
#define HDMARXPTRB 		(S3C_REG(S3C4510_BASE_ADDR+0x803c))
#define HMFLRB 			(S3C_REG(S3C4510_BASE_ADDR+0x8040))
#define HRBSRB 			(S3C_REG(S3C4510_BASE_ADDR+0x8044))

/********************/
/* I2C Bus Register */
/********************/
#define IICCON	 		(S3C_REG(S3C4510_BASE_ADDR+0xf000))
#define IICBUF	 		(S3C_REG(S3C4510_BASE_ADDR+0xf004))
#define IICPS	 		(S3C_REG(S3C4510_BASE_ADDR+0xf008))
#define IICCOUNT 		(S3C_REG(S3C4510_BASE_ADDR+0xf00c))

/********************/
/*    GDMA 0        */
/********************/
#define GDMACON0		(S3C_REG(S3C4510_BASE_ADDR+0xb000))
#define GDMA0_RUN_ENABLE (S3C_REG(S3C4510_BASE_ADDR+0xb020))
#define GDMASRC0		(S3C_REG(S3C4510_BASE_ADDR+0xb004))
#define GDMADST0		(S3C_REG(S3C4510_BASE_ADDR+0xb008))
#define GDMACNT0		(S3C_REG(S3C4510_BASE_ADDR+0xb00c))

/********************/
/*    GDMA 1        */
/********************/
#define GDMACON1		(S3C_REG(S3C4510_BASE_ADDR+0xc000))
#define GDMA1_RUN_ENABLE (S3C_REG(S3C4510_BASE_ADDR+0xc020))
#define GDMASRC1		(S3C_REG(S3C4510_BASE_ADDR+0xc004))
#define GDMADST1		(S3C_REG(S3C4510_BASE_ADDR+0xc008))
#define GDMACNT1		(S3C_REG(S3C4510_BASE_ADDR+0xc00c))

/********************/
/*      UART 0      */
/********************/
#define UARTLCON0       (S3C_REG(S3C4510_BASE_ADDR+0xd000))
#define UARTCONT0       (S3C_REG(S3C4510_BASE_ADDR+0xd004))
#define UARTSTAT0       (S3C_REG(S3C4510_BASE_ADDR+0xd008))
#define UARTTXH0        (S3C_REG(S3C4510_BASE_ADDR+0xd00c))
#define UARTRXB0        (S3C_REG(S3C4510_BASE_ADDR+0xd010))
#define UARTBRD0        (S3C_REG(S3C4510_BASE_ADDR+0xd014))

/********************/
/*     UART 1       */
/********************/
#define UARTLCON1       (S3C_REG(S3C4510_BASE_ADDR+0xe000))
#define UARTCONT1       (S3C_REG(S3C4510_BASE_ADDR+0xe004))
#define UARTSTAT1       (S3C_REG(S3C4510_BASE_ADDR+0xe008))
#define UARTTXH1        (S3C_REG(S3C4510_BASE_ADDR+0xe00c))
#define UARTRXB1        (S3C_REG(S3C4510_BASE_ADDR+0xe010))
#define UARTBRD1        (S3C_REG(S3C4510_BASE_ADDR+0xe014))

/********************/
/*  Timer Register  */
/********************/
#define TMOD  	  		(S3C_REG(S3C4510_BASE_ADDR+0x6000))
#define TDATA0			(S3C_REG(S3C4510_BASE_ADDR+0x6004))
#define TDATA1			(S3C_REG(S3C4510_BASE_ADDR+0x6008))
#define TCNT0			(S3C_REG(S3C4510_BASE_ADDR+0x600c))
#define TCNT1			(S3C_REG(S3C4510_BASE_ADDR+0x6010))

/**********************/
/* I/O Port Interface */
/**********************/
#define IOPMOD	  		(S3C_REG(S3C4510_BASE_ADDR+0x5000))
#define IOPCON  		(S3C_REG(S3C4510_BASE_ADDR+0x5004))
#define IOPDATA 		(S3C_REG(S3C4510_BASE_ADDR+0x5008))

/*********************************/
/* Interrupt Controller Register */
/*********************************/
#define INTMODE			(S3C_REG(S3C4510_BASE_ADDR+0x4000))
#define INTPEND			(S3C_REG(S3C4510_BASE_ADDR+0x4004))
#define INTMASK			(S3C_REG(S3C4510_BASE_ADDR+0x4008))

#define INTPRI0			(S3C_REG(S3C4510_BASE_ADDR+0x400c))
#define INTPRI1			(S3C_REG(S3C4510_BASE_ADDR+0x4010))
#define INTPRI2			(S3C_REG(S3C4510_BASE_ADDR+0x4014))
#define INTPRI3			(S3C_REG(S3C4510_BASE_ADDR+0x4018))
#define INTPRI4			(S3C_REG(S3C4510_BASE_ADDR+0x401c))
#define INTPRI5			(S3C_REG(S3C4510_BASE_ADDR+0x4020))
#define INTOFFSET		(S3C_REG(S3C4510_BASE_ADDR+0x4024))
#define INTPNDPRI		(S3C_REG(S3C4510_BASE_ADDR+0x4028))
#define INTPNDTST		(S3C_REG(S3C4510_BASE_ADDR+0x402C))

#define INTE0			0
#define INTE1			1
#define INTE2			2
#define INTE3			3
#define INTUART0_TX		4
#define INTUART0_RX		5
#define INTUART1_TX		6
#define INTUART1_RX		7
#define INTGDMA0		8
#define INTGDMA1		9
#define INTTIMER0		10
#define INTTIMER1		11
#define INTHDLC0_TX		12
#define INTHDLC0_RX		13
#define INTHDLC1_TX		14
#define INTHDLC1_RX		15
#define INTETH_BDMA_TX	16
#define INTETH_BDMA_RX	17
#define INTETH_MAC_TX	18
#define INTETH_MAC_RX	19
#define INTIIC			20

#define INTGLOBAL		21

/*****************************/
/* CPU Mode                  */
/*****************************/
#define USERMODE		0x10
#define FIQMODE			0x11
#define IRQMODE			0x12
#define SVCMODE			0x13
#define ABORTMODE		0x17
#define UNDEFMODE		0x1b
#define MODEMASK		0x1f
#define NOINT			0xc0

struct arm_register
{
	unsigned long r0;
	unsigned long r1;
	unsigned long r2;
	unsigned long r3;
	unsigned long r4;
	unsigned long r5;
	unsigned long r6;
	unsigned long r7;
	unsigned long r8;
	unsigned long r9;
	unsigned long r10;
	unsigned long fp;
	unsigned long ip;
	unsigned long sp;
	unsigned long lr;
	unsigned long pc;
	unsigned long cpsr;
	unsigned long ORIG_r0;
};

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
