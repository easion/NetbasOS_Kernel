
#ifndef NE2K_H
#define NE2K_H

typedef struct ne2000_t ne2000_t;
struct ne2000_t
{
    pci_state_t *device;
    uint16_t iobase;

    uint8_t saprom[16];
    uint8_t station_address[6];
    bool word_mode;

   u8_t  xstart;
   u8_t  pstart;
   u8_t  pstop;
   u8_t  pnext;
   int irq;
   int tx_packets, rx_packets;
  void* fd;
   void *out;
   void *irq_hld;
   //packetqueue_t *input;
   size_t rxsize;
   size_t txsize;
};

#define COMMAND                 0
#define PAGESTART               1
#define PAGESTOP                2
#define BOUNDARY                3
#define TRANSMITSTATUS          4
#define TRANSMITPAGE            4
#define TRANSMITBYTECOUNT0      5
#define NCR                     5
#define TRANSMITBYTECOUNT1      6
#define INTERRUPTSTATUS         7
#define CURRENT                 7       /* in page 1 */
#define REMOTESTARTADDRESS0     8
#define CRDMA0                  8
#define REMOTESTARTADDRESS1     9
#define CRDMA1                  9
#define REMOTEBYTECOUNT0        10
#define REMOTEBYTECOUNT1        11
#define RECEIVESTATUS           12
#define RECEIVECONFIGURATION    12
#define TRANSMITCONFIGURATION   13
#define FAE_TALLY               13
#define DATACONFIGURATION       14
#define CRC_TALLY               14
#define INTERRUPTMASK           15
#define MISS_PKT_TALLY          15
#define IOPORT                  16
//#define PSTART                  0x46
//#define PSTOP                   0x80
#define TRANSMITBUFFER          0x40

/* Bits in PGX_CR - Command Register */
#define CR_STP                  0x01    /* Stop chip */
#define CR_STA                  0x02    /* Start chip */
#define CR_TXP                  0x04    /* Transmit a frame */
#define CR_RD0                  0x08    /* Remote read */
#define CR_RD1                  0x10    /* Remote write */
#define CR_RD2                  0x20    /* Abort/complete remote DMA */
#define CR_PAGE0                0x00    /* Select page 0 of chip registers */
#define CR_PAGE1                0x40    /* Select page 1 of chip registers */
#define CR_PAGE2                0x80    /* Select page 2 of chip registers */

#define rcr                     0       /* value for Recv config. reg */
#define tcr                     0       /* value for trans. config. reg */
#define dcr                     0x58    /* value for data config. reg */
#define imr                     0x7f /*0x4b*/    /* value for intr. mask reg */

#define ISR_PRX 0x01 // packet received
#define ISR_PTX 0x02 // packet transmitted
#define ISR_RXE 0x04 // receive error
#define ISR_TXE 0x08 // transmit error
#define ISR_OVW 0x10 // overwrite warning
#define ISR_CNT 0x20 // counter overflow
#define ISR_RDC 0x40 // remote DMA complete
#define ISR_RST 0x80 // reset status

/* NE2000 specific implementation registers */
#define NE_RESET                0x1f        /* Reset */
#define NE_DATA                 0x10        /* Data port (use for PROM) */

ne2000_t *get_ne2000_device();

#define CR         0x00 // command register (all pages rw)
#define PSTART     0x01 // page start register (page 0 w)
#define PSTOP      0x02 // page stop register (page 0 w)
#define BNRY       0x03 // boundary pointer (page 0 r/w)
#define TSR        0x04 // transmit status register (page 0 r)
#define TPSR       0x04 // transmit page start address (page 0 w)
#define TBCR0      0x05 // transmit byte count register low (page 0 w)
#define TBCR1      0x06 // transmit byte count register high (page 0 w)
#define ISR        0x07 // interrupt status register (page 0 r/w)
#define RSAR0      0x08 // remote start address register low (page 0 w)
#define RSAR1      0x09 // remote start address register high (page 0 w)
#define RBCR0      0x0A // remote byte count register low (page 0 w)
#define RBCR1      0x0B // remote byte count register high (page 0 w)
#define RSR        0x0C // receive status register (page 0 r)
#define RCR        0x0C // receive configuration register (page 0 w)
#define TCR        0x0D // transmit configuration register (page 0 w)
#define DCR        0x0E // data configuration register (page 0 w)
#define IMR        0x0F // interrupt mask register (page 0 w)
#define PAR0       0x01 // physical address register low (page 1 r/w)
#define CURR       0x07 // current page register (page 1 r/w)
#define MAR0       0x08 // multicast address register low (page 1 r/w)
#define NE2K_DATA  0x10 // data i/o port
#define NE2K_RESET 0x1F // reset chip

#define CR_STP 0x01 // stop
#define CR_STA 0x02 // start
#define CR_TXP 0x04 // transmit packet
#define CR_RD0 0x08 // remote DMA command (bit 0)
#define CR_RD1 0x10 // remote DMA command (bit 1)
#define CR_RD2 0x20 // remote DMA command (bit 2)
#define CR_PS0 0x40 // page select (bit 0)
#define CR_PS1 0x80 // page select (bit 1)

#define ISR_PRX 0x01 // packet received
#define ISR_PTX 0x02 // packet transmitted
#define ISR_RXE 0x04 // receive error
#define ISR_TXE 0x08 // transmit error
#define ISR_OVW 0x10 // overwrite warning
#define ISR_CNT 0x20 // counter overflow
#define ISR_RDC 0x40 // remote DMA complete
#define ISR_RST 0x80 // reset status

#define RCR_SEP 0x01 // save errored packets
#define RCR_AR  0x02 // accept runt packets
#define RCR_AB  0x04 // accept broadcast
#define RCR_AM  0x08 // accept multicast
#define RCR_PRO 0x10 // proiscuous physical
#define RCR_MON 0x20 // monitor mode

int ne2000_handler(void*arg, int irq);
bool read_mac_addr(ne2000_t *nic);
int ne2000_install(void);
void ne_setup(ne2000_t *nic);


#define PCIC_NETWORK	0x02
#define PCIS_NETWORK_ETHERNET	0x00


#define dbg_write //printk

#endif

