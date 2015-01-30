

#ifndef __CONSOLE_DRV_H__
#define __CONSOLE_DRV_H__

#ifdef __cplusplus
extern "C" {
#endif


struct __vga_ops
{
	char *name;
	int ( *putchar )(void *con, unsigned char ch, bool vt);
	int (*readbuf)(u8_t* buffer, int max_len);
	void (*char_erase)(void *con, unsigned count);
	void (*char_attrib)(void *con, unsigned att);
	void (*res0)();
	void (*res1)();

	int ( *init )(void);
	int ( *exit )(void);
};

typedef struct
{
	void *head, *tail;
} thread_wait_t;


#define	KBD_BUF_SIZE	64
#define	CONSOLE_MAGIC	0x37A1

#define NCCS 19
typedef unsigned char	cc_t;
typedef unsigned int	speed_t;
typedef unsigned int	tcflag_t;


typedef struct termios_t {
	tcflag_t iflag;		/* input mode flags */
	tcflag_t oflag;		/* output mode flags */
	tcflag_t cflag;		/* control mode flags */
	tcflag_t lflag;		/* local mode flags */
	cc_t line;		/* line discipline */
	cc_t cc[NCCS];	/* control characters */
}termios_t;

struct winsize_t {
	unsigned short row;
	unsigned short col;
	unsigned short xpixel;
	unsigned short ypixel;
};


typedef struct 
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
	struct __vga_ops *vga_ops;
	thread_wait_t wait;
	termios_t termios;
  struct winsize_t winsize;	/* window size (#lines and #columns) */
} console_t;

int select_console(console_t* , bool);

int regster_console_device(console_t *con);
int unregster_console_device(console_t *con);
int do_vt(console_t *con, unsigned c);

#ifdef __cplusplus
}
#endif

#endif


