/*****************************************************************************
Detect/identify ISA (16-bit) boards using Plug 'n Play (PnP).
Chris Giese <geezer@execpc.com>	http://www.execpc.com/~geezer
Last revised: Sep 3, 2002

This code is based on Linux isapnptools:

	Copyright
	=========
	These programs are copyright P.J.H.Fox (fox@roestock.demon.co.uk)
	and distributed under the GPL (see COPYING).

Bugs are due to Giese. Or maybe not...
*****************************************************************************/
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>

void pnp_init(void);

/* ports */
#define PNP_ADR_PORT		0x279
#define PNP_WRITE_PORT		0xA79

/* MIN and MAX READ_ADDR must have the bottom two bits set */
#define MIN_READ_ADDR		0x203
#define MAX_READ_ADDR		0x3FF
/* READ_ADDR_STEP must be a multiple of 4 */
#define READ_ADDR_STEP		8

/* bits */
#define CONFIG_WAIT_FOR_KEY	0x02
#define CONFIG_RESET_CSN	0x04

#define	IDENT_LEN		9

static int g_port, g_read_port;
static unsigned char g_csum, g_backspaced, g_backspaced_char;

static struct _export_table_entry pnp_sym_table []=
{
	/*file system*/
	EXPORT_PC_SYMBOL(pnp_init),
	//{"inb", 1},
};



int dll_main(char **argv)
{
	int i;
   int pnpn;

	printk("PnP init ...\n");
	pnp_init();

	pnpn =sizeof(pnp_sym_table)/sizeof(struct _export_table_entry);

	install_dll_table("pnp32.dll", 1,pnpn, pnp_sym_table);
	printk("PNP init OK!\n");
	return 0;
}

int dll_version()
{
	printk("JICAMA PNP Driver Version 0.01!\n");
	return 0;
}

int dll_destroy()
{
	remove_dll_table("pnp32.dll");
	return 0;
}

/*****************************************************************************
*****************************************************************************/
static void pnp_poke(unsigned char x)
{
	outb(PNP_WRITE_PORT, x);
}
/*****************************************************************************
*****************************************************************************/
static unsigned char pnp_peek(void)
{
	return inb(g_read_port);
}
/*****************************************************************************
*****************************************************************************/
static void pnp_wake(unsigned char x)
{
	outb(PNP_ADR_PORT, 3);
	outb(PNP_WRITE_PORT, x);
}
/*****************************************************************************
*****************************************************************************/
static void pnp_set_read_port(unsigned x)
{
	outb(PNP_ADR_PORT, 0);
	outb(PNP_WRITE_PORT, x >> 2);
	g_read_port = x | 3;
}
/*****************************************************************************
*****************************************************************************/
static unsigned char pnp_status(void)
{
	outb(PNP_ADR_PORT, 5);
	return pnp_peek();
}
/*****************************************************************************
*****************************************************************************/
static unsigned char pnp_resource_data(void)
{
	outb(PNP_ADR_PORT, 4);
	return pnp_peek();
}
/*****************************************************************************
*****************************************************************************/
static int pnp_await_status(void)
{
	unsigned timeout;

	for (timeout = 5; timeout != 0; timeout--)
	{
		usleep(1000);
		if ((pnp_status() & 1) != 0)
			break;
	}
	if (timeout == 0)
	{
		printk("pnp_await_status: timeout\n");
		return 1;
	}
	return 0;
}


/*****************************************************************************
*****************************************************************************/
static int pnp_read_resource_data(unsigned char *result)
{
	if (pnp_await_status())
		return 1;
	if (g_backspaced)
	{
		*result = g_backspaced_char;
		g_backspaced = 0;
	}
	*result = pnp_resource_data();
	g_csum = (g_csum + *result) & 0xFF;
	return (OK);
}


/*****************************************************************************
*****************************************************************************/
static void pnp_unread_resource_data(char prev)
{
	g_csum = (g_csum - prev) & 0xFF;
	g_backspaced = 1;
	g_backspaced_char = prev;
}


/*****************************************************************************
*****************************************************************************/
static int pnp_read_one_resource(void)
{
	unsigned char buffer[256], res_type;
	unsigned short i, res_len;
	unsigned long adr;

	if (pnp_read_resource_data(buffer + 0) != 0)
		return 1;
/* Large item */
	if (buffer[0] & 0x80)
	{
		res_type = buffer[0];
		for (i = 0; i <= 1; i++)
		{
			if (pnp_read_resource_data(buffer + i) != (OK))
				return 1;
		}
		res_len = buffer[1];
		res_len <<= 8;
		res_len |= buffer[0];
		printk("large item of size %3u: ", res_len);
	}
/* Small item */
	else
	{
		res_type = (buffer[0] >> 3) & 0x0f;
		res_len = buffer[0] & 7;
		printk("small item of size %3u: ", res_len);
	}
	for (i = 0; i < res_len; i++)
	{
		if (pnp_read_resource_data(buffer + i) != 0)
			return 1;
	}
/* */
	switch (res_type)
	{

#define IRQ_TAG			0x04
#define DMA_TAG			0x05
//#define StartDep_TAG		0x06
//#define EndDep_TAG		0x07
#define IOport_TAG		0x08
#define FixedIO_TAG		0x09
#define End_TAG			0x0F

/* Long  Tags */
#define MemRange_TAG		0x81
#define ANSIstr_TAG		0x82
//efine UNICODEstr_TAG		0x83
//efine VendorLong_TAG		0x84
#define Mem32Range_TAG		0x85
#define FixedMem32Range_TAG	0x86
	case IRQ_TAG:
		i = buffer[1];
		i <<= 8;
		i |= buffer[0];
		printk("IRQ_TAG: 0x%X\n", i);
		break;
	case DMA_TAG:
		printk("DMA_TAG: 0x%X\n", buffer[0]);
		break;
	case IOport_TAG:
		i = buffer[2];
		i <<= 8;
		i |= buffer[1];
		printk("IOport_TAG: start=0x%X, ", i);

		i = buffer[4];
		i <<= 8;
		i |= buffer[3];
		i++;
		printk("end=0x%X, step=%u, size=%u\n",
			i, buffer[5], buffer[6]);
		break;
	case FixedIO_TAG:
		i = buffer[1];
		i <<= 8;
		i |= buffer[0];
		printk("FixedIO_TAG: start=0x%X, size=%u, end=0x%X\n",
			i, buffer[2], i + 1);
		break;
	case MemRange_TAG:
		adr = buffer[2];
		adr <<= 8;
		adr |= buffer[1];
		adr <<= 8;
		printk("MemRange_TAG: start=0x%lX, ", adr);

		adr = buffer[4];
		adr <<= 8;
		adr |= buffer[3];
		adr <<= 8;
		adr++;
		printk("end=0x%lX, ", adr);

		i = buffer[6];
		i <<= 8;
		i |= buffer[5];
		printk("step=0x%X, ", i);

		adr = buffer[8];
		adr <<= 8;
		adr |= buffer[7];
		adr <<= 8;
		adr++;
		printk("size=0x%lX, ", adr);
		break;
	case ANSIstr_TAG:
		printk("ANSIstr_TAG: '");
		for (i = 0; i < res_len; i++)
			printk("%c", buffer[i]);
		printk("'\n");
		break;

	case End_TAG:
		printk("End_TAG\n");
		return 1;
	default:
		printk("unknown tag type %u\n", res_type);
		break;
	}
	return (OK);
}
/*****************************************************************************
*****************************************************************************/
static void pnp_read_board(unsigned csn, unsigned char serial_id0)
{
	unsigned char temp, dummy;
	unsigned i;

	pnp_wake(csn);
/* Check for broken cards that don't reset their resource pointer properly.
Get the first byte */
	if (pnp_read_resource_data(&temp) != 0)
		return;
/* Just check the first byte, if these match we assume it's ok */
	if (temp != serial_id0)
	{
/* Assume the card is broken and this is the start of the resource data. */
		pnp_unread_resource_data(temp);
		goto broken;
	}
/* Read resource data past serial identifier */
	for (i = 1; i < IDENT_LEN; i++)
	{
		if (pnp_read_resource_data(&dummy) != 0)
			return;
	}
/* Now for the actual resource data */
	g_csum = 0;
broken:
	do
	{
		i = pnp_read_one_resource();
	} while (!i);
}
/*****************************************************************************
*****************************************************************************/
static int pnp_isolate(void)
{
	static unsigned boards_found;
/**/
	unsigned char checksum = 0, good_adr = 0;
	unsigned char c1, c2, bit, new_bit, serial_id[IDENT_LEN];
	unsigned short byte;

	checksum = 0x6A;
/* Assume we will find one */
	boards_found++;
	pnp_wake(0);
	pnp_set_read_port(g_port);
	usleep(1000);
	outb(PNP_ADR_PORT, 0x01); /* SERIALISOLATION */
	usleep(1000);
	for (byte = 0; byte < IDENT_LEN - 1; byte++)
	{
/* xxx - tighten this up */
		for (bit = 8; bit != 0; bit--)
		{
			new_bit = 0;
			usleep(250);
			c1 = pnp_peek();
			usleep(250);
			c2 = pnp_peek();
			if (c1 == 0x55)
			{
				if (c2 == 0xAA)
				{
					good_adr = 1;
					new_bit = 0x80;
				}
				else
					good_adr = 0;
			}
			serial_id[byte] >>= 1;
			serial_id[byte] |= new_bit;
/* Update checksum */
			if (((checksum >> 1) ^ checksum) & 1)
				new_bit ^= 0x80;
			checksum >>= 1;
			checksum |= new_bit;
		}
	}
	for (bit = 8; bit != 0; bit--)
	{
		new_bit = 0;
		usleep(250);
		c1 = pnp_peek();
		usleep(250);
		c2 = pnp_peek();
		if (c1 == 0x55)
		{
			if (c2 == 0xAA)
			{
				good_adr = 1;
				new_bit = 0x80;
			}
		}
		serial_id[byte] >>= 1;
		serial_id[byte] |= new_bit;
	}
	if (good_adr && (checksum == serial_id[byte]))
	{
		outb(PNP_ADR_PORT, 0x06); /* CARDSELECTNUMBER */
		pnp_poke(boards_found);

		printk("found board #%u\n", boards_found);
		pnp_read_board(boards_found, serial_id[0]);
		return 1;
	}
/* We didn't find one */
	boards_found--;
	return (OK);
}
/*****************************************************************************
*****************************************************************************/
static void pnp_send_key(void)
{
/* Turbo C++ 1.0 seems to "lose" anything declared 'static const'
	static const char i_data[] = */
	static char i_data[] =
	{
		0x6A, 0xB5, 0xDA, 0xED, 0xF6, 0xFB, 0x7D, 0xBE,
		0xDF, 0x6F, 0x37, 0x1B, 0x0D, 0x86, 0xC3, 0x61,
		0xB0, 0x58, 0x2C, 0x16, 0x8B, 0x45, 0xA2, 0xD1,
		0xE8, 0x74, 0x3A, 0x9D, 0xCE, 0xE7, 0x73, 0x39
	};
/**/
	unsigned short temp;

	outb(PNP_ADR_PORT, 0);
	outb(PNP_ADR_PORT, 0);
	for (temp = 0; temp < sizeof(i_data) / sizeof(char); temp++)
		outb(PNP_ADR_PORT, i_data[temp]);
	usleep(2000);
}
/*****************************************************************************
*****************************************************************************/
void pnp_init(void)
{
	g_read_port=MIN_READ_ADDR;
/* All cards now isolated, read the first one */
	g_port = MIN_READ_ADDR;
	for (; g_port <= MAX_READ_ADDR; g_port += READ_ADDR_STEP)
	{
/* Make sure all cards are in Wait For Key */
		outb(PNP_ADR_PORT, 0x02); /* CONFIGCONTROL */
		pnp_poke(CONFIG_WAIT_FOR_KEY);
/* Make them listen */
		pnp_send_key();
/* Reset the cards */
		outb(PNP_ADR_PORT, 0x02); /* CONFIGCONTROL */
		pnp_poke(CONFIG_RESET_CSN | CONFIG_WAIT_FOR_KEY);
		usleep(2000);
/* Send the key again */
		pnp_send_key();
		//printk("Trying port address %04x\n", g_port);
/* Look for a PnP board */
		if (pnp_isolate())
			break;
	}
	if (g_port > MAX_READ_ADDR)
	{
		printk("pnp_init(): No boards found\n");
		return;
	}
/* found one board: print info then isolate the other boards
and print info for them */
	while (pnp_isolate())
		/* nothing */;
}
