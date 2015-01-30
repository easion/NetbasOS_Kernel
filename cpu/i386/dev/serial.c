
#include <jicama/system.h> 
#include <jicama/process.h> 
#include <jicama/devices.h>
#include <jicama/console.h>
#include <arch/x86/io.h>
#include <jicama/spin.h>
#include <string.h>
#include <conio.h> 
#include <signal.h> 

static void ser_putch(u16_t io_adr, unsigned c);

enum{
	/* 3F8=COM1, 2F8=COM2, 
	3E8=COM3, 2E8=COM4 */
	COM1_PORT=0X3F8,
	COM2_PORT=0X2F8,
	COM3_PORT=0X3E8,
	COM4_PORT=0X2E8,
	NOCOM_PORT=0,
};

static int *port_list[] = {COM1_PORT,COM2_PORT,COM3_PORT,COM4_PORT};
/* The offsets of UART registers.  */
#define UART_TX		0
#define UART_RX		0
#define UART_DLL	0
#define UART_IER	1
#define UART_DLH	1
#define UART_IIR	2
#define UART_FCR	2
#define UART_LCR	3
#define UART_MCR	4
#define UART_LSR	5
#define UART_MSR	6
#define UART_SR		7

/* For LSR bits.  */
#define UART_DATA_READY		0x01
#define UART_EMPTY_TRANSMITTER	0x20

/* The type of parity.  */
#define UART_NO_PARITY		0x00
#define UART_ODD_PARITY		0x08
#define UART_EVEN_PARITY	0x18

/* The type of word length.  */
#define UART_5BITS_WORD	0x00
#define UART_6BITS_WORD	0x01
#define UART_7BITS_WORD	0x02
#define UART_8BITS_WORD	0x03

/* The type of the length of stop bit.  */
#define UART_1_STOP_BIT		0x00
#define UART_2_STOP_BITS	0x04

/* the switch of DLAB.  */
#define UART_DLAB	0x80

/* Enable the FIFO.  */
#define UART_ENABLE_FIFO	0xC7

/* Turn on DTR, RTS, and OUT2.  */
#define UART_ENABLE_MODEM	0x0B
#define	BUF_SIZE	64

typedef struct
{
	key_queue_t rx, tx;
	/* number of: interrupts, receive interrupts, transmit interrupts */
	unsigned int_count, rx_count, tx_count;
	/* number of: framing errors, parity errors, overrun errors */
	unsigned ferr_count, perr_count, oerr_count;
	unsigned fifo_size;
	/* hardware resources */
	unsigned io_adr;
	/* wait queue */
	thread_wait_t wait;
} serial_t;



__local serial_t g_com[4];

__local void serial_handler(serial_t *port)
{
	/* count number of transmit and receive INTERRUPTS; not number of bytes */
	unsigned char rx_int = 0, tx_int = 0, c, reason;
	int status, i;

	port->int_count++;
	reason = inb(port->io_adr + 2);
	//kprintf("ser status %x\n", reason);
	/* careful: a loop inside an interrupt serial_handlerial can cause the system to hang */
	while((reason & 0x01) == 0)
	{
		reason >>= 1;
		reason &= 0x07;
	/* line status interrupt (highest priority)
	cleared by reading line status register (LSR; register 5) */
		if(reason == 3)
		{
			status = inb(port->io_adr + UART_LSR);
	/* 0x80 == one or more errors in Rx FIFO
	   0x10 == received BREAK */
			if(status & 0x08) /* framing error */
				port->ferr_count++;
			if(status & 0x04) /* parity error */
				port->perr_count++;
			if(status & 0x02) /* overrun error */
				port->oerr_count++;
		}
	/* receive data interrupt (2nd highest priority)
	cleared by reading receiver buffer register (register 0) */
		else if(reason == 2)
		{
	/* count ONE receive interrupt */
			if(!rx_int)
			{
				port->rx_count++;
				rx_int = 1;
			}
	/* drain the receive FIFO completely!
	poll the Data Ready bit; not the interrupt bits */
			while(inb(port->io_adr + UART_LSR) & 0x01){
				//(void)inq(&port->rx, inb(port->io_adr + 0));
				kprintf("CH%c", inb(port->io_adr + UART_RX));
			}
		}
	/* character timeout interrupt (2nd highest priority; FIFO mode only)
	cleared by receive buffer register */
		else if(reason == 6)
		{
	/* count ONE receive interrupt */
			if(!rx_int)
			{
				port->rx_count++;
				rx_int = 1;
			}
	/* drain the receive FIFO completely!
	poll the Data Ready bit; not the interrupt bits */
			while(inb(port->io_adr + UART_LSR) & 0x01){
				kprintf("%c", inb(port->io_adr + UART_RX));
				//(void)inq(&port->rx, inb(port->io_adr + 0));
			}
		}
	/* transmit holding register empty interrupt (3rd highest priority)
	cleared by reading the interrupt ID register (IIR; register 2)
	or by writing into transmit holding register (THR; register 0) */
		else if(reason == 1)
		{
	/* queue up to port->fifo_size bytes */
			for(i = port->fifo_size; i != 0; i--)
			{
				if(deq(&port->tx, &c) < 0)
				{
	/* empty transmit queue: disable further transmit interrupts */
					c = inb(port->io_adr + 1);
					if(c & 0x02)
						outb(port->io_adr + 1, c & ~0x02);
					break;
				}
	/* count ONE transmit interrupt */
				if(!tx_int)
				{
					port->tx_count++;
					tx_int = 1;
				}
				outb(port->io_adr + UART_TX, c);
			}
		}
	/* modem status interrupt (4th highest priority) cleared by reading the modem status register (MSR; register 6) */
		else if(reason == 0)
		{
			(void)inb(port->io_adr + UART_MSR);
		}
		reason = inb(port->io_adr + 2);
	}
}

void  __irq  serial_irq3(void* arg, int irq)
{
	serial_t *port;

	//kprintf(__FUNCTION__"(): called\n");

	port = g_com + 1;
	if(port->io_adr != 0)
		serial_handler(port);
	port = g_com + 3;
	if(port->io_adr != 0)
		serial_handler(port);
}

void  __irq serial_irq4(void* arg, int irq)
{
	serial_t *port;

	//kprintf(__FUNCTION__"(): called\n");
	port = g_com + 0;
	if(port->io_adr != 0)
		serial_handler(port);
	port = g_com + 2;
	if(port->io_adr != 0)
		serial_handler(port);
}

/*****************************************************************************
Identifies serial chip type (8250, 16550, etc.)
Returns FIFO size or 1 if no/defective FIFO. 16650+ detection is UNTESTED.
*****************************************************************************/
__local unsigned serial_id(unsigned io_adr)
{
	unsigned i, j;

	/* set EFR = 0 (16650+ chips only) "The EFR can only be accessed after writing [0xBF] to the LCR..."
	For 16550/A, this code zeroes the FCR instead */
	outb(io_adr + 3, 0xBF);
	outb(io_adr + 2, 0);

	/* set FCR = 1 to enable FIFOs (if any) */
	outb(io_adr + 3, 0);
	outb(io_adr + 2, 0x01);

	/* enabling FIFOs should set bits b7 and b6 in Interrupt ID register */
	i = inb(io_adr + 2) & 0xC0;

	//syslog(3, "Serial chip type: ");
	/* no FIFO -- check if scratch register exists */
	if(i == 0)
	{
		outb(io_adr + UART_SR, 0xA5);
		outb(0x80, 0xFF); /* prevent bus float returning 0xA5 */
		i = inb(io_adr + UART_SR);
		outb(io_adr + UART_SR, 0x5A);
		outb(0x80, 0xFF); /* prevent bus float returning 0x5A */
		j = inb(io_adr + UART_SR);
	/* scratch register 7 exists */
		if(i == 0xA5 && j == 0x5A){
			//syslog(3, "8250A/16450 (no FIFO)\n");
		}
		else
	/* ALL 8250s (including 8250A) have serious problems... */
			syslog(3,"ewww, 8250/8250B (no FIFO)\n");
	}
	else if(i == 0x40)
		syslog(3, "UNKNOWN; assuming no FIFO\n");
	else if(i == 0x80)
		syslog(3, "16550; defective FIFO disabled\n");
	else if(i == 0xC0)
	{
	/* for 16650+, should be able to read 0 from EFR else will read 1 from FCR */
		outb(io_adr + 3, 0xBF);
		if(inb(io_adr + 2) == 0)
		{
			syslog(3, "16650+ (32-byte FIFO)\n");
			return 32;
		}
		else
		{
			//syslog(3, "16550A (16-byte FIFO)\n");
			return 16;
		}
	}
	return 1;
}

/*****************************************************************************
	Sets bit rate and number of data bits and optionally enables FIFO.
	Also enables all interrupts except transmit 	and clears dangling interrupts.
*****************************************************************************/
static int serial_setup(serial_t *port, unsigned long baud,		unsigned bits, char enable_fifo)
{
	unsigned divisor, io_adr, i;

	if(baud > 115200L || baud < 2)
	{
		syslog(3, "serial_setup: bit rate (%lu) "
			"must be < 115200 and > 2\n", baud);
		return -1;// xxx
	}
	divisor = (unsigned)(115200L / baud);
	if(bits < 7 || bits > 8)
	{
		syslog(3, "serial_setup: number of data bits (%u) "
			"must be 7 or 8\n", bits);
		return -1;// xxx
	}

	/* set bit rate */
	io_adr = port->io_adr;
	outb(io_adr + 3, 0x80);
	outb(io_adr + 0, divisor);
	divisor >>= 8;
	outb(io_adr + 1, divisor);
	/* set number of data bits This also sets parity=none and stop bits=1 */
	outb(io_adr + 3, (bits == 7) ? 2 : 3);
	/* enable all interrupts EXCEPT transmit */
	outb(io_adr + 1, 0x0D);

	/* turn on FIFO, if any */
	if(port->fifo_size > 1 && enable_fifo)
		outb(io_adr + 2, UART_ENABLE_FIFO);
	else
		outb(io_adr + 2, 0);

	/* loopback off, interrupt gate (Out2) on,handshaking lines (RTS, DTR) off */
	outb(io_adr + 4, 0x08);

	/* clear dangling interrupts */
	while(1)
	{
		i = inb(io_adr + 2);
		if(i & 0x01)
			break;
		(void)inb(io_adr + 0);
		(void)inb(io_adr + UART_LSR);
		(void)inb(io_adr + UART_MSR);
	}

	return 0;
}

//////////////////////////////////////////////////////////
__local int ser_read(dev_prvi_t* devfp, off_t  pos, char * buf,int len)
 {
	serial_t *port;
	unsigned i, t = INFINITE;
	int minor=MINOR(devfp->devno);

	if (minor>=4)
		return 0;

	port = &g_com[minor];
	for(i = 0; i < len; i++)
	{
		do
		{
			if(port->rx.out_ptr == port->rx.in_ptr) /* empty queue */
				thread_sleep_on(&port->wait); /* no timeout */
		} while(deq(&port->rx, buf));
		buf++;
	}
	return len;
	}

__local int ser_write(dev_prvi_t* devfp, off_t  pos, char * buf,int len)
{
	unsigned t = 0;
	unsigned i, kick = 0;
	serial_t *port;
	int minor=MINOR(devfp->devno);


	if (minor>=4)
		return 0;

	port = &g_com[minor];
	kprintf("ser write to %x\n", port->io_adr);
	for(i = 0; i < len; i++)
	{
		#if 0
		while(inq(&port->tx, *buf))
		{
	/* output queue is full? kick-start transmit */
			if(!kick)
			{
				outb(port->io_adr + 1,
					inb(port->io_adr + 1) | 0x02);
				kick = 1;
			}
			thread_sleep_on(&port->wait); /* no timeout */
		}
		#else
			ser_putch(0, *buf);
		#endif
		buf++;
	}

	if(!kick)
		outb(port->io_adr + 1, inb(port->io_adr + 1) | 0x02);
	return len;
	
	}

__local int ser_ioctl(dev_prvi_t* devfp, int cmd, void* args,int size, int fromkernel)
{
	return 0;
}
static void *isr_entry3;
static void *isr_entry4;

__local int ser_open(char *devpath, int mode)
{
	int pos=(devpath[10]-'1')%4;
	serial_t *port=&g_com[pos];

	kprintf("open pos %i\n",pos);

	pos = 1;

	if (strnicmp(devpath,"/dev/ttyS",9)!=0 || port->io_adr==0)
	{
		kprintf("open %s(%d) failed!\n",devpath,pos);
		return -1;
	}

	isr_entry3=put_irq_handler(3,(u32_t)&serial_irq3,NULL,"serial1"); 
	isr_entry4=put_irq_handler(4,(u32_t)&serial_irq4,NULL,"serial2"); 

	/* zero stats */
	port->int_count = port->rx_count = port->tx_count = 0;
	port->ferr_count = port->perr_count = port->oerr_count = 0;

	/* set 115200 baud, 8N1, FIFO on */
	if(serial_setup(port, 115200L, 8, 1))
	{
		kprintf("Error setting serial port bit rate\n");
		return -1;// xxx
	}

	/* raise RTS and DTR or modem will "play dead"
	after connecting to server (hardware handshaking) */
	outb(port->io_adr + 4, UART_ENABLE_MODEM);
	return 0;
}

__local int ser_close(dev_prvi_t* devfp)
{
	serial_t *port;

	port = &g_com[MINOR(devfp->devno)];

	free_irq_handler(3,isr_entry3); 
	free_irq_handler(4,isr_entry4); 

	/* display statistics */
	kprintf("Total interrupts   : %5u\t", port->int_count);
	kprintf("Receive interrupts : %5u\n", port->rx_count);
	kprintf("Transmit interrupts: %5u\t", port->tx_count);
	kprintf("Framing errors     : %5u\n", port->ferr_count);
	kprintf("Parity errors      : %5u\t", port->perr_count);
	kprintf("Overrun errors     : %5u\n", port->oerr_count);
	return 0;
}

void dump_ser_info()
{
	int i;
	serial_t *port;

	for ( i=0; i<4; i++)
	{
		port = &g_com[i];
		kprintf("g_com%d: io_adr=0x%x port->fifo_size:%d\n",i+1, port->io_adr, port->fifo_size);
	}
}

int ser_init(void)
{
	__local const driver_ops_t ops =
	{				
		d_name:		"ttyS",
		d_author:	"OSD",
		d_kver:	CURRENT_KERNEL_VERSION,
		d_version:	KMD_VERSION,
		d_index:	ALLOC_MAJOR_NUMBER,
		open:		ser_open,
		close:		ser_close,
		read:		ser_read,
		write:		ser_write,
		ioctl:		ser_ioctl,		
	};
	int  i;


	for(i = 0; i < 4; i++)
	{
		u16_t io_adr;
		serial_t *port;
		port = &g_com[i];
		memset(port,0,sizeof(serial_t));

		io_adr = peek16(0x400 + i * 2);//COM port I/O addresses stored	in the BIOS data area,
		if(io_adr == 0){
			port->io_adr=0;//port_list[i];
			//COM1_PORT
			continue;
		}

	/* allocate memory for receive and transmit queues */
		port->rx.data = (void*)kcalloc(BUF_SIZE);
		if(port->rx.data == NULL)
			return -1;

		port->tx.data = (void*)kcalloc(BUF_SIZE);
		if(port->tx.data == NULL)
		{
			mm_free(port->rx.data,BUF_SIZE);
			return -1;
		}

		port->rx.size = port->tx.size = BUF_SIZE;
		port->io_adr =(u16_t) io_adr;
		port->fifo_size = serial_id(io_adr);
		//kprintf("init port %d io_adr=0x%x\n",i+1, port->io_adr);
	}/*end for*/

	if(kernel_driver_register(&ops)!=OK)
		return -1;



	//dump_ser_info();
	//ser_test();
	return 0;
	//while (1);
	//一次只能打开一个设备
	ser_open("/dev/ttyS",  0);


	return 0;
}

static void ser_putch(u16_t io_adr, unsigned c)
{
	if(io_adr==NOCOM_PORT)
		io_adr=COM1_PORT;

		outportb(io_adr + 3, 0x80);

	/* 115200 baud */
		outportb(io_adr + 0, 1);
		outportb(io_adr + 1, 0);

	/* 8N1 */
		outportb(io_adr + 3, 0x03);

	/* all interrupts disabled */
		outportb(io_adr + 1, 0);

	/* turn off FIFO, if any */
		outportb(io_adr + 2, 0);

	/* loopback off, interrupts (Out2) off, Out1/RTS/DTR off */
		outportb(io_adr + 4, 0);

	/* wait for transmitter ready */
	while((inportb(io_adr + UART_LSR) & 0x40) == 0)
		/* nothing */;

	/* send char */
	outportb(io_adr + 0, c);
}

void uart_putchar(int ch)
{
	ser_putch(NOCOM_PORT, ch);
}

void ser_test()
{
	char *pbuf="ser test from jicama os send ok! \n hello world!\n\tbaybay!\n中文输出！baybay";

	while (*pbuf!='\0')
	{
		ser_putch(NOCOM_PORT, *pbuf++);
	}
	//while (1)
	//{
	//}
}


