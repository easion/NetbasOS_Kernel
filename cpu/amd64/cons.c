
#include <jicama/system.h>

#define		DEF_MODE	(0x0a00|0x00)
inline  unsigned char inb(int port);
inline  void outb(int port, unsigned char data);

__local short* vidmem = (short*) 0XB8000+0x80000000;
__local unsigned int curpos = 0;


static void clrscr()
{
	int c;

	for(c = 0; c < 2000; c++)
		vidmem[c] = DEF_MODE | ' ';
}

static void setposxy(int x, int y)
{
	curpos = (y*80) + x;
}	

static void
setpos(int pos)
{
	curpos = pos;
}

static int
getpos()
{
	return (curpos);
}

static 
update_curpos()
{
	outb(0X03D4, 0X0E);
	outb(0X03D5, curpos>>8);
	outb(0X03D4, 0X0F);
	outb(0X03D5, curpos);
}

static void
get_curpos()
{
	unsigned char lo = 0, hi = 0;

	outb(0X03D4, 0X0E);
	hi = inb(0X03D5);
	outb(0X03D4, 0X0F);
	lo = inb(0X03D5);

	curpos = (hi<<8) + lo;
}	

static void putc(unsigned char ch)
{
	int c;
	
	if(ch == '\r')
		return;

	if( ch == '\n' ) {
		curpos = ( curpos + 80 ) / 80 * 80;
	} else {
		vidmem[curpos++] = DEF_MODE | ch;
	}

	if( curpos >= 2000 ) {
		memcpy(vidmem + 80, vidmem, 3840);
		for(c = 1920; c <= 2000; c++)
			vidmem[ c ] = DEF_MODE | ' ';
		curpos -= 80;
	}

	update_curpos();
}

static void
ungetc()
{
	curpos--;
	vidmem[curpos] = DEF_MODE | ' ';

	update_curpos();
}

void d_puts(char* str)
{
	while(*str)
		putc(*str++);
}

int
dprintf( char *format, ...)
{
	int *dataptr = (int *)&format;
	char c;

	get_curpos();

	dataptr++;
	while ((c = *format++))
		if (c != '%') {
			if(c == '\t') {
				unsigned int c = 4;
				while(c--)
					putc(' ');
			} else {
				putc(c);
			}
		}
		else
			switch (c = *format++) {
			      case 'd': {
				      int num = *dataptr++;
				      char buf[10], *ptr = buf;
				      if (num<0) {
					      num = -num;
					      putc('-');
				      }
				      do
					      *ptr++ = '0'+num%10;
				      while (num /= 10);
				      do
					      putc(*--ptr);
				      while (ptr != buf);
				      break;
			      }
			      case 'x': {
				      unsigned int num = *dataptr++, dig;
				      char buf[8], *ptr = buf;
				      do
					      *ptr++ = (dig=(num&0xf)) > 9?
							'a' + dig - 10 :
							'0' + dig;
				      while (num >>= 4);
				      do
					      putc(*--ptr);
				      while (ptr != buf);
				      break;
			      }
			      case 'c': putc((*dataptr++)&0xff); break;
			      case 's': {
				      char *ptr = (char *)*dataptr++;
				      while ((c = *ptr++))
					      putc(c);
				      break;
			      }
			}
			return 0;
}
