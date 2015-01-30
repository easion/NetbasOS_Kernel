#include <drv/defs.h>
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/ia.h>
#include <drv/pci.h>
#include <drv/pbuf.h>
#include <drv/errno.h>

#include "ne2000.h"
#include <sys/queue.h>
#include <sys/compat_bsd.h>
#include <sys/endian.h>


struct support_card
{
	int vet;
	int dev;
	char *devname;
}card_type[] ={
	{0x10ec, 0x8029, "RealTek RTL-8029" },
	{0x1050, 0x0940, "Winbond 89C940" },
	{0x11f6, 0x1401, "Compex RL2000"},
	{0x8e2e, 0x3000, "KTI ET32P2" },
	{0x4a14, 0x5000, "NetVin NV5000SC" },
	{0x1106, 0x0926, "Via 86C926" },
	{0x10bd, 0x0e34, "SureCom NE34" },
	{0x1050, 0x5a5a, "Winbond W89C940F" },
	{0x12c3, 0x0058, "Holtek HT80232" },
	{0x12c3, 0x5598, "Holtek HT80229" },
	{ 0x8c4a, 0x1980, "Winbond 89C940 (misprogrammed)" },
	{0,0,NULL}
};


#define SIGTTIN           21	/* background process wants to read */

static void writepkt(ne2000_t *nic, struct pbuf *p);
void dump_packet(struct pbuf *packet);


int ne2000_drv_transmit(void* dev, struct pbuf *p)
{
   unsigned eflags;
    ne2000_t *nic= get_ne2000_device();
   
   save_eflags(&eflags);
   if ( inportb(nic->iobase + CR) & CR_TXP ) // currently transmitting
   {
      queue_pkt_add(nic->out, p);
   }
   else
   {
      writepkt(nic, p);      
   }
 
   restore_eflags(eflags);
   return 0;
}


struct pbuf *readpkt(ne2000_t *nic)
{
   struct pbuf *packet;
   u8_t rsr, next;
   int i, len;
   
   outportb(nic->iobase + RBCR0, 4);
   outportb(nic->iobase + RBCR1, 0);
   outportb(nic->iobase + RSAR0, 0);
   outportb(nic->iobase + RSAR1, nic->pnext);
   outportb(nic->iobase + CR, CR_RD0 | CR_STA);
   rsr  = inportb(nic->iobase + NE2K_DATA);
   next = inportb(nic->iobase + NE2K_DATA);
   len  = inportb(nic->iobase + NE2K_DATA);
   len += inportb(nic->iobase + NE2K_DATA) << 8;
   outportb(nic->iobase + ISR, ISR_RDC);
   
   if ( (rsr & 31) == 1 &&
	next >= nic->pstart && next <= nic->pstop &&
	len <= 1532 ) // sanity check
   {
      packet = pbuf_alloc(PBUF_RAW, ETHER_FRAME_LEN, PBUF_RW);
      
      outportb(nic->iobase + RBCR0, len);
      outportb(nic->iobase + RBCR1, len >> 8);
      outportb(nic->iobase + RSAR0, 4);
      outportb(nic->iobase + RSAR1, nic->pnext);
      outportb(nic->iobase + CR, CR_RD0 | CR_STA);

	  nic->rxsize+=len;

	  u8_t *data = packet->payload;
	  pbuf_realloc(packet, len);
      
      for ( i = 0 ; i < len ; ++i ){
		 data[i] = inportb(nic->iobase + NE2K_DATA);
	  }
      outportb(nic->iobase + ISR, ISR_RDC);
      
      nic->pnext = (next == nic->pstop) ? nic->pstart : next;      
   }
   else // insane NIC -- my NE2000 goes insane quite often
   { 
      
      outportb(nic->iobase + CR, CR_PS0 | CR_RD2 | CR_STA);
      nic->pnext = inportb(nic->iobase + CURR);
      outportb(nic->iobase + CR, CR_RD2 | CR_STA);
      
      packet = 0;
   }
   
   if ( nic->pnext == nic->pstart )
      outportb(nic->iobase + BNRY, nic->pstop - 1);
   else
      outportb(nic->iobase + BNRY, nic->pnext - 1);
   
    ++nic->rx_packets;
  return packet;
}

void writepkt(ne2000_t *nic, struct pbuf *p)
{
   int i, len = p->tot_len, size = len < 60 ? 60 : len;
	int left;
	struct pbuf *q;

   outportb(nic->iobase + RBCR0, size);
   outportb(nic->iobase + RBCR1, size >> 8);
   outportb(nic->iobase + RSAR0, 0);
   outportb(nic->iobase + RSAR1, nic->xstart);
   outportb(nic->iobase + CR, CR_RD1 | CR_STA);

   nic->txsize+=p->tot_len;
	left = p->tot_len;
	for (q = p; q != NULL; q = q->next) 
	{
		len = q->len;
		left -= len;
		unsigned char *data = q->payload;

		if (len > 0){
			for ( i = 0 ; i < q->len ; ++i ){
			outportb(nic->iobase + NE2K_DATA, data[i]);
			}
		}
		pbuf_free(q);
   }

   for ( i = len ; i < size ; ++i )
      outportb(nic->iobase + NE2K_DATA, 0);

   outportb(nic->iobase + ISR, ISR_RDC);   
   outportb(nic->iobase + TPSR, nic->xstart);
   outportb(nic->iobase + TBCR0, size);
   outportb(nic->iobase + TBCR1, size >> 8);
   outportb(nic->iobase + CR, CR_RD2 | CR_TXP | CR_STA);
   ++nic->tx_packets;

}

int ne2000_handler(void*arg, int irq)
{
   int cause;
    ne2000_t *nic= get_ne2000_device();	
	struct pbuf *p;

   while (( cause = inportb(nic->iobase + ISR) ))
   {
      outportb(nic->iobase + ISR, cause);

      if ( cause & ISR_PRX ) // packet received
      {
	 u8_t curr;
	 outportb(nic->iobase + CR, CR_PS0 | CR_RD2 | CR_STA);
	 curr = inportb(nic->iobase + CURR);
	 outportb(nic->iobase + CR, CR_RD2 | CR_STA);
	 
	 while ( nic->pnext != curr )
	 {
	    p = readpkt(nic);
	    if ( !p ){
			kprintf("readpkt(): no buffer\n");
			break;
		}

		//dump_packet(p);

		//queue_pkt_add(nic->input, p);
		dev_receive(nic->fd, p);
		nic->rx_packets++;
		
	    outportb(nic->iobase + ISR, ISR_PRX);
	    outportb(nic->iobase + CR, CR_PS0 | CR_RD2 | CR_STA);
	    curr = inportb(nic->iobase + CURR);
	    outportb(nic->iobase + CR, CR_RD2 | CR_STA);
	 }/*end while*/

      }/*receive*/

      if ( cause & (ISR_PTX | ISR_TXE) ) // packet transmission done
      {
			//dbg_write("send handleNetIRQ happen %x", nic->iobase);
			struct pbuf *p = queue_pkt_del(nic->out);
			if ( p ) {
			nic->tx_packets++;
			writepkt(nic, p);
	 }
      }

   }
   return 1;
}




unsigned char pci_get_class(device_t dev)
{
	pci_softc_t *sc = device_get_ivars(dev);
	return sc->dev->base_class;
}


int pci_get_irq(device_t dev)
{
	pci_softc_t *sc = device_get_ivars(dev);
	return sc->dev->irq;
}
unsigned char pci_get_subclass(device_t dev)
{
	pci_softc_t *sc = device_get_ivars(dev);
	return sc->dev->sub_class;
}
static int ne_found=0;
int
pci_ne2000_probe(device_t self)
{
	pci_softc_t *sc = device_get_ivars(self);
	pci_state_t *pci_ne;
	int warn=0;
	//struct support_card *sc=card_type;
	int i=0, irq;
	ne2000_t *nic = get_ne2000_device();

	if (!sc)
	{
	kprintf("pci_ne2000_probe not found\n");
	return 1;
	}


	pci_ne = sc->dev;

	if((pci_get_class(self) != PCIC_NETWORK) &&
	   (pci_get_subclass(self) != PCIS_NETWORK_ETHERNET))
	{
		return 1;
	}

	for (i=0; card_type[i].devname; i++)
	{
		if (pci_ne->deviceid == card_type[i].dev 
			&& pci_ne->vendorid==card_type[i].vet){
				break;
			}
	}

	if (!card_type[i].devname && pci_ne->deviceid!=0)
	{
		//kprintf("ne2000 not found %x,%x\n",pci_ne->deviceid,pci_ne->vendorid);
		return 1;
	}


	memset(nic,0,sizeof(ne2000_t));

    //nic->iobase = 0x300;
	//irq = 12;/*just tmp*/

	for (i=0; i<6; i++)	{
	  if(pci_ne->base[i]){
        nic->iobase = pci_ne->base[i];
	  }
	}
	
	//kprintf(" NE2000 io base is 0x%x ", nic->iobase);

	nic->device = pci_ne;
	nic->irq = pci_ne->irq;

	

	read_mac_addr(nic);

	u8_t sa = nic->station_address[0];



	for (i=0;i<6 ;i++ )
	{
		if (nic->station_address[i]!=sa){
			break;
		}
	}

	if (i == 6)
	{
		return 1;
	}
   kprintf("NE2000: MAC (station address) is %02X-%02X-%02X-%02X-%02X-%02X\n",
        nic->station_address[0], nic->station_address[1], nic->station_address[2], 
        nic->station_address[3], nic->station_address[4], nic->station_address[5]);	
		

   ne_found = 1;

	return 0;
}



int
pci_ne2000_attach(device_t self)
{
	static int inited = 0;

	if (inited)
	{
		return -1;
	}

	if (!ne_found)
	{
		printf("pci_ne2000_attach error\n");
		return -1;
	}

	load_ne2000();

	inited=1;



	//printf("pci_ne2000_attach called\n");



	//kprintf("ne2000 netcard registered with io %x\n",nic->iobase );
	return 0;
}

/*
1050 Winbond Electronics Corp
	10500000	ethernet	ne2k-pci	NE2000

Vendor: Device: Class/Rev/Sub Vendor or Device name: 
1050h - - 0000h  Ethernet Adapter  Winbond Electronics Corp  NE2000 

*/


int ne2000_install(void)
{	
	int i;
	ne2000_t *nic = get_ne2000_device();

	//kprintf("install ne2000 9999 iobase = %x\n",nic->iobase);
	ne_setup(nic);
   nic->out = new_pktque();

   if (!nic->out)   {
	kprintf("nic->out  error\n\n");
	return -1;
   }

	nic->rx_packets=nic->tx_packets=0;
	nic->irq_hld = put_irq_handler(nic->irq, (u32_t)&ne2000_handler,NULL,"NE2000" );
	return OK;
}




bool read_mac_addr(ne2000_t *nic)
{
    unsigned i;
    u8_t Buffer[32];
    u8_t WordLength;

    /* Read Station Address PROM (SAPROM) which is 16 bytes at remote DMA address 0.
       Some cards double the data read which we must compensate for */

    /* Initialize RBCR0 and 0x0b - Remote Byte Count Registers */
    outportb(nic->iobase + REMOTEBYTECOUNT0, 0x20);
    outportb(nic->iobase + REMOTEBYTECOUNT1, 0x00);

    /* Initialize RSAR0 and RSAR1 - Remote Start Address Registers */
    outportb(nic->iobase + REMOTESTARTADDRESS0, 0x00);
    outportb(nic->iobase + REMOTESTARTADDRESS1, 0x00);

    /* Select page 0, read and start the NIC */
    outportb(nic->iobase + COMMAND, CR_STA | CR_RD0 | CR_PAGE0);

    /* Read one byte at a time */
    WordLength = 2; /* Assume a word is two bytes */
    for (i = 0; i < 32; i += 2)
    {
        Buffer[i] = inportb(nic->iobase + IOPORT);
        Buffer[i + 1] = inportb(nic->iobase + IOPORT);
		if (Buffer[i] != Buffer[i + 1])
			WordLength = 1; /* A word is one byte long */
	}

    /* If WordLength is 2 the data read before was doubled. We must compensate for this */
    if (WordLength == 2)
    {
        nic->word_mode = TRUE;

        /* Move the SAPROM data to the adapter object */
        for (i = 0; i < 16; i++)
            nic->saprom[i] = Buffer[i * 2];

        /* Copy the station address */
        memcpy(&nic->station_address, &nic->saprom, sizeof(nic->station_address));

        /* Initialize DCR - Data Configuration Register (word mode/4 words FIFO) */
        outportb(nic->iobase + DATACONFIGURATION, dcr);
		
        return TRUE;
    }
    else
    {
        printk("NE2000:  adapter not found, may NE1000\n");
        nic->word_mode = FALSE;
        return FALSE;
    }
}



void ne_setup(ne2000_t *nic)
{
	int i;
 
   outportb(nic->iobase + 0x1f, inportb(nic->iobase + 0x1f));
   while ( (inportb(nic->iobase + 0x07) & 0x80) == 0 ) {}
   
   outportb(nic->iobase + 0x00, 0x20 | 0x01);
   outportb(nic->iobase + 0x0e, 0x48);
   outportb(nic->iobase + 0x0a, 0);
   outportb(nic->iobase + 0x0b, 0);
   outportb(nic->iobase + 0x0c, 0x04);
   outportb(nic->iobase + 0x04, nic->xstart = 0x40);
   outportb(nic->iobase + TCR, 2);
   outportb(nic->iobase + PSTART, nic->pstart = 0x46);
   outportb(nic->iobase + BNRY, nic->pstart);
   outportb(nic->iobase + PSTOP, nic->pstop = 0x60);
   outportb(nic->iobase + 0x07, 0xFF);
   outportb(nic->iobase + IMR, 0x1F);
   outportb(nic->iobase + 0x00, CR_PS0 | 0x20 | 0x01);
   for ( i = 0 ; i < 6 ; ++i ) outportb(nic->iobase + PAR0 + i, nic->station_address[i]);
   for ( i = 0 ; i < 8 ; ++i ) outportb(nic->iobase + MAR0 + i, 0xFF);
   outportb(nic->iobase + 0x07, nic->pnext = nic->pstart + 1);
   outportb(nic->iobase + 0x00, 0x20 | CR_STA);
   outportb(nic->iobase + TCR, 0);	
}



