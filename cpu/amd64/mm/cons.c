

#include <jicama/system.h>
#include <string.h>

#define		DEF_MODE	(0x0700|0x00)

__local short* vidmem = (short*) 0XB8000;
__local unsigned int curpos = 0;

void clrscr()
{
	int c;

	for(c = 0; c < 2000; c++)
		vidmem[c] = DEF_MODE | ' ';
}
#if (0)

void setposxy(int x, int y)
{
	curpos = (y*80) + x;
}	

void
setpos(int pos)
{
	curpos = pos;
}

int
getpos()
{
	return (curpos);
}

void
update_curpos()
{
	outb(0X03D4, 0X0E);
	outb(0X03D5, curpos>>8);
	outb(0X03D4, 0X0F);
	outb(0X03D5, curpos);
}

void
get_curpos()
{
	unsigned char lo = 0, hi = 0;

	outb(0X03D4, 0X0E);
	hi = inb(0X03D5);
	outb(0X03D4, 0X0F);
	lo = inb(0X03D5);

	curpos = (hi<<8) + lo;
}	

static void putchar(unsigned char ch)
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

void
ungetc()
{
	curpos--;
	vidmem[curpos] = DEF_MODE | ' ';

	update_curpos();
}

static void puts(char* str)
{
	while(*str)
		putchar(*str++);
}

__public int __kprintf( char *format, ...)
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
					putchar(' ');
			} else {
				putchar(c);
			}
		}
		else
			switch (c = *format++) {
			      case 'd': {
				      int num = *dataptr++;
				      char buf[10], *ptr = buf;
				      if (num<0) {
					      num = -num;
					      putchar('-');
				      }
				      do
					      *ptr++ = '0'+num%10;
				      while (num /= 10);
				      do
					      putchar(*--ptr);
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
					      putchar(*--ptr);
				      while (ptr != buf);
				      break;
			      }
			      case 'c': putchar((*dataptr++)&0xff); break;
			      case 's': {
				      char *ptr = (char *)*dataptr++;
				      while ((c = *ptr++))
					      putchar(c);
				      break;
			      }
			}
			return 0;
}

#endif
