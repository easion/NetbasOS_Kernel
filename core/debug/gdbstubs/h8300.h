#if !defined(H8300_H__INCLUDED)
#define H8300_H__INCLUDED


#if defined(H8300_H8S2148EDK)
#define H8S2148E
#endif



#if defined(H8S2148E)

/* h8s/2148 running in enhanced mode */


#define gdb_breakpoint() __asm__(" trapa #3")


#define IOBASE 0xFF0000UL

#define P8DDR ((volatile unsigned char *)(IOBASE + 0xffbdUL))
#define P8DR ((volatile unsigned char *)(IOBASE + 0xffbfUL))


#define MSTPCR  ((volatile unsigned short *)(IOBASE + 0xff86UL))
#define MSTPCRH ((volatile unsigned char *)(IOBASE + 0xff86UL))
#define MSTPCRL ((volatile unsigned char *)(IOBASE + 0xff87UL))
#define MSTPCR_MSTP7 0x0080
#define MSTPCR_MSTP6 0x0040
#define MSTPCR_MSTP5 0x0020

#define SMR0  ((volatile unsigned char *)(IOBASE + 0xffd8UL))
#define SMR_CA   0x80
#define SMR_CHR  0x40
#define SMR_PE   0x20
#define SMR_OE   0x10
#define SMR_STOP 0x08
#define SMR_MP   0x04
#define SMR_CKS1 0x02
#define SMR_CKS0 0x01

#define BRR0  ((volatile unsigned char *)(IOBASE + 0xffd9UL))
#define SCR0  ((volatile unsigned char *)(IOBASE + 0xffdaUL))
#define SCR_TIE   0x80
#define SCR_RIE   0x40
#define SCR_TE    0x20
#define SCR_RE    0x10
#define SCR_MPIE  0x08
#define SCR_TEIE  0x04
#define SCR_CKE1  0x02
#define SCR_CKE0  0x01

#define TDR0  ((volatile unsigned char *)(IOBASE + 0xffdbUL))
#define SSR0  ((volatile unsigned char *)(IOBASE + 0xffdcUL))
#define SSR_TDRE  0x80
#define SSR_RDRF  0x40
#define SSR_ORER  0x20
#define SSR_FER   0x10
#define SSR_PER   0x08
#define SSR_TEND  0x04
#define SSR_MPB   0x02
#define SSR_MPBT  0x01

#define RDR0  ((const volatile unsigned char *)(IOBASE + 0xffddUL))
#define SCMR0 ((volatile unsigned char *)(IOBASE + 0xffdeUL))
#define SCMR_SDIR 0x08
#define SCMR_SINV 0x04
#define SCMR_SMIF 0x01

#define SMR1  ((volatile unsigned char *)(IOBASE + 0xff88UL))
#define BRR1  ((volatile unsigned char *)(IOBASE + 0xff89UL))
#define SCR1  ((volatile unsigned char *)(IOBASE + 0xff8aUL))
#define TDR1  ((volatile unsigned char *)(IOBASE + 0xff8bUL))
#define SSR1  ((volatile unsigned char *)(IOBASE + 0xff8cUL))
#define RDR1  ((const volatile unsigned char *)(IOBASE + 0xff8dUL))
#define SCMR1 ((volatile unsigned char *)(IOBASE + 0xff8eUL))

#define SMR2  ((volatile unsigned char *)(IOBASE + 0xffa8UL))
#define BRR2  ((volatile unsigned char *)(IOBASE + 0xffa9UL))
#define SCR2  ((volatile unsigned char *)(IOBASE + 0xffaaUL))
#define TDR2  ((volatile unsigned char *)(IOBASE + 0xffabUL))
#define SSR2  ((volatile unsigned char *)(IOBASE + 0xffacUL))
#define RDR2  ((const volatile unsigned char *)(IOBASE + 0xffadUL))
#define SCMR2 ((volatile unsigned char *)(IOBASE + 0xffaeUL))

#else
#error Unknown processor variant.  (Insert more variants here).
#endif

#endif
