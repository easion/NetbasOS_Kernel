
// -------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//--------------------------------------------------------------------------------------

#include <jicama/system.h>
#include <jicama/utsname.h>
#include <arch/x86/io.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <assert.h>
#include <ansi.h>
#include <string.h>

__local void clear_video(void);

__local unsigned char new_color = 0x56;
extern int  bytenum, mouse_old_cursor;

__local void vga03_erase(console_t *con, unsigned count);
__local void vga03_set_attrib(console_t *con, unsigned att);


__local struct tty_ops text_ops = {
	name:		"vga text mode",
	putchar:	vga03_putchar,
	readbuf:	kb_read,
	char_erase:	vga03_erase,
	char_attrib:vga03_set_attrib,
};

/* BIOS mode 0x03 */
__local char vga0x03_regs[60] = {
  0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,0x00,0x4F,0x0D,0x0E,
  0x00,0x00,0x00,0x00,0x9C,0x0E,0x8F,0x28,0x1F,0x96,0xB9,0xA3,
  0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,0x38,0x39,0x3A,0x3B,
  0x3C,0x3D,0x3E,0x3F,0x0C,0x00,0x0F,0x08,0x00,
  0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,0xFF,
  0x03,0x00,0x03,0x00,0x03,
  0x67
};

__local console_t vtty[3];


void console_init(void)
{
	int i;
	unsigned int pos;
	unsigned char* textmode;
	unsigned the_eflag;
	int g_vc_width, g_vc_height, bios_end, bios_start, _x, _y;

	save_eflags(&the_eflag);

	for(i=0;i<3;i++){
		vtty[i].attrib = WHITE | BG_BLACK;
		vtty[i].base = VIDEO_START + VC_WIDTH * VC_HEIGHT * 2 * i;
		vtty[i].vga_ops =  &text_ops; 
		regster_console_device(&vtty[i]);
	}

	select_console(&vtty[0],true);

	g_vc_width = peek16(0x44A);
	g_vc_height = peek8(0x484) + 1;     
	_x = peek8((void *)0x00450);
	_y = peek8((void *)0x00451);
	bios_end = peek8((void *)0x00460);
	bios_start = peek8((void *)0x00461);
	current_tty->attrib = (u8_t)peek8(0xB8000 + 159);

	current_tty->cur_x = _x;
	current_tty->cur_y = _y;
	pos=(_y*g_vc_width+_x)*2; 

	textmode = (unsigned char*)current_tty->base;

	restore_eflags(the_eflag);


	set_cursor (12, 0);
	move_cursor(&vtty[0]);

#if defined(__GNUC__)
#if __GNUC__>=3
	syslog(LOG_INFO, "Kernel built with GCC(3) version %u.%u.%u\n",
		__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
	syslog(LOG_INFO, "Kernel built with GCC version %u.%u\n",
		__GNUC__, __GNUC_MINOR__);
#endif
#endif
	trace("Video: [x=%d, y=%d(text mode: %u*%u | %d-%d)]\n",_x, _y, g_vc_width, g_vc_height, bios_start, bios_end);

}



void draw_t(unsigned int new_x, unsigned int new_y, const unsigned char ch, const unsigned char attrib)
{
	unsigned int pos;
    unsigned char* textmode;
    textmode = (unsigned char*)current_tty->base;

	current_tty->cur_x = new_x;
	current_tty->cur_y = new_y;

	pos = (current_tty->cur_y*VC_WIDTH*2+ current_tty->cur_x*2) % 4000;
	textmode[pos] = ch;
	textmode[pos+1] = attrib;

		move_cursor(current_tty);
}

void vga03h_entry(console_t *con)
{
     con->cur_x = 0;
     move_cursor(con);
}

void vga03h_move_text(console_t *con)
{
	int i;
	unsigned char* textmode = (unsigned char*)current_tty->base;

	memcpy(textmode, (textmode + VC_WIDTH*2),
		(VC_HEIGHT-1)*VC_WIDTH*2);  //must rewrite it!

	for(i = 3840; i < 4000; i+=2){
		textmode[ i ] = ' '; 
		textmode[i +1] = con->attrib ;
	}

	if(con->cur_y > VC_HEIGHT){
		 con->cur_x = 0;
		 con->cur_y--;
	}

	move_cursor(con);
}

void vga03h_new_line(console_t *con)
{
		    con->cur_x = 0;
			con->cur_y++;

	    if( con->cur_y >= VC_HEIGHT) 
			vga03h_move_text(con);

	 move_cursor(con);
}

void vga03h_backspace(console_t *con)
{
	unsigned char* textmode = (unsigned char*)current_tty->base;

	if(con->cur_x>0){
		con->cur_x -= 1;
		textmode[(con->cur_y*VC_WIDTH + con->cur_x)*2] = ' ';
		textmode[(con->cur_y*VC_WIDTH + con->cur_x)*2+1] = con->attrib ;
	   }
		else{
		if (con->cur_y > 0)
		{
			con->cur_x = 79;
			con->cur_y--;
			textmode[(con->cur_y*VC_WIDTH + con->cur_x)*2] = ' ';
			textmode[(con->cur_y*VC_WIDTH + con->cur_x)*2+1] = con->attrib ;
		}
	}
	move_cursor(con);
}

void vga03h_insert(console_t *con)
{
}

void vga03h_clean_line(console_t *con)
{
}

void beep();

int vga03_putchar (console_t *con, unsigned char ch, bool vt)
{
	 unsigned int i;
     unsigned char* textmode = (unsigned char*)current_tty->base;

	//if (!(con->termios.oflag & OPOST))
		//return;

 	if(!vt){
   
 	 switch(ch) {
	case '\a': 
			beep();
			return;
		case '\r':
		    con->cur_x = 0;
		     break;

		case '\n':
		    con->cur_x = 0;
			con->cur_y++;
			break;	

		case '\b':
			//if(con->cur_x > con->lock_x1){
			con->cur_x --;
			con->lock_x2--;
			//}
			textmode[(con->cur_y*VC_WIDTH + con->cur_x)*2] = ' ';
			textmode[(con->cur_y*VC_WIDTH + con->cur_x)*2+1] = con->attrib ;
		   
			break;

		case '\t':
			con->cur_x = (con->cur_x + 8) & ~(8 - 1);
		break;

		default:	 
		textmode[(con->cur_y*VC_WIDTH + con->cur_x)*2+1]  = con->attrib;
		textmode[(con->cur_y*VC_WIDTH+con->cur_x)*2] = ch;
		     ++con->cur_x;
		    if(con->cur_x >= VC_WIDTH){
				con->cur_x = 0;
				con->cur_y++;
			}
			break;
	}
	}
	
	if( con->cur_y >= VC_HEIGHT) {
	memcpy(textmode, (textmode + VC_WIDTH*2),
		(VC_HEIGHT-1)*VC_WIDTH*2);  //must rewrite it!
	for(i = 3840; i < 4000; i+=2)
		textmode[ i ] = ' '; 
		textmode[i +1] = con->attrib ;
		 con->cur_x = 0;
		 con->cur_y--;
	  }
	
	move_cursor(con);
	return 0;
}





__local void clear_video(void)
{  
    unsigned int i;
    unsigned char* textmode;
    textmode = (unsigned char*)current_tty->base;

	for (i=0; i<VC_WIDTH*VC_HEIGHT; i++)
	{
		textmode[i*2] = ' ';
		textmode[i*2+1] = current_tty->attrib;
	}
	return;
}



void move_cursor(console_t *con)
{
	unsigned long temp;
	unsigned short off;

	temp = (con->cur_y * VC_WIDTH + con->cur_x) * 2;
	temp = con->base + temp - VIDEO_START;
	off = temp;

    set_cursor (14, off);

	return;
}


 void  set_cursor (int reg, unsigned position)
{
	unsigned the_eflag;

    save_eflags(&the_eflag);

	outb(0x03d4, reg);
	outb(0x03d5, 0xff & (position >> 9));
	outb(0x03d4, (reg+1));
	outb(0x03d5, 0xff & (position >> 1));
	restore_eflags(the_eflag);
}


unsigned int get_position(void)
{
	unsigned int hi = 0, lo = 0;
	unsigned the_eflag;

    save_eflags(&the_eflag);

	outb(0x3d4, 14);
	hi = inb(0x3d5);
	outb(0x3d4, 15);
	lo = inb(0x3d5);

	restore_eflags(the_eflag);
	return ((hi<<9) + (lo<<1));
} 

int vga03h_mv_video(int start, int _long,  char *_address)
{
	int  t;
	char *begin = (char *)(0xb8000+start);
	char *__address = _address;
    
	for(t=0; t<_long; t++){
	  if(((int)(begin+t))%2)
		 *(unsigned char*)(__address+t) = *(unsigned char*)(begin+t);
	}
	return t;
}

/*
void draw_cursor( int new_x, int new_y )
{
	int _x,_y;
    unsigned char *video = ( unsigned char * ) current_tty -> base;

    _x = new_x;
    _y = new_y;

    if (  ( mouse.dx != _x )	|| ( mouse.dy != _y ) )
    {
	video[ (mouse.dx + mouse.dy * VC_WIDTH)*2 ] = mouse_old_cursor;
	mouse_old_cursor = video[ (_x + _y * VC_WIDTH)*2 ];
	video[ (_x + _y * VC_WIDTH)*2 ] = ( int ) '<';
	video[ (_x + _y * VC_WIDTH)*2+1 ] = get_console_color();
	mouse.dx = _x;
	mouse.dy = _y;
    }
}
*/

__local inline void vga03_gotoxy(unsigned int x, unsigned int y)
{
	unsigned pos = (VC_WIDTH * y + x) * 2;

	
    set_cursor (14, pos);

	vtty[VGA03_VC].cur_x = x;
	vtty[VGA03_VC].cur_y = y;
}


__local inline void *memsetw(void *dst_ptr, int val, size_t count)
{
	u16_t *dst = (u16_t *)dst_ptr;

	for(; count != 0; count--)
	{
		*dst = (uint16_t)val;
		dst++;
	}
	return dst_ptr;
}

__local void vga03_erase(console_t *con, unsigned count)
{
	u16_t *where;
	unsigned blank;

	ASSERT(con->magic == CONSOLE_MAGIC);

	
	where = (uint16_t *)current_tty -> base;	
	where += (con->cur_y * VC_WIDTH + con->cur_x);
	blank = ' ' | ((unsigned)con->attrib << 8);
	memsetw(where, blank, count);
}

__local void vga03_set_attrib(console_t *con, unsigned att)
{
	__local const unsigned ansi_to_vga[] =
	{
		0, 4, 2, 6, 1, 5, 3, 7
	};
	unsigned new_att;

	ASSERT(con->magic == CONSOLE_MAGIC);

	new_att = con->attrib;
/* "All attributes off" */
	if(att == 0)
		new_att = 7;
/* bold */
	else if(att == 1)
		new_att |= 0x08;
/* reverse video */
	else if(att == 7)
		new_att = (new_att & 0x88) | ((new_att & 0x07) << 4) |
			((new_att & 0x70) >> 4);
/* set foreground color */
	else if(att >= 30 && att <= 37)
	{
		att = ansi_to_vga[att - 30];
		new_att = (new_att & ~0x07) | att;
	}
/* set background color */
	else if(att >= 40 && att <= 47)
	{
		att = ansi_to_vga[att - 40] << 4;
		new_att = (new_att & ~0x70) | att;
	}
	con->attrib = new_att;
}

