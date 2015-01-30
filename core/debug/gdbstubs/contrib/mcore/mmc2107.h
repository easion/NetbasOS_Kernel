/*
    mmc2107.h - macros, register definitions and bit masks for MMC2107

    Copyright (c) Motorola, Inc., 2002
    ALL RIGHTS RESERVED

    You are hereby granted a copyright license to use, modify, and
    distribute the SOFTWARE so long as this entire notice is retained
    without alteration in any modified and/or redistributed versions,
    and that such modified versions are clearly identified as such.
    No licenses are granted by implication, estoppel or otherwise under
    any patents or trademarks of Motorola, Inc.

    The SOFTWARE is provided on an "AS IS" basis and without warranty.
    To the maximum extent permitted by applicable law, MOTOROLA DISCLAIMS
    ALL WARRANTIES WHETHER EXPRESS OR IMPLIED, INCLUDING IMPLIED
    WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
    PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH
    REGARD TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS
    THEREOF) AND ANY ACCOMPANYING WRITTEN MATERIALS.

    To the maximum extent permitted by applicable law, IN NO EVENT SHALL
    MOTOROLA BE LIABLE FOR ANY DAMAGES WHATSOEVER
    (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF
    BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS
    INFORMATION, OR OTHER PECUNIARY LOSS) ARISING OF THE USE OR
    INABILITY TO USE THE SOFTWARE.   Motorola assumes no responsibility
    for the maintenance and support of the SOFTWARE.
*/

#ifndef _MMC2107
#define _MMC2107

/* inline asm macro, stacks r4-r7 */
#define stack_quadrant() \
__asm__("subi  r0, 16; \
         stq   r4-r7, (r0)")

/* inline asm macro, stacks epsr and epc */
#define stack_shadow_reg() \
__asm__("mfcr  r4, epc; \
         mfcr  r5, epsr; \
         subi  r0, 8; \
         stw   r4, (r0); \
         stw   r5, (r0, 4)") 

/* inline asm macro, unstacks r4-r7 */
#define unstack_quadrant() \
__asm__("ldq   r4-r7, (r0); \
         addi  r0, 16")

/* inline asm macro, unstacks epsr and epc */
#define unstack_shadow_reg() \
__asm__("ldw   r5, (r0, 4); \
         ldw   r4, (r0); \
         addi  r0, 8; \
         mtcr  r5, epsr; \
         mtcr  r4, epc")

/* inline asm macro, sets PSR(IE) bit */
#define set_psr_ie() \
__asm__("mfcr  r4, psr; \
         bseti r4, 6; \
         mtcr  r4, psr")

/* inline asm macro, sets PSR(EE) bit */
#define set_psr_ee() \
__asm__("mfcr  r4, psr; \
         bseti r4, 8; \
         mtcr  r4, psr")

/* inline asm macro, sets a trace exception */
#define gdb_breakpoint() \
__asm__("mfcr  r4, psr; \
         bseti r4, 14; \
         mtcr  r4, psr")

/* vector()

   write the address of an ISR into a specified vector table entry

   The vector base register holds the address of the vector table.
   The vectored interrupt section of the table begins at an
   offset of 32 words from the beginning of the table.  The 
   priority of the ISR determines how much to add to that offset.

   Example: Insert the address of a priority-5 ISR called "isr_PIT2"

       vector(isr_PIT2, readvbr() + ((vec_offset + 5) * 4)); 

   Where: "readvbr" is an mcore assembly language function:

       __asm__("mfcr r2, vbr");
       __asm__("rts");
*/

#define vector(isr,address) (*(void **)(address)=(isr))
#define vec_offset 32

/* Bit names for general use */
#define bit39   0x00000080
#define bit38   0x00000040
#define bit37   0x00000020
#define bit36   0x00000010
#define bit35   0x00000008
#define bit34   0x00000004
#define bit33   0x00000002
#define bit32   0x00000001
#define bit31   0x80000000
#define bit30   0x40000000
#define bit29   0x20000000
#define bit28   0x10000000
#define bit27   0x08000000
#define bit26   0x04000000
#define bit25   0x02000000
#define bit24   0x01000000
#define bit23   0x00800000
#define bit22   0x00400000
#define bit21   0x00200000
#define bit20   0x00100000
#define bit19   0x00080000
#define bit18   0x00040000
#define bit17   0x00020000
#define bit16   0x00010000
#define bit15   0x00008000
#define bit14   0x00004000
#define bit13   0x00002000
#define bit12   0x00001000
#define bit11   0x00000800
#define bit10   0x00000400
#define bit09   0x00000200
#define bit08   0x00000100
#define bit07   0x00000080
#define bit06   0x00000040
#define bit05   0x00000020
#define bit04   0x00000010
#define bit03   0x00000008
#define bit02   0x00000004
#define bit01   0x00000002
#define bit00   0x00000001

/* Following are beginning address of chip on-board modules
Note that these address names end with an underscore '_' */
#define FLASH_  0x00000000
#define RAM_    0x00800000
#define PORTS_  0x00c00000  /* Digital I/O Ports */
#define CCM_    0x00c10000  /* Chip Configuration */
#define CS_     0x00c20000  /* Chip Selects */
#define CLOCK_  0x00c30000  /* Clocks */
#define RESET_  0x00c40000  /* Resets */
#define INTC_   0x00c50000  /* Interrupts */
#define EPORT_  0x00c60000  /* Edge Port */
#define WDT_    0x00c70000  /* Watchdog Timer */
#define PIT1_   0x00c80000  /* Programmable Interrupt Timer 1 */
#define PIT2_   0x00c90000  /* Programmable Interrupt Timer 2 */
#define QADC_   0x00ca0000  /* Queued Analog-to-digital Converter */
#define SPI_    0x00cb0000  /* Serial Peripheral Interface */
#define SCI1_   0x00cc0000  /* Serial Communications Interface 1 */
#define SCI2_   0x00cd0000  /* Serial Communications Interface 2 */
#define TIM1_   0x00ce0000  /* Timer 1 */
#define TIM2_   0x00cf0000  /* Timer 2 */
#define CMFR_   0x00d00000  /* FLASH Registers */
#define EXTMEM_ 0x80000000  /* External Memory */

/* PSR Bits */
#define PSR_S       bit31
#define PSR_SP      bit29+bit28
#define PSR_U3      bit27
#define PSR_U2      bit26
#define PSR_U1      bit25
#define PSR_U0      bit24
#define PSR_PSR_VEC bit22+bit21+bit20+bit19+bit18+bit17+bit16
#define PSR_TM      bit15+bit14
#define PSR_TP      bit13
#define PSR_TCTL    bit12
#define PSR_SC      bit10
#define PSR_MM      bit09
#define PSR_EE      bit08
#define PSR_IC      bit07
#define PSR_IE      bit06
#define PSR_FIE     bit04
#define PSR_AF      bit01
#define PSR_C       bit00

#define mPIN7  0x80
#define mPIN6  0x40
#define mPIN5  0x20
#define mPIN4  0x10
#define mPIN3  0x08
#define mPIN2  0x04
#define mPIN1  0x02
#define mPIN0  0x01

#define rPORTA      (*(volatile unsigned char  *) (0x00c00000))    /* Port A Output Data Register */
#define rPORTB      (*(volatile unsigned char  *) (0x00c00001))    /* Port B Output Data Register */
#define rPORTC      (*(volatile unsigned char  *) (0x00c00002))    /* Port C Output Data Register */
#define rPORTD      (*(volatile unsigned char  *) (0x00c00003))    /* Port D Output Data Register */
#define rPORTE      (*(volatile unsigned char  *) (0x00c00004))    /* Port E Output Data Register */
#define rPORTF      (*(volatile unsigned char  *) (0x00c00005))    /* Port F Output Data Register */
#define rPORTG      (*(volatile unsigned char  *) (0x00c00006))    /* Port G Output Data Register */
#define rPORTH      (*(volatile unsigned char  *) (0x00c00007))    /* Port H Output Data Register */
#define rPORTI      (*(volatile unsigned char  *) (0x00c00008))    /* Port I Output Data Register */
#define rPORT_A_D   (*(volatile unsigned int   *) (0x00c00000))    /* Port A - D Output Data Pointer */
#define rPORT_E_H   (*(volatile unsigned int   *) (0x00c00004))    /* Port E - H Output Data Pointer */

#define rDDRA       (*(volatile unsigned char  *) (0x00c0000c))     /* Port A Data Direction Register */
#define rDDRB       (*(volatile unsigned char  *) (0x00c0000d))     /* Port B Data Direction Register */
#define rDDRC       (*(volatile unsigned char  *) (0x00c0000e))     /* Port C Data Direction Register */
#define rDDRD       (*(volatile unsigned char  *) (0x00c0000f))     /* Port D Data Direction Register */
#define rDDRE       (*(volatile unsigned char  *) (0x00c00010))     /* Port E Data Direction Register */
#define rDDRF       (*(volatile unsigned char  *) (0x00c00011))     /* Port F Data Direction Register */
#define rDDRG       (*(volatile unsigned char  *) (0x00c00012))     /* Port G Data Direction Register */
#define rDDRH       (*(volatile unsigned char  *) (0x00c00013))     /* Port H Data Direction Register */
#define rDDRI       (*(volatile unsigned char  *) (0x00c00014))     /* Port I Data Direction Register */
#define rDDR_A_D    (*(volatile unsigned int   *) (0x00c0000c))     /* Port A - D Data Direction Pointer */
#define rDDR_E_H    (*(volatile unsigned int   *) (0x00c00010))     /* Port E - H Data Direction Pointer */

#define rPORTAP     (*(volatile unsigned char  *) (0x00c00018))   /* Port A Pin Data Register */
#define rPORTBP     (*(volatile unsigned char  *) (0x00c00019))   /* Port B Pin Data Register */
#define rPORTCP     (*(volatile unsigned char  *) (0x00c0001a))   /* Port C Pin Data Register */
#define rPORTDP     (*(volatile unsigned char  *) (0x00c0001b))   /* Port D Pin Data Register */
#define rPORTEP     (*(volatile unsigned char  *) (0x00c0001c))   /* Port E Pin Data Register */
#define rPORTFP     (*(volatile unsigned char  *) (0x00c0001d))   /* Port F Pin Data Register */
#define rPORTGP     (*(volatile unsigned char  *) (0x00c0001e))   /* Port G Pin Data Register */
#define rPORTHP     (*(volatile unsigned char  *) (0x00c0001f))   /* Port H Pin Data Register */
#define rPORTIP     (*(volatile unsigned char  *) (0x00c00020))   /* Port I Pin Data Register */

#define rSETA       (*(volatile unsigned char  *) (0x00c00018))     /* Port A Pin Set Data Register */
#define rSETB       (*(volatile unsigned char  *) (0x00c00019))     /* Port B Pin Set Data Register */
#define rSETC       (*(volatile unsigned char  *) (0x00c0001a))     /* Port C Pin Set Data Register */
#define rSETD       (*(volatile unsigned char  *) (0x00c0001b))     /* Port D Pin Set Data Register */
#define rSETE       (*(volatile unsigned char  *) (0x00c0001c))     /* Port E Pin Set Data Register */
#define rSETF       (*(volatile unsigned char  *) (0x00c0001d))     /* Port F Pin Set Data Register */
#define rSETG       (*(volatile unsigned char  *) (0x00c0001e))     /* Port G Pin Set Data Register */
#define rSETH       (*(volatile unsigned char  *) (0x00c0001f))     /* Port H Pin Set Data Register */
#define rSETI       (*(volatile unsigned char  *) (0x00c00020))     /* Port I Pin Set Data Register */

#define rCLRA       (*(volatile unsigned char  *) (0x00c00024))     /* Port A Clear Output Data Register */
#define rCLRB       (*(volatile unsigned char  *) (0x00c00025))     /* Port B Clear Output Data Register */
#define rCLRC       (*(volatile unsigned char  *) (0x00c00026))     /* Port C Clear Output Data Register */
#define rCLRD       (*(volatile unsigned char  *) (0x00c00027))     /* Port D Clear Output Data Register */
#define rCLRE       (*(volatile unsigned char  *) (0x00c00028))     /* Port E Clear Output Data Register */
#define rCLRF       (*(volatile unsigned char  *) (0x00c00029))     /* Port F Clear Output Data Register */
#define rCLRG       (*(volatile unsigned char  *) (0x00c0002a))     /* Port G Clear Output Data Register */
#define rCLRH       (*(volatile unsigned char  *) (0x00c0002b))     /* Port H Clear Output Data Register */
#define rCLRI       (*(volatile unsigned char  *) (0x00c0002c))     /* Port I Clear Output Data Register */

#define rPCDPAR     (*(volatile unsigned char  *) (0x00c00030))
#define rPEPAR      (*(volatile unsigned char  *) (0x00c00031))

#define rCCR        (*(volatile unsigned short *) (0x00c10000))
#define rRCON       (*(volatile unsigned short *) (0x00c10004))
#define rCIR        (*(volatile unsigned short *) (0x00c10006))

/* Chip Test Register */
#define rCTR        (*(volatile unsigned short *) (0x00c10008))

/*  Chip Select Module  */
#define rCSCR0      (*(volatile unsigned short *) (0x00c20000)) /* Chip Select Control Register 0 */
#define rCSCR1      (*(volatile unsigned short *) (0x00c20002)) /* Chip Select Control Register 1 */
#define rCSCR2      (*(volatile unsigned short *) (0x00c20004)) /* Chip Select Control Register 2 */
#define rCSCR3      (*(volatile unsigned short *) (0x00c20006)) /* Chip Select Control Register 3 */

/*  Clock Module  */
#define rSYNCR      (*(volatile unsigned short *) (0x00c30000))
#define rSYNSR      (*(volatile unsigned char  *) (0x00c30002))

/*  Reset Module  */
#define rRCR        (*(volatile unsigned char  *) (0x00c40000))
#define rRSR        (*(volatile unsigned char  *) (0x00c40001))

/*  Interrupt Controller Module  */
#define rICR        (*(volatile unsigned short *) (0x00c50000))
#define rISR        (*(volatile unsigned short *) (0x00c50002))
#define rIFRH       (*(volatile unsigned int   *) (0x00c50004))
#define rIFRL       (*(volatile unsigned int   *) (0x00c50008))
#define rIPR        (*(volatile unsigned int   *) (0x00c5000c))
#define rNIER       (*(volatile unsigned int   *) (0x00c50010))
#define rNIPR       (*(volatile unsigned int   *) (0x00c50014))
#define rFIER       (*(volatile unsigned int   *) (0x00c50018))
#define rFIPR       (*(volatile unsigned int   *) (0x00c5001c))

/* Priority Level Select Registers */
#define rPLSR0      (*(volatile unsigned char  *) (0x00c50040))
#define rPLSR1      (*(volatile unsigned char  *) (0x00c50041))
#define rPLSR2      (*(volatile unsigned char  *) (0x00c50042))
#define rPLSR3      (*(volatile unsigned char  *) (0x00c50043))
#define rPLSR4      (*(volatile unsigned char  *) (0x00c50044))
#define rPLSR5      (*(volatile unsigned char  *) (0x00c50045))
#define rPLSR6      (*(volatile unsigned char  *) (0x00c50046))
#define rPLSR7      (*(volatile unsigned char  *) (0x00c50047))
#define rPLSR8      (*(volatile unsigned char  *) (0x00c50048))
#define rPLSR9      (*(volatile unsigned char  *) (0x00c50049))
#define rPLSR10     (*(volatile unsigned char  *) (0x00c5004a))
#define rPLSR11     (*(volatile unsigned char  *) (0x00c5004b))
#define rPLSR12     (*(volatile unsigned char  *) (0x00c5004c))
#define rPLSR13     (*(volatile unsigned char  *) (0x00c5004d))
#define rPLSR14     (*(volatile unsigned char  *) (0x00c5004e))
#define rPLSR15     (*(volatile unsigned char  *) (0x00c5004f))
#define rPLSR16     (*(volatile unsigned char  *) (0x00c50050))
#define rPLSR17     (*(volatile unsigned char  *) (0x00c50051))
#define rPLSR18     (*(volatile unsigned char  *) (0x00c50052))
#define rPLSR19     (*(volatile unsigned char  *) (0x00c50053))
#define rPLSR20     (*(volatile unsigned char  *) (0x00c50054))
#define rPLSR21     (*(volatile unsigned char  *) (0x00c50055))
#define rPLSR22     (*(volatile unsigned char  *) (0x00c50056))
#define rPLSR23     (*(volatile unsigned char  *) (0x00c50057))
#define rPLSR24     (*(volatile unsigned char  *) (0x00c50058))
#define rPLSR25     (*(volatile unsigned char  *) (0x00c50059))
#define rPLSR26     (*(volatile unsigned char  *) (0x00c5005a))
#define rPLSR27     (*(volatile unsigned char  *) (0x00c5005b))
#define rPLSR28     (*(volatile unsigned char  *) (0x00c5005c))
#define rPLSR29     (*(volatile unsigned char  *) (0x00c5005d))
#define rPLSR30     (*(volatile unsigned char  *) (0x00c5005e))
#define rPLSR31     (*(volatile unsigned char  *) (0x00c5005f))
#define rPLSR32     (*(volatile unsigned char  *) (0x00c50060))
#define rPLSR33     (*(volatile unsigned char  *) (0x00c50061))
#define rPLSR34     (*(volatile unsigned char  *) (0x00c50062))
#define rPLSR35     (*(volatile unsigned char  *) (0x00c50063))
#define rPLSR36     (*(volatile unsigned char  *) (0x00c50064))
#define rPLSR37     (*(volatile unsigned char  *) (0x00c50065))
#define rPLSR38     (*(volatile unsigned char  *) (0x00c50066))
#define rPLSR39     (*(volatile unsigned char  *) (0x00c50067))
#define rPLSR_0_3   (*(volatile unsigned int   *) (0x00c50040))
#define rPLSR_4_7   (*(volatile unsigned int   *) (0x00c50044))
#define rPLSR_8_11  (*(volatile unsigned int   *) (0x00c50048))
#define rPLSR_12_15 (*(volatile unsigned int   *) (0x00c5004c))
#define rPLSR_16_19 (*(volatile unsigned int   *) (0x00c50050))
#define rPLSR_20_23 (*(volatile unsigned int   *) (0x00c50054))
#define rPLSR_24_27 (*(volatile unsigned int   *) (0x00c50058))
#define rPLSR_28_31 (*(volatile unsigned int   *) (0x00c5005c))
#define rPLSR_32_35 (*(volatile unsigned int   *) (0x00c50060))
#define rPLSR_36_39 (*(volatile unsigned int   *) (0x00c50064))

/*  Edge Port Module  */
#define rEPPAR      (*(volatile unsigned short *) (0x00c60000))
#define rEPDDR      (*(volatile unsigned char  *) (0x00c60002))
#define rEPIER      (*(volatile unsigned char  *) (0x00c60003))
#define rEPDR       (*(volatile unsigned char  *) (0x00c60004))
#define rEPPDR      (*(volatile unsigned char  *) (0x00c60005))
#define rEPFR       (*(volatile unsigned char  *) (0x00c60006))

/*  Watchdog Timer Module */
#define rWCR        (*(volatile unsigned short *) (0x00c70000))
#define rWMR        (*(volatile unsigned short *) (0x00c70002))
#define rWCNTR      (*(volatile unsigned short *) (0x00c70004))
#define rWSR        (*(volatile unsigned short *) (0x00c70006))

/* Programmable Interrupt Timer Module */
#define rPCSR1      (*(volatile unsigned short *) (0x00c80000))
#define rPCSR2      (*(volatile unsigned short *) (0x00c90000))
#define rPMR1       (*(volatile unsigned short *) (0x00c80002))
#define rPMR2       (*(volatile unsigned short *) (0x00c90002))
#define rPCNTR1     (*(volatile unsigned short *) (0x00c80004))
#define rPCNTR2     (*(volatile unsigned short *) (0x00c90004))

#define mPCSR_PIF   0x0001 << 2

/*  Queued Analog-to-Digital Converter Module */
#define rQADCMCR    (*(volatile unsigned short *) (0x00ca0000))
#define rPORTQA     (*(volatile unsigned char  *) (0x00ca0006))
#define rPORTQB     (*(volatile unsigned char  *) (0x00ca0007))
#define rDDRQA      (*(volatile unsigned short *) (0x00ca0008))
#define rQACR0      (*(volatile unsigned short *) (0x00ca000a))
#define rQACR1      (*(volatile unsigned short *) (0x00ca000c))
#define rQACR2      (*(volatile unsigned short *) (0x00ca000e))
#define rQASR0      (*(volatile unsigned short *) (0x00ca0010))
#define rQASR1      (*(volatile unsigned short *) (0x00ca0012))

/* QADC Conversion Command Word */
#define rCCW0       (*(volatile unsigned short *) (0x00ca0200))
#define rCCW1       (*(volatile unsigned short *) (0x00ca0202))
#define rCCW2       (*(volatile unsigned short *) (0x00ca0204))
#define rCCW3       (*(volatile unsigned short *) (0x00ca0206))
#define rCCW4       (*(volatile unsigned short *) (0x00ca0208))
#define rCCW5       (*(volatile unsigned short *) (0x00ca020a))
#define rCCW6       (*(volatile unsigned short *) (0x00ca020c))
#define rCCW7       (*(volatile unsigned short *) (0x00ca020e))
#define rCCW8       (*(volatile unsigned short *) (0x00ca0210))
#define rCCW9       (*(volatile unsigned short *) (0x00ca0212))
#define rCCW10      (*(volatile unsigned short *) (0x00ca0214))
#define rCCW11      (*(volatile unsigned short *) (0x00ca0216))
#define rCCW12      (*(volatile unsigned short *) (0x00ca0218))
#define rCCW13      (*(volatile unsigned short *) (0x00ca021a))
#define rCCW14      (*(volatile unsigned short *) (0x00ca021c))
#define rCCW15      (*(volatile unsigned short *) (0x00ca021e))
#define rCCW16      (*(volatile unsigned short *) (0x00ca0220))
#define rCCW17      (*(volatile unsigned short *) (0x00ca0222))
#define rCCW18      (*(volatile unsigned short *) (0x00ca0224))
#define rCCW19      (*(volatile unsigned short *) (0x00ca0226))
#define rCCW20      (*(volatile unsigned short *) (0x00ca0228))
#define rCCW21      (*(volatile unsigned short *) (0x00ca022a))
#define rCCW22      (*(volatile unsigned short *) (0x00ca022c))
#define rCCW23      (*(volatile unsigned short *) (0x00ca022e))
#define rCCW24      (*(volatile unsigned short *) (0x00ca0230))
#define rCCW25      (*(volatile unsigned short *) (0x00ca0232))
#define rCCW26      (*(volatile unsigned short *) (0x00ca0234))
#define rCCW27      (*(volatile unsigned short *) (0x00ca0236))
#define rCCW28      (*(volatile unsigned short *) (0x00ca0238))
#define rCCW29      (*(volatile unsigned short *) (0x00ca023a))
#define rCCW30      (*(volatile unsigned short *) (0x00ca023c))
#define rCCW31      (*(volatile unsigned short *) (0x00ca023e))
#define rCCW32      (*(volatile unsigned short *) (0x00ca0240))
#define rCCW33      (*(volatile unsigned short *) (0x00ca0242))
#define rCCW34      (*(volatile unsigned short *) (0x00ca0244))
#define rCCW35      (*(volatile unsigned short *) (0x00ca0246))
#define rCCW36      (*(volatile unsigned short *) (0x00ca0248))
#define rCCW37      (*(volatile unsigned short *) (0x00ca024a))
#define rCCW38      (*(volatile unsigned short *) (0x00ca024c))
#define rCCW39      (*(volatile unsigned short *) (0x00ca024e))
#define rCCW40      (*(volatile unsigned short *) (0x00ca0250))
#define rCCW41      (*(volatile unsigned short *) (0x00ca0252))
#define rCCW42      (*(volatile unsigned short *) (0x00ca0254))
#define rCCW43      (*(volatile unsigned short *) (0x00ca0256))
#define rCCW44      (*(volatile unsigned short *) (0x00ca0258))
#define rCCW45      (*(volatile unsigned short *) (0x00ca025a))
#define rCCW46      (*(volatile unsigned short *) (0x00ca025c))
#define rCCW47      (*(volatile unsigned short *) (0x00ca025e))
#define rCCW48      (*(volatile unsigned short *) (0x00ca0260))
#define rCCW49      (*(volatile unsigned short *) (0x00ca0262))
#define rCCW50      (*(volatile unsigned short *) (0x00ca0264))
#define rCCW51      (*(volatile unsigned short *) (0x00ca0266))
#define rCCW52      (*(volatile unsigned short *) (0x00ca0268))
#define rCCW53      (*(volatile unsigned short *) (0x00ca026a))
#define rCCW54      (*(volatile unsigned short *) (0x00ca026c))
#define rCCW55      (*(volatile unsigned short *) (0x00ca026e))
#define rCCW56      (*(volatile unsigned short *) (0x00ca0270))
#define rCCW57      (*(volatile unsigned short *) (0x00ca0272))
#define rCCW58      (*(volatile unsigned short *) (0x00ca0274))
#define rCCW59      (*(volatile unsigned short *) (0x00ca0276))
#define rCCW60      (*(volatile unsigned short *) (0x00ca0278))
#define rCCW61      (*(volatile unsigned short *) (0x00ca027a))
#define rCCW62      (*(volatile unsigned short *) (0x00ca027c))
#define rCCW63      (*(volatile unsigned short *) (0x00ca027e))

/* QADC Right-Justified Unsigned Result Table */
#define rRJURR0     (*(volatile unsigned short *) (0x00ca0280))
#define rRJURR1     (*(volatile unsigned short *) (0x00ca0282))
#define rRJURR2     (*(volatile unsigned short *) (0x00ca0284))
#define rRJURR3     (*(volatile unsigned short *) (0x00ca0286))
#define rRJURR4     (*(volatile unsigned short *) (0x00ca0288))
#define rRJURR5     (*(volatile unsigned short *) (0x00ca028a))
#define rRJURR6     (*(volatile unsigned short *) (0x00ca028c))
#define rRJURR7     (*(volatile unsigned short *) (0x00ca028e))
#define rRJURR8     (*(volatile unsigned short *) (0x00ca0290))
#define rRJURR9     (*(volatile unsigned short *) (0x00ca0292))
#define rRJURR10    (*(volatile unsigned short *) (0x00ca0294))
#define rRJURR11    (*(volatile unsigned short *) (0x00ca0296))
#define rRJURR12    (*(volatile unsigned short *) (0x00ca0298))
#define rRJURR13    (*(volatile unsigned short *) (0x00ca029a))
#define rRJURR14    (*(volatile unsigned short *) (0x00ca029c))
#define rRJURR15    (*(volatile unsigned short *) (0x00ca02ae))
#define rRJURR16    (*(volatile unsigned short *) (0x00ca02a0))
#define rRJURR17    (*(volatile unsigned short *) (0x00ca02a2))
#define rRJURR18    (*(volatile unsigned short *) (0x00ca02a4))
#define rRJURR19    (*(volatile unsigned short *) (0x00ca02a6))
#define rRJURR20    (*(volatile unsigned short *) (0x00ca02a8))
#define rRJURR21    (*(volatile unsigned short *) (0x00ca02aa))
#define rRJURR22    (*(volatile unsigned short *) (0x00ca02ac))
#define rRJURR23    (*(volatile unsigned short *) (0x00ca02ae))
#define rRJURR24    (*(volatile unsigned short *) (0x00ca02b0))
#define rRJURR25    (*(volatile unsigned short *) (0x00ca02b2))
#define rRJURR26    (*(volatile unsigned short *) (0x00ca02b4))
#define rRJURR27    (*(volatile unsigned short *) (0x00ca02b6))
#define rRJURR28    (*(volatile unsigned short *) (0x00ca02b8))
#define rRJURR29    (*(volatile unsigned short *) (0x00ca02ba))
#define rRJURR30    (*(volatile unsigned short *) (0x00ca02bc))
#define rRJURR31    (*(volatile unsigned short *) (0x00ca02be))
#define rRJURR32    (*(volatile unsigned short *) (0x00ca02c0))
#define rRJURR33    (*(volatile unsigned short *) (0x00ca02c2))
#define rRJURR34    (*(volatile unsigned short *) (0x00ca02c4))
#define rRJURR35    (*(volatile unsigned short *) (0x00ca02c6))
#define rRJURR36    (*(volatile unsigned short *) (0x00ca02c8))
#define rRJURR37    (*(volatile unsigned short *) (0x00ca02ca))
#define rRJURR38    (*(volatile unsigned short *) (0x00ca02cc))
#define rRJURR39    (*(volatile unsigned short *) (0x00ca02ce))
#define rRJURR40    (*(volatile unsigned short *) (0x00ca02d0))
#define rRJURR41    (*(volatile unsigned short *) (0x00ca02d2))
#define rRJURR42    (*(volatile unsigned short *) (0x00ca02d4))
#define rRJURR43    (*(volatile unsigned short *) (0x00ca02d6))
#define rRJURR44    (*(volatile unsigned short *) (0x00ca02d8))
#define rRJURR45    (*(volatile unsigned short *) (0x00ca02da))
#define rRJURR46    (*(volatile unsigned short *) (0x00ca02dc))
#define rRJURR47    (*(volatile unsigned short *) (0x00ca02de))
#define rRJURR48    (*(volatile unsigned short *) (0x00ca02e0))
#define rRJURR49    (*(volatile unsigned short *) (0x00ca02e2))
#define rRJURR50    (*(volatile unsigned short *) (0x00ca02e4))
#define rRJURR51    (*(volatile unsigned short *) (0x00ca02e6))
#define rRJURR52    (*(volatile unsigned short *) (0x00ca02e8))
#define rRJURR53    (*(volatile unsigned short *) (0x00ca02ea))
#define rRJURR54    (*(volatile unsigned short *) (0x00ca02ec))
#define rRJURR55    (*(volatile unsigned short *) (0x00ca02ee))
#define rRJURR56    (*(volatile unsigned short *) (0x00ca02f0))
#define rRJURR57    (*(volatile unsigned short *) (0x00ca02f2))
#define rRJURR58    (*(volatile unsigned short *) (0x00ca02f4))
#define rRJURR59    (*(volatile unsigned short *) (0x00ca02f6))
#define rRJURR60    (*(volatile unsigned short *) (0x00ca02f8))
#define rRJURR61    (*(volatile unsigned short *) (0x00ca02fa))
#define rRJURR62    (*(volatile unsigned short *) (0x00ca02fc))
#define rRJURR63    (*(volatile unsigned short *) (0x00ca02fe))

/* QADC Left-Justified Signed Result Table */
#define rLJSRR0     (*(volatile unsigned short *) (0x00ca0300))
#define rLJSRR1     (*(volatile unsigned short *) (0x00ca0302))
#define rLJSRR2     (*(volatile unsigned short *) (0x00ca0304))
#define rLJSRR3     (*(volatile unsigned short *) (0x00ca0306))
#define rLJSRR4     (*(volatile unsigned short *) (0x00ca0308))
#define rLJSRR5     (*(volatile unsigned short *) (0x00ca030a))
#define rLJSRR6     (*(volatile unsigned short *) (0x00ca030c))
#define rLJSRR7     (*(volatile unsigned short *) (0x00ca030e))
#define rLJSRR8     (*(volatile unsigned short *) (0x00ca0310))
#define rLJSRR9     (*(volatile unsigned short *) (0x00ca0312))
#define rLJSRR10    (*(volatile unsigned short *) (0x00ca0314))
#define rLJSRR11    (*(volatile unsigned short *) (0x00ca0316))
#define rLJSRR12    (*(volatile unsigned short *) (0x00ca0318))
#define rLJSRR13    (*(volatile unsigned short *) (0x00ca031a))
#define rLJSRR14    (*(volatile unsigned short *) (0x00ca031c))
#define rLJSRR15    (*(volatile unsigned short *) (0x00ca031e))
#define rLJSRR16    (*(volatile unsigned short *) (0x00ca0320))
#define rLJSRR17    (*(volatile unsigned short *) (0x00ca0322))
#define rLJSRR18    (*(volatile unsigned short *) (0x00ca0324))
#define rLJSRR19    (*(volatile unsigned short *) (0x00ca0326))
#define rLJSRR20    (*(volatile unsigned short *) (0x00ca0328))
#define rLJSRR21    (*(volatile unsigned short *) (0x00ca032a))
#define rLJSRR22    (*(volatile unsigned short *) (0x00ca032c))
#define rLJSRR23    (*(volatile unsigned short *) (0x00ca032e))
#define rLJSRR24    (*(volatile unsigned short *) (0x00ca0330))
#define rLJSRR25    (*(volatile unsigned short *) (0x00ca0332))
#define rLJSRR26    (*(volatile unsigned short *) (0x00ca0334))
#define rLJSRR27    (*(volatile unsigned short *) (0x00ca0336))
#define rLJSRR28    (*(volatile unsigned short *) (0x00ca0338))
#define rLJSRR29    (*(volatile unsigned short *) (0x00ca033a))
#define rLJSRR30    (*(volatile unsigned short *) (0x00ca033c))
#define rLJSRR31    (*(volatile unsigned short *) (0x00ca033e))
#define rLJSRR32    (*(volatile unsigned short *) (0x00ca0340))
#define rLJSRR33    (*(volatile unsigned short *) (0x00ca0342))
#define rLJSRR34    (*(volatile unsigned short *) (0x00ca0344))
#define rLJSRR35    (*(volatile unsigned short *) (0x00ca0346))
#define rLJSRR36    (*(volatile unsigned short *) (0x00ca0348))
#define rLJSRR37    (*(volatile unsigned short *) (0x00ca034a))
#define rLJSRR38    (*(volatile unsigned short *) (0x00ca034c))
#define rLJSRR39    (*(volatile unsigned short *) (0x00ca034e))
#define rLJSRR40    (*(volatile unsigned short *) (0x00ca0350))
#define rLJSRR41    (*(volatile unsigned short *) (0x00ca0352))
#define rLJSRR42    (*(volatile unsigned short *) (0x00ca0354))
#define rLJSRR43    (*(volatile unsigned short *) (0x00ca0356))
#define rLJSRR44    (*(volatile unsigned short *) (0x00ca0358))
#define rLJSRR45    (*(volatile unsigned short *) (0x00ca035a))
#define rLJSRR46    (*(volatile unsigned short *) (0x00ca035c))
#define rLJSRR47    (*(volatile unsigned short *) (0x00ca035e))
#define rLJSRR48    (*(volatile unsigned short *) (0x00ca0360))
#define rLJSRR49    (*(volatile unsigned short *) (0x00ca0362))
#define rLJSRR50    (*(volatile unsigned short *) (0x00ca0364))
#define rLJSRR51    (*(volatile unsigned short *) (0x00ca0366))
#define rLJSRR52    (*(volatile unsigned short *) (0x00ca0368))
#define rLJSRR53    (*(volatile unsigned short *) (0x00ca036a))
#define rLJSRR54    (*(volatile unsigned short *) (0x00ca036c))
#define rLJSRR55    (*(volatile unsigned short *) (0x00ca036e))
#define rLJSRR56    (*(volatile unsigned short *) (0x00ca0370))
#define rLJSRR57    (*(volatile unsigned short *) (0x00ca0372))
#define rLJSRR58    (*(volatile unsigned short *) (0x00ca0374))
#define rLJSRR59    (*(volatile unsigned short *) (0x00ca0376))
#define rLJSRR60    (*(volatile unsigned short *) (0x00ca0378))
#define rLJSRR61    (*(volatile unsigned short *) (0x00ca037a))
#define rLJSRR62    (*(volatile unsigned short *) (0x00ca037c))
#define rLJSRR63    (*(volatile unsigned short *) (0x00ca037e))

/* QADC Left-Justified Unsigned Result Table */
#define rLJURR0     (*(volatile unsigned short *) (0x00ca0380))
#define rLJURR1     (*(volatile unsigned short *) (0x00ca0382))
#define rLJURR2     (*(volatile unsigned short *) (0x00ca0384))
#define rLJURR3     (*(volatile unsigned short *) (0x00ca0386))
#define rLJURR4     (*(volatile unsigned short *) (0x00ca0388))
#define rLJURR5     (*(volatile unsigned short *) (0x00ca038a))
#define rLJURR6     (*(volatile unsigned short *) (0x00ca038c))
#define rLJURR7     (*(volatile unsigned short *) (0x00ca038e))
#define rLJURR8     (*(volatile unsigned short *) (0x00ca0390))
#define rLJURR9     (*(volatile unsigned short *) (0x00ca0392))
#define rLJURR10    (*(volatile unsigned short *) (0x00ca0394))
#define rLJURR11    (*(volatile unsigned short *) (0x00ca0396))
#define rLJURR12    (*(volatile unsigned short *) (0x00ca0398))
#define rLJURR13    (*(volatile unsigned short *) (0x00ca039a))
#define rLJURR14    (*(volatile unsigned short *) (0x00ca039c))
#define rLJURR15    (*(volatile unsigned short *) (0x00ca03ae))
#define rLJURR16    (*(volatile unsigned short *) (0x00ca03a0))
#define rLJURR17    (*(volatile unsigned short *) (0x00ca03a2))
#define rLJURR18    (*(volatile unsigned short *) (0x00ca03a4))
#define rLJURR19    (*(volatile unsigned short *) (0x00ca03a6))
#define rLJURR20    (*(volatile unsigned short *) (0x00ca03a8))
#define rLJURR21    (*(volatile unsigned short *) (0x00ca03aa))
#define rLJURR22    (*(volatile unsigned short *) (0x00ca03ac))
#define rLJURR23    (*(volatile unsigned short *) (0x00ca03ae))
#define rLJURR24    (*(volatile unsigned short *) (0x00ca03b0))
#define rLJURR25    (*(volatile unsigned short *) (0x00ca03b2))
#define rLJURR26    (*(volatile unsigned short *) (0x00ca03b4))
#define rLJURR27    (*(volatile unsigned short *) (0x00ca03b6))
#define rLJURR28    (*(volatile unsigned short *) (0x00ca03b8))
#define rLJURR29    (*(volatile unsigned short *) (0x00ca03ba))
#define rLJURR30    (*(volatile unsigned short *) (0x00ca03bc))
#define rLJURR31    (*(volatile unsigned short *) (0x00ca03be))
#define rLJURR32    (*(volatile unsigned short *) (0x00ca03c0))
#define rLJURR33    (*(volatile unsigned short *) (0x00ca03c2))
#define rLJURR34    (*(volatile unsigned short *) (0x00ca03c4))
#define rLJURR35    (*(volatile unsigned short *) (0x00ca03c6))
#define rLJURR36    (*(volatile unsigned short *) (0x00ca03c8))
#define rLJURR37    (*(volatile unsigned short *) (0x00ca03ca))
#define rLJURR38    (*(volatile unsigned short *) (0x00ca03cc))
#define rLJURR39    (*(volatile unsigned short *) (0x00ca03ce))
#define rLJURR40    (*(volatile unsigned short *) (0x00ca03d0))
#define rLJURR41    (*(volatile unsigned short *) (0x00ca03d2))
#define rLJURR42    (*(volatile unsigned short *) (0x00ca03d4))
#define rLJURR43    (*(volatile unsigned short *) (0x00ca03d6))
#define rLJURR44    (*(volatile unsigned short *) (0x00ca03d8))
#define rLJURR45    (*(volatile unsigned short *) (0x00ca03da))
#define rLJURR46    (*(volatile unsigned short *) (0x00ca03dc))
#define rLJURR47    (*(volatile unsigned short *) (0x00ca03de))
#define rLJURR48    (*(volatile unsigned short *) (0x00ca03e0))
#define rLJURR49    (*(volatile unsigned short *) (0x00ca03e2))
#define rLJURR50    (*(volatile unsigned short *) (0x00ca03e4))
#define rLJURR51    (*(volatile unsigned short *) (0x00ca03e6))
#define rLJURR52    (*(volatile unsigned short *) (0x00ca03e8))
#define rLJURR53    (*(volatile unsigned short *) (0x00ca03ea))
#define rLJURR54    (*(volatile unsigned short *) (0x00ca03ec))
#define rLJURR55    (*(volatile unsigned short *) (0x00ca03ee))
#define rLJURR56    (*(volatile unsigned short *) (0x00ca03f0))
#define rLJURR57    (*(volatile unsigned short *) (0x00ca03f2))
#define rLJURR58    (*(volatile unsigned short *) (0x00ca03f4))
#define rLJURR59    (*(volatile unsigned short *) (0x00ca03f6))
#define rLJURR60    (*(volatile unsigned short *) (0x00ca03f8))
#define rLJURR61    (*(volatile unsigned short *) (0x00ca03fa))
#define rLJURR62    (*(volatile unsigned short *) (0x00ca03fc))
#define rLJURR63    (*(volatile unsigned short *) (0x00ca03fe))

/*  Serial Peripheral Interface Module  */
#define rSPICR1     (*(volatile unsigned char  *) (0x00cb0000))
#define rSPICR2     (*(volatile unsigned char  *) (0x00cb0001))
#define rSPIBR      (*(volatile unsigned char  *) (0x00cb0002))
#define rSPISR      (*(volatile unsigned char  *) (0x00cb0003))
#define rSPIDR      (*(volatile unsigned char  *) (0x00cb0005))
#define rSPIPURD    (*(volatile unsigned char  *) (0x00cb0006))
#define rSPIPORT    (*(volatile unsigned char  *) (0x00cb0007))
#define rSPIDDR     (*(volatile unsigned char  *) (0x00cb0008))

/*  Serial Communications Interface Modules  */
#define rSCI1BD     (*(volatile unsigned short *) (0x00cc0000))
#define rSCI2BD     (*(volatile unsigned short *) (0x00cd0000))
#define rSCI1CR1    (*(volatile unsigned char  *) (0x00cc0002))
#define rSCI2CR1    (*(volatile unsigned char  *) (0x00cd0002))
#define rSCI1CR2    (*(volatile unsigned char  *) (0x00cc0003))
#define rSCI2CR2    (*(volatile unsigned char  *) (0x00cd0003))
#define rSCI1SR1    (*(volatile unsigned char  *) (0x00cc0004))
#define rSCI2SR1    (*(volatile unsigned char  *) (0x00cd0004))
#define rSCI1SR2    (*(volatile unsigned char  *) (0x00cc0005))
#define rSCI2SR2    (*(volatile unsigned char  *) (0x00cd0005))
#define rSCI1DRH    (*(volatile unsigned char  *) (0x00cc0006))
#define rSCI2DRH    (*(volatile unsigned char  *) (0x00cd0006))
#define rSCI1DRL    (*(volatile unsigned char  *) (0x00cc0007))
#define rSCI2DRL    (*(volatile unsigned char  *) (0x00cd0007))
#define rSCI1PURD   (*(volatile unsigned char  *) (0x00cc0008))
#define rSCI2PURD   (*(volatile unsigned char  *) (0x00cd0008))
#define rSCI1PORT   (*(volatile unsigned char  *) (0x00cc0009))
#define rSCI2PORT   (*(volatile unsigned char  *) (0x00cd0009))
#define rSCI1DDR    (*(volatile unsigned char  *) (0x00cc000a))
#define rSCI2DDR    (*(volatile unsigned char  *) (0x00cd000a))

/* SCI status and control register bit masks */
#define mRDRF  0x20     /* Receive Data Register Full Flag */
#define mTDRE  0x80     /* Transmit Data Register Empty Flag */
#define mTIE   0x80     /* Transmitter Interrupt Enable */

/*  Timer Modules  */
#define rTIM1IOS    (*(volatile unsigned char  *) (0x00ce0000))
#define rTIM2IOS    (*(volatile unsigned char  *) (0x00cf0000))
#define rTIM1CFORC  (*(volatile unsigned char  *) (0x00ce0001))
#define rTIM2CFORC  (*(volatile unsigned char  *) (0x00cf0001))
#define rTIM1OC3M   (*(volatile unsigned char  *) (0x00ce0002))
#define rTIM2OC3M   (*(volatile unsigned char  *) (0x00cf0002))
#define rTIM1OC3D   (*(volatile unsigned char  *) (0x00ce0003))
#define rTIM2OC3D   (*(volatile unsigned char  *) (0x00cf0003))
#define rTIM1FOC    (*(volatile unsigned int   *) (0x00ce0000))
#define rTIM2FOC    (*(volatile unsigned int   *) (0x00cf0000))
#define rTIM1CNT    (*(volatile unsigned short *) (0x00ce0004))
#define rTIM2CNT    (*(volatile unsigned short *) (0x00cf0004))
#define rTIM1SCR1   (*(volatile unsigned char  *) (0x00ce0006))
#define rTIM2SCR1   (*(volatile unsigned char  *) (0x00cf0006))
#define rTIM1TOV    (*(volatile unsigned char  *) (0x00ce0008))
#define rTIM2TOV    (*(volatile unsigned char  *) (0x00cf0008))
#define rTIM1CTL1   (*(volatile unsigned char  *) (0x00ce0009))
#define rTIM2CTL1   (*(volatile unsigned char  *) (0x00cf0009))
#define rTIM1CTL2   (*(volatile unsigned char  *) (0x00ce000b))
#define rTIM2CTL2   (*(volatile unsigned char  *) (0x00cf000b))
#define rTIM1TOVCTL (*(volatile unsigned int   *) (0x00ce0008))
#define rTIM2TOVCTL (*(volatile unsigned int   *) (0x00cf0008))
#define rTIM1IE     (*(volatile unsigned char  *) (0x00ce000c))
#define rTIM2IE     (*(volatile unsigned char  *) (0x00cf000c))
#define rTIM1SCR2   (*(volatile unsigned char  *) (0x00ce000d))
#define rTIM2SCR2   (*(volatile unsigned char  *) (0x00cf000d))
#define rTIM1FLG1   (*(volatile unsigned char  *) (0x00ce000e))
#define rTIM2FLG1   (*(volatile unsigned char  *) (0x00cf000e))
#define rTIM1FLG2   (*(volatile unsigned char  *) (0x00ce000f))
#define rTIM2FLG2   (*(volatile unsigned char  *) (0x00cf000f))

#define rTIM1C0     (*(volatile unsigned short *) (0x00ce0010))
#define rTIM1C1     (*(volatile unsigned short *) (0x00ce0012))
#define rTIM1C2     (*(volatile unsigned short *) (0x00ce0014))
#define rTIM1C3     (*(volatile unsigned short *) (0x00ce0016))
#define rTIM2C0     (*(volatile unsigned short *) (0x00cf0010))
#define rTIM2C1     (*(volatile unsigned short *) (0x00cf0012))
#define rTIM2C2     (*(volatile unsigned short *) (0x00cf0014))
#define rTIM2C3     (*(volatile unsigned short *) (0x00cf0016))

#define rTIM1PACTL  (*(volatile unsigned char  *) (0x00ce0018))
#define rTIM2PACTL  (*(volatile unsigned char  *) (0x00cf0018))
#define rTIM1PAFLG  (*(volatile unsigned char  *) (0x00ce0019))
#define rTIM2PAFLG  (*(volatile unsigned char  *) (0x00cf0019))
#define rTIM1PACNT  (*(volatile unsigned short *) (0x00ce001a))
#define rTIM2PACNT  (*(volatile unsigned short *) (0x00cf001a))

#define rTIM1PORT   (*(volatile unsigned char  *) (0x00ce001d))
#define rTIM2PORT   (*(volatile unsigned char  *) (0x00cf001d))
#define rTIM1DDR    (*(volatile unsigned char  *) (0x00ce001e))
#define rTIM2DDR    (*(volatile unsigned char  *) (0x00cf001e))

/*  FLASH Memory Control Module  */
#define rCMFRMCR    (*(volatile unsigned int   *) (0x00d00000))
#define rCMFRMTR    (*(volatile unsigned int   *) (0x00d00004))
#define rCMFRCTL    (*(volatile unsigned int   *) (0x00d00008))

#endif /* #ifndef _MMC2107 */
