
#ifndef CONSOLE_H
#define CONSOLE_H

#include <type.h>
#include <termios.h>

#define BLINK		0x80

#define	BLACK		0x00
#define BLUE		0x01
#define GREEN		0x02
#define CYAN		0x03
#define RED		0x04
#define MAGENTA		0x05
#define BROWN		0x06
#define WHITE		0x07
#define	GRAY		0x08
#define LTBLUE		0x09
#define LTGREEN		0x0A
#define LTCYAN		0x0B
#define LTRED		0x0C
#define LTMAGENTA	0x0D
#define YELLOW		0x0E
#define BWHITE		0x0F

#define	BG_BLACK	0x00
#define BG_BLUE		0x10
#define BG_GREEN	0x20
#define BG_CYAN		0x30
#define BG_RED		0x40
#define BG_MAGENTA	0x50
#define BG_YELLOW	0x60
#define BG_WHITE	0x70

// 颜色属性:
//            MSB           LSB
//             0 0 0 0 0 0 0 0
//             + +-+-+ + +-+-+
//             |   |   |   |------> 前景
//             |   |   |----------> 亮度
//             |   |--------------> 背景
//             |------------------> Blink
//#define HANGSIZE     0xA0  //即160
#define VIDEO_START 		0x000B8000  //显存开始点
#define VIDEO_END   0xc0000  //结束点
#define VIDEO_RAM 0xa0000

#define VC_WIDTH 		80  //每行80字，每字有2个字节
#define VC_HEIGHT 		25 //共有25行
#define	NR_TTY		10

#define   VGA03_VC           0
#define   VGA12_VC           4
#define   VGA13_VC           5
#define    VBE_VC			  6
#define KEY_BUF_SIZE 255  //tty key buffer


	#define TEOF_DEF        '\4'    /* ^D */
	#define TEOL_DEF        _POSIX_VDISABLE
	#define TERASE_DEF      '\10'   /* ^H */
	#define TINTR_DEF       '\177'  /* ^? */
	#define TKILL_DEF       '\25'   /* ^U */
	#define TMIN_DEF        1
	#define TQUIT_DEF       '\34'   /* ^\ */
	#define TSTART_DEF      '\21'   /* ^Q */
	#define TSTOP_DEF       '\23'   /* ^S */
	#define TSUSP_DEF       '\32'   /* ^Z */
	#define TTIME_DEF       0
	#define TREPRINT_DEF    '\22'   /* ^R */
	#define TLNEXT_DEF      '\26'   /* ^V */
	#define TDISCARD_DEF    '\17'   /* ^O */


struct tty_ops
{
	char *name;
	int ( *putchar )(void *con, unsigned char ch, bool vt);
	int (*readbuf)(u8_t* buffer, int max_len);
	void (*char_erase)(void *con, unsigned count);
	void (*char_attrib)(void *con, unsigned att);
	void (*res0)(void);
	void (*res1)(void);

	int ( *init )(void);
	int ( *exit )(void);
};

typedef struct	/* circular queue */
{
	unsigned char *data;
	unsigned size, in_ptr, out_ptr;
} key_queue_t;

int deq(key_queue_t *q, unsigned char *data);
int inq(key_queue_t *q, unsigned char data);

#define	KBD_BUF_SIZE	64
#define	CONSOLE_MAGIC	0x37A1

typedef struct tty_struct__
{
	unsigned magic;	/* magic value; for validation */
	key_queue_t keystrokes;

	u16_t *fb_adr;
	unsigned char mode, index;
	unsigned char attrib, esc,  esc1, esc2, esc3;
	signed char cur_x, cur_y;
	signed char save_x, lock_x1, lock_x2;
	unsigned char save_y;
	unsigned long base;
	struct tty_ops *vga_ops;
	thread_wait_t wait;
	termios_t termios;
  struct winsize_t winsize;	/* window size (#lines and #columns) */
} console_t;



//define tty
extern console_t *current_tty;
extern console_t *tty[NR_TTY];



extern void set_color (const unsigned char c);
extern unsigned char COLOR;
extern void console_init(void);
extern void draw_t(unsigned int x, unsigned int y, const unsigned char ch, const unsigned char color);
extern  void  set_cursor (int reg, unsigned position);
extern int select_console(console_t* , bool);
extern void move_cursor(console_t *con);
extern int puts(const unsigned char *str);

 void  set_cursor (int reg, unsigned position);

extern char get_console_color(void);
extern int set_console_color (const char c);
int vga03_putchar (console_t *con, unsigned char ch, bool vt);

void termios_init(struct termios_t *ter);
int tty_gettermios(termios_t * ts,termios_t *ter);
int tty_settermios(termios_t * ts,termios_t *ter);
int tty_sigchar(char c,termios_t *ter);
int tty_editchar(char c,termios_t *ter);
int tty_inputchar(unsigned char *c,termios_t *ter);

int regster_console_device(console_t *con);
int unregster_console_device(console_t *con);
 int do_vt(console_t *con, unsigned c);

#endif
