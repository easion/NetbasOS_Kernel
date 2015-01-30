
#include <jicama/system.h> 
#include <jicama/process.h> 
#include <jicama/devices.h>
#include <jicama/console.h>
#include <arch/x86/io.h>
#include <jicama/spin.h>
#include <string.h>
#include <conio.h> 
#include <signal.h> 
#include <clist.h> 


__local int convert_key(unsigned key);

void halt(void);

/* "raw" set 1 scancodes from PC keyboard. Keyboard info here:
http://my.execpc.com/~geezer/osd/kbd */
#define	RAW1_LEFT_CTRL		0x1D
#define	RAW1_RIGHT_CTRL		0x1D	/* same as left */
#define	RAW1_LEFT_SHIFT		0x2A
#define	RAW1_RIGHT_SHIFT	0x36
#define	RAW1_LEFT_ALT		0x38
#define	RAW1_RIGHT_ALT		0x38	/* same as left */
#define	RAW1_CAPS_LOCK		0x3A
#define	RAW1_F1			0x3B
#define	RAW1_F2			0x3C
#define	RAW1_F3			0x3D
#define	RAW1_F4			0x3E
#define	RAW1_F5			0x3F
#define	RAW1_F6			0x40
#define	RAW1_F7			0x41
#define	RAW1_F8			0x42
#define	RAW1_F9			0x43
#define	RAW1_F10		0x44
#define	RAW1_NUM_LOCK		0x45
#define	RAW1_SCROLL_LOCK	0x46
#define	RAW1_DEL		0x53
#define	RAW1_F11		0x57
#define	RAW1_F12		0x58

int __irq keyboard_irq(void *arg,int irq);
int key_cook(	int value,	func_t p);
void reset_keylist();

int key_message(u8_t key);
static void *isr_entry;

 void kb_init()
{
	reset_keylist();

	isr_entry=put_irq_handler(1,(u32_t)&keyboard_irq,NULL,"keyboard"); 
}

void kb_deinit(void)
{
	free_irq_handler(1,isr_entry); 
}


__local void reboot(void)
{
	unsigned i;

	disable();
	/* flush the keyboard controller */
	do
	{
		i = inportb(0x64);
		if(i & 0x01)
		{
			(void)inportb(0x60);
			continue;
		}
	} while(i & 0x02);
/* pulse the CPU reset line */
	outportb(0x64, 0xFE);
/* ...and if that didn't work, just halt */
	halt();
}


__local void write_kbd(unsigned adr, unsigned data)
{
	unsigned long timeout;
	unsigned status;

	for(timeout = 500000L; timeout != 0; timeout--)
	{
		status = inportb(0x64);
/* loop until 8042 input buffer empty */
		if((status & 0x02) == 0)
			break;
	}
	if(timeout != 0)
		outportb(adr, data);
}

extern dev_prvi_t *g_tty_fd;


__local inline int scan_kb(void)
{
#define PORT_B          0x61	/* I/O port for 8255 port B (kbd, beeper...) */
	int code;
  	int val;

  	code = inb(0x60);
  	val = inb(PORT_B);
  	outb(PORT_B, val | 0x80);
  	outb(PORT_B, val);
  	return code;
#undef 	PORT_B
}

extern int kb_raw_mode;

int __irq  keyboard_irq(void *arg,int irq)
{
	int code=0;
	int err;
	struct Event e;

	console_t *con= current_tty;

	//kprintf("kb irq = %d\n", irq);

	code =scan_kb();
	disable();


	//if (!kb_raw_mode)
	{

		memset(&e,0,sizeof(struct Event));
		e.what = E_KEY_DOWN;
		e.modifiers = 0;
		e.key = code;
		e.dev = CONSOLE_DEVNO<<16;
		/*sumit to gui*/
		err = submit_input_enent(&e);

		if (!err)
		{
			goto out;
		}

		code= convert_key(code);
		if (code<0)	{goto out;}

		if(tty_sigchar(code, &current_tty->termios))
		{
			//kprintf("tty_sigchar\n");
			goto out;
		}
		if(tty_editchar(code, &current_tty->termios)){
			//kprintf("tty_editchar\n");
			goto out;
		}
		if(tty_inputchar(&code, &current_tty->termios)){
			//kprintf("tty_inputchar\n");
			goto out;
		}
	}

	/*put it to queue, wait user get, it seems work good*/
	inq(&con->keystrokes, code);


	if(g_tty_fd){
		//kprintf("set_io_event kbd called\n");
		if(set_io_event(&g_tty_fd->iob, IOEVT_READ)==0)
			thread_wakeup(&con->wait);
	}
	else{
	  thread_wakeup(&con->wait);
	}

	out:

		enable();

	//	clear_io_event(&g_tty_fd->iob, IOEVT_READ);
	//thread_wakeup(&con->wait);
	return 0;
}

 

__local int convert_key(unsigned key)
{
	__local const unsigned char set1_map[] =
	{
/* 00 */0,	0x1B,	'1',	'2',	'3',	'4',	'5',	'6',
/* 08 */'7',	'8',	'9',	'0',	'-',	'=',	'\b',	'\t',
/* 10 */'q',	'w',	'e',	'r',	't',	'y',	'u',	'i',
/* 1Dh is left Ctrl */
/* 18 */'o',	'p',	'[',	']',	'\n',	0,	'a',	's',
/* 20 */'d',	'f',	'g',	'h',	'j',	'k',	'l',	';',
/* 2Ah is left Shift */
/* 28 */'\'',	'`',	0,	'\\',	'z',	'x',	'c',	'v',
/* 36h is right Shift */
/* 30 */'b',	'n',	'm',	',',	'.',	'/',	0,	0,
/* 38h is left Alt, 3Ah is Caps Lock */
/* 38 */0,	' ',	0,	KEY_F1,	KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,
/* 45h is Num Lock, 46h is Scroll Lock */
/* 40 */KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10,0,	0,	KEY_HOME,
/* 48 */KEY_UP,	KEY_PGUP,'-',	KEY_LEFT,'5',	KEY_RIGHT,'+',	KEY_END,
/* 50 */KEY_DOWN,KEY_PGDN,KEY_INS,KEY_DEL,0,	0,	0,	KEY_F11,
/* 58 */KEY_F12
	};
	__local const unsigned char shift1_map[]=
	{
/* 00 */0,	0x1B,	'!',	'@',	'#',	'$',	'%',	'^',
/* 08 */'&',	'*',	'(',	')',	'_',	'+',	'\b',	'\t',
/* 10 */'Q',	'W',	'E',	'R',	'T',	'Y',	'U',	'I',
/* 1Dh is left Ctrl */
/* 18 */'O',	'P',	'{',	'}',	'\n',	0,	'A',	'S',
/* 20 */'D',	'F',	'G',	'H',	'J',	'K',	'L',	':',
/* 2Ah is left Shift */
/* 28 */'\"',	'~',	0,	'|',	'Z',	'X',	'C',	'V',
/* 36h is right Shift */
/* 30 */'B',	'N',	'M',	'<',	'>',	'?',	0,	0,
/* 38h is left Alt, 3Ah is Caps Lock */
/* 38 */0,	' ',	0,	KEY_F1,	KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,
/* 45h is Num Lock, 46h is Scroll Lock */
/* 40 */KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10,0,	0,	KEY_HOME,
/* 48 */KEY_UP,	KEY_PGUP,'-',	KEY_LEFT,'5',	KEY_RIGHT,'+',	KEY_END,
/* 50 */KEY_DOWN,KEY_PGDN,KEY_INS,KEY_DEL,0,	0,	0,	KEY_F11,
/* 58 */KEY_F12
	};
	__local unsigned kbd_status;
	unsigned i;
	

/* check for break key (i.e. a key is released) */
	if(key >= 0x80)
	{
		key &= 0x7F;
/* the only break codes we're interested in are Shift, Ctrl, Alt */
		if(key == RAW1_LEFT_ALT || key == RAW1_RIGHT_ALT)
			kbd_status &= ~KBD_META_ALT;
		else if(key == RAW1_LEFT_CTRL || key == RAW1_RIGHT_CTRL)
			kbd_status &= ~KBD_META_CTRL;
		else if(key == RAW1_LEFT_SHIFT || key == RAW1_RIGHT_SHIFT)
			kbd_status &= ~KBD_META_SHIFT;
		return -1;
	}

/* it's a make key (key preseed): check the "meta" keys, as above */
	if(key == RAW1_LEFT_ALT || key == RAW1_RIGHT_ALT)	{
		kbd_status |= KBD_META_ALT;
		return -1;
	}

	if(key == RAW1_LEFT_CTRL || key == RAW1_RIGHT_CTRL)	{
		kbd_status |= KBD_META_CTRL;
		return -1;
	}

	if(key == RAW1_LEFT_SHIFT || key == RAW1_RIGHT_SHIFT)	{
		kbd_status |= KBD_META_SHIFT;
		return -1;
	}

/* Scroll Lock, Num Lock, and Caps Lock set the LEDs. These keys
have on-off (toggle or XOR) action, instead of momentary action */
	if(key == RAW1_SCROLL_LOCK)	{
		kbd_status ^= KBD_META_SCRL;
		goto LEDS;
	}

	if(key == RAW1_NUM_LOCK)	{
		kbd_status ^= KBD_META_NUM;
		goto LEDS;
	}

	if(key == RAW1_CAPS_LOCK)	{
		kbd_status ^= KBD_META_CAPS;
LEDS:
	write_kbd(0x60, 0xED);	/* "set LEDs" command */
		i = 0;
		if(kbd_status & KBD_META_SCRL)
			i |= 1;
		if(kbd_status & KBD_META_NUM)
			i |= 2;
		if(kbd_status & KBD_META_CAPS)
			i |= 4;
		write_kbd(0x60, i);	/* bottom 3 bits set LEDs */

		return -1;
	}

/* now that we've tested for CTRL and ALT,
we can handle the three-finger salute */
	if((kbd_status & KBD_META_CTRL) &&
		(kbd_status & KBD_META_ALT) && key == RAW1_DEL)
	{
		trace("\n""\x1B[42;37;1m""*** rebooting!");
		reboot();
	}

/* ignore invalid scan codes */
	if(key >= sizeof(set1_map) / sizeof(set1_map[0]))
		return -1;

/* handle Ctrl+A-Z */
	if(kbd_status & KBD_META_CTRL){
		i = set1_map[key];
		if(i < 'a' || i > 'z')
			return 0;
		return i - 'a' + 1;
	}

/* handle Shift */
	if(kbd_status & KBD_META_SHIFT || kbd_status & KBD_META_CAPS){
		i = shift1_map[key];
	}
	else if(kbd_status & RAW1_LEFT_ALT){
		i = 0;
	}
	else{
		i = set1_map[key];
	}

	return i;
}

