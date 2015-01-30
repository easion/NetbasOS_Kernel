/*
  Copyright (c) 1999 by      William A. Gatliff
  All rights reserved.     bgat@open-widgets.com

  See the file COPYING for details.

  This file is provided "as-is", and without any express
  or implied warranties, including, without limitation,
  the implied warranties of merchantability and fitness
  for a particular purpose.

  The author welcomes feedback regarding this file.
*/

/* $Id: sh4-775x-rmap.h,v 1.3 2002/04/08 15:08:20 bgat Exp $ */

#if !defined( SH4_775X_RMAP_H_INCLUDED )
#define SH4_775X_RMAP_H_INCLUDED


/* SCI */

#define SH4_775X_SCI_SCSMR1_CA      (1<<7)
#define SH4_775X_SCI_SCSMR1_CHR     (1<<6)
#define SH4_775X_SCI_SCSMR1_PE      (1<<5)
#define SH4_775X_SCI_SCSMR1_OE      (1<<4)
#define SH4_775X_SCI_SCSMR1_STOP    (1<<3)
#define SH4_775X_SCI_SCSMR1_MP      (1<<2)
#define SH4_775X_SCI_SCSMR1_CKS1    (1<<1)
#define SH4_775X_SCI_SCSMR1_CKS0    (1<<0)

#define SH4_775X_SCI_SCSCR1_TIE     (1<<7)
#define SH4_775X_SCI_SCSCR1_RIE     (1<<6)
#define SH4_775X_SCI_SCSCR1_TE      (1<<5)
#define SH4_775X_SCI_SCSCR1_RE      (1<<4)
#define SH4_775X_SCI_SCSCR1_MPIE    (1<<3)
#define SH4_775X_SCI_SCSCR1_TEIE    (1<<2)
#define SH4_775X_SCI_SCSCR1_CKE1    (1<<1)
#define SH4_775X_SCI_SCSCR1_CKE0    (1<<0)

#define SH4_775X_SCI_SCSSR1_TDRE    (1<<7)
#define SH4_775X_SCI_SCSSR1_RDRF    (1<<6)
#define SH4_775X_SCI_SCSSR1_ORER    (1<<5)
#define SH4_775X_SCI_SCSSR1_FER     (1<<4)
#define SH4_775X_SCI_SCSSR1_PER     (1<<3)
#define SH4_775X_SCI_SCSSR1_TEND    (1<<2)
#define SH4_775X_SCI_SCSSR1_MPB     (1<<1)
#define SH4_775X_SCI_SCSSR1_MPBT    (1<<0)

#define SH4_775X_SCI_SCSPTR1_EIO    (1<<7)
#define SH4_775X_SCI_SCSPTR1_SPB1IO (1<<3)
#define SH4_775X_SCI_SCSPTR1_SPB1DT (1<<2)
#define SH4_775X_SCI_SCSPTR1_SPB0IO (1<<1)
#define SH4_775X_SCI_SCSPTR1_SPB0DT (1<<0)

#define SH4_775X_SCI_SCSMR1   ((volatile char *)0xffe00000L)
#define SH4_775X_SCI_SCBRR1   ((volatile char *)0xffe00004L)
#define SH4_775X_SCI_SCSCR1   ((volatile char *)0xffe00008L)
#define SH4_775X_SCI_SCTDR1   ((volatile char *)0xffe0000cL)
#define SH4_775X_SCI_SCSSR1   ((volatile char *)0xffe00010L)
#define SH4_775X_SCI_SCRDR1   ((volatile char *)0xffe00014L)
#define SH4_775X_SCI_SCSPTR1  ((volatile char *)0xffe0001cL)


/* SCIF */

#define SH4_775X_SCIF_SCSMR2_CHR    (1<<6)
#define SH4_775X_SCIF_SCSMR2_PE     (1<<5)
#define SH4_775X_SCIF_SCSMR2_OE     (1<<4)
#define SH4_775X_SCIF_SCSMR2_STOP   (1<<3)
#define SH4_775X_SCIF_SCSMR2_CKS1   (1<<1)
#define SH4_775X_SCIF_SCSMR2_CKS0   (1<<0)

#define SH4_775X_SCIF_SCSCR2_TIE    (1<<7)
#define SH4_775X_SCIF_SCSCR2_RIE    (1<<6)
#define SH4_775X_SCIF_SCSCR2_TE     (1<<5)
#define SH4_775X_SCIF_SCSCR2_RE     (1<<4)
#define SH4_775X_SCIF_SCSCR2_REIE   (1<<3)
#define SH4_775X_SCIF_SCSCR2_CKE1   (1<<1)

#define SH4_775X_SCIF_SCFSR2_PER3   (1<<15)
#define SH4_775X_SCIF_SCFSR2_PER2   (1<<14)
#define SH4_775X_SCIF_SCFSR2_PER1   (1<<13)
#define SH4_775X_SCIF_SCFSR2_PER0   (1<<12)
#define SH4_775X_SCIF_SCFSR2_FER3   (1<<11)
#define SH4_775X_SCIF_SCFSR2_FER2   (1<<10)
#define SH4_775X_SCIF_SCFSR2_FER1   (1<<9)
#define SH4_775X_SCIF_SCFSR2_FER0   (1<<8)
#define SH4_775X_SCIF_SCFSR2_ER     (1<<7)
#define SH4_775X_SCIF_SCFSR2_TEND   (1<<6)
#define SH4_775X_SCIF_SCFSR2_TDFE   (1<<5)
#define SH4_775X_SCIF_SCFSR2_BRK    (1<<4)
#define SH4_775X_SCIF_SCFSR2_FER    (1<<3)
#define SH4_775X_SCIF_SCFSR2_PER    (1<<2)
#define SH4_775X_SCIF_SCFSR2_RDF    (1<<1)
#define SH4_775X_SCIF_SCFSR2_DR     (1<<0)

#define SH4_775X_SCIF_SCFCR2_RTRG1  (1<<7)
#define SH4_775X_SCIF_SCFCR2_RTRG0  (1<<6)
#define SH4_775X_SCIF_SCFCR2_TTRG1  (1<<5)
#define SH4_775X_SCIF_SCFCR2_TTRG0  (1<<4)
#define SH4_775X_SCIF_SCFCR2_MCE    (1<<3)
#define SH4_775X_SCIF_SCFCR2_TFRST  (1<<2)
#define SH4_775X_SCIF_SCFCR2_RFRST  (1<<1)
#define SH4_775X_SCIF_SCFCR2_LOOP   (1<<0)

#define SH4_775X_SCIF_SCFDR2_T4     (1<<12)
#define SH4_775X_SCIF_SCFDR2_T3     (1<<11)
#define SH4_775X_SCIF_SCFDR2_T2     (1<<10)
#define SH4_775X_SCIF_SCFDR2_T1     (1<<9)
#define SH4_775X_SCIF_SCFDR2_T0     (1<<8)
#define SH4_775X_SCIF_SCFDR2_R4     (1<<4)
#define SH4_775X_SCIF_SCFDR2_R3     (1<<3)
#define SH4_775X_SCIF_SCFDR2_R2     (1<<2)
#define SH4_775X_SCIF_SCFDR2_R1     (1<<1)
#define SH4_775X_SCIF_SCFDR2_R0     (1<<0)

#define SH4_775X_SCIF_SCSPTR2_RTSIO    (1<<7)
#define SH4_775X_SCIF_SCSPTR2_RTSDT    (1<<6)
#define SH4_775X_SCIF_SCSPTR2_CTSIO    (1<<5)
#define SH4_775X_SCIF_SCSPTR2_CTSDT    (1<<4)
#define SH4_775X_SCIF_SCSPTR2_SPB2IO   (1<<1)
#define SH4_775X_SCIF_SCSPTR2_SPB2DT   (1<<0)

#define SH4_775X_SCIF_SCLSR2_ORER   (1<<0)

#define SH4_775X_SCIF_SCSMR2     ((volatile short *)0xffe80000L)
#define SH4_775X_SCIF_SCBRR2     ((volatile char *) 0xffe80004L)
#define SH4_775X_SCIF_SCSCR2     ((volatile short *)0xffe80008L)
#define SH4_775X_SCIF_SCFTDR2    ((volatile char *) 0xffe8000cL)
#define SH4_775X_SCIF_SCFSR2     ((volatile short *)0xffe80010L)
#define SH4_775X_SCIF_SCFRDR2    ((volatile char *) 0xffe80014L)
#define SH4_775X_SCIF_SCFCR2     ((volatile short *)0xffe80018L)
#define SH4_775X_SCIF_SCFDR2     ((volatile short *)0xffe8001cL)
#define SH4_775X_SCIF_SCSPTR2    ((volatile short *)0xffe80020L)
#define SH4_775X_SCIF_SCLSR2     ((volatile short *)0xffe80024L)


/* UBC */

#define SH4_775X_UBC_BAMRA_BAMA2 (1<<3)
#define SH4_775X_UBC_BAMRA_BASMA (1<<2)
#define SH4_775X_UBC_BAMRA_BAMA1 (1<<1)
#define SH4_775X_UBC_BAMRA_BAMA0 (1<<0)

#define SH4_775X_UBC_BBRA_SZA2   (1<<6)
#define SH4_775X_UBC_BBRA_IDA1   (1<<5)
#define SH4_775X_UBC_BBRA_IDA0   (1<<4)
#define SH4_775X_UBC_BBRA_RWA1   (1<<3)
#define SH4_775X_UBC_BBRA_RWA0   (1<<2)
#define SH4_775X_UBC_BBRA_SZA1   (1<<1)
#define SH4_775X_UBC_BBRA_SZA0   (1<<0)

#define SH4_775X_UBC_BAMRB_BAMA2 (1<<3)
#define SH4_775X_UBC_BAMRB_BASMA (1<<2)
#define SH4_775X_UBC_BAMRB_BAMA1 (1<<1)
#define SH4_775X_UBC_BAMRB_BAMA0 (1<<0)

#define SH4_775X_UBC_BBRB_SZA2   (1<<6)
#define SH4_775X_UBC_BBRB_IDA1   (1<<5)
#define SH4_775X_UBC_BBRB_IDA0   (1<<4)
#define SH4_775X_UBC_BBRB_RWA1   (1<<3)
#define SH4_775X_UBC_BBRB_RWA0   (1<<2)
#define SH4_775X_UBC_BBRB_SZA1   (1<<1)
#define SH4_775X_UBC_BBRB_SZA0   (1<<0)

#define SH4_775X_UBC_BRBR_CMFA   (1<<15)
#define SH4_775X_UBC_BRBR_CMFB   (1<<14)
#define SH4_775X_UBC_BRBR_PCBA   (1<<10)
#define SH4_775X_UBC_BRBR_DBEB   (1<<7)
#define SH4_775X_UBC_BRBR_PCBB   (1<<6)
#define SH4_775X_UBC_BRBR_SEQ    (1<<3)
#define SH4_775X_UBC_BRBR_UBDE   (1<<0)

#define SH4_775X_UBC_BARA     ((volatile long *) 0xff200000L)
#define SH4_775X_UBC_BAMRA    ((volatile char *) 0xff200004L)
#define SH4_775X_UBC_BBRA     ((volatile short *)0xff200008L)
#define SH4_775X_UBC_BASRA    ((volatile char *) 0xff000014L)
#define SH4_775X_UBC_BARB     ((volatile long *) 0xff20000cL)
#define SH4_775X_UBC_BAMRB    ((volatile char *) 0xff200010L)
#define SH4_775X_UBC_BBRB     ((volatile short *)0xff200014L)
#define SH4_775X_UBC_BASRB    ((volatile char *) 0xff000018L)
#define SH4_775X_UBC_BDRB     ((volatile long *) 0xff200018L)
#define SH4_775X_UBC_BDMRB    ((volatile long *) 0xff20001cL)
#define SH4_775X_UBC_BRCR     ((volatile short *)0xff200020L)


/* BSC */

#define SH4_775X_BSC_BCR1     ((volatile long *) 0xff800000L)
#define SH4_775X_BSC_BCR2     ((volatile short *)0xff800004L)
#define SH4_775X_BSC_WCR1     ((volatile long *) 0xff800008L)
#define SH4_775X_BSC_WCR2     ((volatile long *) 0xff80000cL)
#define SH4_775X_BSC_WCR3     ((volatile long *) 0xff800010L)
#define SH4_775X_BSC_MCR      ((volatile long *) 0xff800014L)
#define SH4_775X_BSC_PCR      ((volatile short *)0xff800018L)
#define SH4_775X_BSC_RTCSR    ((volatile short *)0xff80001cL)
#define SH4_775X_BSC_RTCNT    ((volatile short *)0xff800020L)
#define SH4_775X_BSC_RTCOR    ((volatile short *)0xff800024L)
#define SH4_775X_BSC_RFCR     ((volatile short *)0xff800028L)


/* INTC */

#define SH4_775X_INTC_ICR_NMIL   (1<<15)
#define SH4_775X_INTC_ICR_MAI    (1<<14)
#define SH4_775X_INTC_ICR_NMIB   (1<<9)
#define SH4_775X_INTC_ICR_NMIE   (1<<8)
#define SH4_775X_INTC_ICR_IRLM   (1<<7)

#define SH4_775X_INTC_ICR     ((volatile short *) 0xffd00000L)
#define SH4_775X_INTC_IPRA    ((volatile short *) 0xffd00004L)
#define SH4_775X_INTC_IPRB    ((volatile short *) 0xffd00008L)
#define SH4_775X_INTC_IPRC    ((volatile short *) 0xffd0000cL)


/* IO */

#define SH4_775X_IO_PCTRA     ((volatile long *) 0xff80002cL)
#define SH4_775X_IO_PDTRA     ((volatile short *)0xff800030L)
#define SH4_775X_IO_PCTRB     ((volatile long *) 0xff800040L)
#define SH4_775X_IO_PDTRB     ((volatile short *)0xff800044L)
#define SH4_775X_IO_GPIOIC    ((volatile short *)0xff800048L)


/* CCR */

#define SH4_775X_CCR_IIX      (1<<15)
#define SH4_775X_CCR_ICI      (1<<11)
#define SH4_775X_CCR_ICE      (1<<8)
#define SH4_775X_CCR_OIX      (1<<7)
#define SH4_775X_CCR_ORA      (1<<5)
#define SH4_775X_CCR_OCI      (1<<3)
#define SH4_775X_CCR_CB       (1<<2)
#define SH4_775X_CCR_WT       (1<<1)
#define SH4_775X_CCR_OCE      (1<<0)

#define SH4_775X_CCR_CCR      ((volatile long *) 0xff00001cL)
#define SH4_775X_CCR_QACR0    ((volatile long *) 0xff000038L)
#define SH4_775X_CCR_QACR1    ((volatile long *) 0xff00003cL)

#endif // SH4_775X_RMAP_H_INCLUDED

