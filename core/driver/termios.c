
// ------------------------------------------------------------------------------------------
//Jicama OS  
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//---------------------------------------------------------------------------------------------

#include <jicama/process.h>
#include <signal.h>
#include <termios.h>
#include <string.h>
#include <ansi.h>
#include <conio.h> 

int kb_del_char(unsigned char *tmp);
int kb_add_char(unsigned char tmp);
int puts(const unsigned char *);

void termios_init(termios_t *termios)
{
#define CHARCTRL(a) (1 + a - 'A')

	termios->iflag = ICRNL;
	termios->oflag = OPOST | ONLCR;
	termios->cflag = 0;
	//termios->lflag =  ISIG |  ECHOE | ECHOKE;
	termios->lflag = ICANON | ISIG | ECHO | ECHOE | ECHOKE;
	termios->line = 0;
	termios->cc[VINTR] = CHARCTRL('C');
	termios->cc[VQUIT] = 034; 
	termios->cc[VERASE] = 127;
	termios->cc[VKILL] = CHARCTRL('U');
	termios->cc[VEOF] = CHARCTRL('D');
	termios->cc[VTIME] = 0;
	termios->cc[VMIN] = 1;
	termios->cc[VSWTC] = 0;
	termios->cc[VSTART] = CHARCTRL('Q');
	termios->cc[VSTOP] = CHARCTRL('S');
	termios->cc[VSUSP] = CHARCTRL('Z');
	termios->cc[VEOL] = 0;
	termios->cc[VREPRINT] = 0;
	termios->cc[VDISCARD] = CHARCTRL('U');
	termios->cc[VWERASE] = CHARCTRL('W');
	termios->cc[VLNEXT] = CHARCTRL('V');
	termios->cc[VEOL2] = 0;
#undef CHARCTRL
}


int tty_gettermios(termios_t * ts,termios_t *termios)
{
	memcpy(ts, termios, sizeof(termios_t));
	return 0;
}

int tty_settermios(termios_t * ts,termios_t *termios)
{
	memcpy(termios,ts, sizeof(termios_t));
	return 0;
}

int tty_sigchar(char c,termios_t *termios)
{
	int ret=0;

	if (!(termios->lflag & ISIG))
		return ret;

	if (c ==  termios->cc[VINTR]) {
		sigaddset(current_thread(), SIGINT);
		return 1;
	}
	else if (c ==  termios->cc[VQUIT]) {
		sigaddset(current_thread(), SIGQUIT);
		return 1;
	}else{
		/*edit chars check */
	}	
	return 0;
}

/*CONV a key code to vt100 format*/
__local char *keycode2vt100(int key)
{
	//__local char vt100str[32];

	//memset(vt100str, 0 , sizeof(vt100str));

	switch (key)
	{
	case KEY_LEFT:		
		//memcpy(vt100str, "\x1B[D",3);
		return "\x1B[D";
		break;
	case KEY_RIGHT:
		//memcpy(vt100str, "\x1B[C", 3);
		return "\x1B[C";
		break;
	case KEY_DOWN:
		//memcpy(vt100str, "\x1B[B", 3);
		return "\x1B[B";
		break;		
	case KEY_UP:
		//memcpy(vt100str, "\x1B[A", 3);
		return "\x1B[A";
		break;	
		default:
			return NULL;
	}
	return NULL;
	//kprintf("found %s", vt100str);
	//return vt100str;
}

int tty_inputchar(unsigned char *c,termios_t *termios)
{
	int ret=0;	//check
	char *vtstr;

	if (*c == '\r') {
		if (termios->iflag & ICRNL) {
			*c = '\n';
		}
	} else if (*c == '\n') { 
		if (termios->iflag & INLCR) {
			*c = '\r';
		}
	}

	if (termios->iflag & IUCLC)
		*c = tolower(*c);

	if (termios->lflag & ECHO) {
		if ((vtstr=keycode2vt100(*c))!=NULL){
			puts(vtstr);
		}else if (c == '\n') {
			tty_putchar('\n');
			tty_putchar('\r');
		}/* else if (c == '\b') {
			tty_putchar('\b');
			tty_putchar('\b');
		}*/
		else if ((termios->lflag & ECHOCTL) && iscntrl(*c)) {
			tty_putchar('^');
			tty_putchar(*c+32);
		} else{
			//kprintf("[%d]", *c);

			tty_putchar(*c);
		}
	}

	return ret;
}

__local	int eol(char c, termios_t *termios) { return c == '\n' || c == termios->cc[VEOL]; }
__local	int eof(char c,termios_t *termios) { return c == termios->cc[VEOF]; }

void tty_echoerase(char c,termios_t *termios)
{
	if (termios->lflag & ECHO) {
		if (iscntrl(c))
			tty_putchar('\b');
		else if (c == '\t') {
			puts("\b\b\b\b\b\b\b\b");
		}
		tty_putchar('\b');		
	}
}



int tty_editchar(char c,termios_t *termios)
{
	if (!(termios->lflag & ICANON))
		return 0;

	if (c == termios->cc[VERASE]) {
		if (kb_del_char(&c)) { 
			if (eol(c,termios) || eof(c,termios)) {
				kb_add_char(c);
				return 1;
			}
			tty_echoerase(c, termios);
		}
		return 1;
	}
	if (c == termios->cc[VWERASE]) {
		while (kb_del_char(&c)) {
			if (!(c == ' ' || c == '\t')) {
				kb_add_char(c);
				break;
			}
			tty_echoerase(c,termios);
		}
		while (kb_del_char(&c)) { 
			if ((eol(c,termios) || eof(c,termios)) || c == ' ' || c == '\t') {
				kb_add_char(c);
				return 1;
			}
			tty_echoerase(c,termios);
		}
		return 1;
	}
	if (c == termios->cc[VKILL]) {
		while (kb_del_char(&c)) {
			if ((eol(c,termios) || eof(c,termios))) {
				kb_add_char(c);
				return 1;
			}
			tty_echoerase(c,termios);
		}
		return 1;
	}
	return 0;
}

