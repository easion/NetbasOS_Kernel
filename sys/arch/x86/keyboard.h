#ifndef KB_US_STD_H
#define KB_US_STD_H

#define PORT_B          0x61	/* I/O port for 8255 port B (kbd, beeper...) */
#define MAX_CHAR  255

#define	KEY_F1		0x80
#define	KEY_F2		0x81   //(KEY_F1 + 1)
#define	KEY_F3		0x82   //(KEY_F2 + 1)
#define	KEY_F4		0x83   //(KEY_F3 + 1)
#define	KEY_F5		0x84   //(KEY_F4 + 1)
#define	KEY_F6		0x85   //(KEY_F5 + 1)
#define	KEY_F7		0x86   //(KEY_F6 + 1)
#define	KEY_F8		0x87   //(KEY_F7 + 1)
#define	KEY_F9		0x88   //(KEY_F8 + 1)
#define	KEY_F10	0x89	//(KEY_F9 + 1)
#define	KEY_F11	0x8a	//(KEY_F10 + 1)
#define	KEY_F12	0x8b   //	(KEY_F11 + 1)
/* cursor keys */
#define	KEY_INS		0x90
#define	KEY_DEL		0x91 //(KEY_INS + 1)
#define	KEY_HOME	0x92 //(KEY_DEL + 1)
#define	KEY_END		0x93 //(KEY_HOME + 1)
#define	KEY_PGUP	0x94 //(KEY_END + 1)
#define	KEY_PGDN	0x95 //(KEY_PGUP + 1)
#define	KEY_LFT		0x96 //(KEY_PGDN + 1)
#define	KEY_UP		     0x97 //(KEY_LFT + 1)
#define	KEY_DN		0x98 //(KEY_UP + 1)
#define	KEY_RT		     0x99 //(KEY_DN + 1)
/* print screen/sys rq and pause/break */
#define	KEY_PRNT	  0x9a //(KEY_RT + 1)
#define	KEY_PAUSE	0x9b //(KEY_PRNT + 1)
/* these return a value but they could also act as additional bucky keys */
#define	KEY_LWIN	   0x9c //(KEY_PAUSE + 1)
#define	KEY_RWIN	    0x9d //(KEY_LWIN + 1)
#define	KEY_MENU	    0x9f //(KEY_RWIN + 1)


static inline void waitempty (void)
{
  char status;
  do {
    status = inb (0x64); /* ¼üÅÌ¿ØÖÆÆ÷×´Ì¬¶Ë¿Ú */
  } while (status & 0x02); /* wait until bit1 = 0 */ 
}

static inline int set_leds(u8_t kb_led)
{
	waitempty();
	outb(0x60,0xed);
	while(inb(0x60 != 0xfa))
	waitempty();
	outb(0x60,kb_led & 0x07);
	return 0;
}

static inline int kb_ack()
{
	int retries;

  	retries = 0x1000 + 1;
  	while (--retries != 0 && inb(0x60) != 0xFA)
		;
  	return(retries);
}

extern  int putchar(const unsigned char ch);

#endif


