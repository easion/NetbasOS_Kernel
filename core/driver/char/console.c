
// -------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//--------------------------------------------------------------------------------------

#include <jicama/process.h>
#include <jicama/system.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <termios.h>
#include <errno.h>
#include <ansi.h>

dev_prvi_t *g_tty_fd;

int con_read(dev_prvi_t* devfp, off_t  pos, void * buf,int count);
int con_write(dev_prvi_t* devfp, off_t  pos, void * buf,int count);
int con_ioctl(dev_prvi_t* devfp, int cmd, void* args,int,int);
__local int kb_open(char *f, int mode,dev_prvi_t* devfp);
__local int kb_close(dev_prvi_t* devfp);
__asmlink void kb_init(void);
void kb_flush(void);

#define TCGETS		0x5401
#define TCSETS		0x5402
#define TCSETSW		0x5403
#define TCSETSF		0x5404
#define TCGETA		0x5405
#define TCSETA		0x5406
#define TCSETAW		0x5407
#define TCSETAF		0x5408
#define TCSBRK		0x5409
#define TCXONC		0x540A
#define TCFLSH		0x540B
#define TIOCEXCL	0x540C
#define TIOCNXCL	0x540D
#define TIOCSCTTY	0x540E
#define TIOCGPGRP	0x540F
#define TIOCSPGRP	0x5410
#define TIOCOUTQ	0x5411
#define TIOCSTI		0x5412
#define TIOCGWINSZ	0x5413
#define TIOCSWINSZ	0x5414
#define TIOCMGET	0x5415
#define TIOCMBIS	0x5416
#define TIOCMBIC	0x5417
#define TIOCMSET	0x5418
#define TIOCGSOFTCAR	0x5419
#define TIOCSSOFTCAR	0x541A

int select_console(console_t * which_con, bool switchto)
{
	int ret = 0;
	if(!which_con || which_con->magic != CONSOLE_MAGIC)
		return -1;

	if (switchto)
	{
		if (which_con->vga_ops->init){
			ret = which_con->vga_ops->init();
		}
		if(!ret){
			current_tty = which_con;
		}
	}     
   return ret;
}

int current_console()
{
	int no;
	no = current_tty->index;
   return no;
}



int tty_putchar(const unsigned char c)
 {
	 bool vt;
	 unsigned char ch = c;
	 console_t *con = current_tty;

	if (con->termios.oflag & OLCUC)
		ch = toupper(ch);

	vt = do_vt(con, ch);
	con->vga_ops->putchar(con, ch, vt);
	return 0;
 }

int tty_readbuf(u8_t* buffer, int max_len)
 {
	 if (!current_tty->vga_ops ||
		 !current_tty->vga_ops->readbuf)
	 {
		 return -1;
	 }
	   return current_tty->vga_ops->readbuf(buffer, max_len);
 }

int puts(const unsigned char *str)
{
      unsigned char ch;

       while ( (ch = *str++) != '\0')
	      tty_putchar(ch);
	   return 0;
}

int set_console_color (const char c)
{
	int w = current_tty->index;

      if(w>0 && w<4){
          current_tty->attrib = c;
		  return SUCCESS;
	  }
	  else{
		  //kprintf(" set_console_color(): you are not in vga03h or vga12h mode!\n");
          return FAILED;
	  }
          return FAILED;
}

char get_console_color()
{
	int w = current_tty->index;

      if(w>=0 && w<=4)
	    return current_tty->attrib;
      else{
		  panic("get_console_color(): bad console, ");
		  return 0;
	      }
}



__local int kb_inited;
int kb_raw_mode;

void con_init()
{
	int err;
	__local const driver_ops_t ops0 =
	{
		d_name: "console",
		d_author: "easion",
		d_version:KMD_VERSION,
		d_kver:	CURRENT_KERNEL_VERSION,
		d_index:CONSOLE_DEVNO,
		open:kb_open,
		close:kb_close,
		read:con_read,
		write:con_write,
		ioctl:con_ioctl,	
		access:0,
	};

		g_tty_fd=NULL;
		kb_raw_mode=0;

	
	kb_inited=0;

	err = kernel_driver_register(&ops0);

	if(err!=OK)
		panic("register tty failed: err=%d",err);
}

int nrefs=0;
__local int kb_open(char *f, int mode,dev_prvi_t* devfp)
{
	nrefs++;

	if (kb_inited==1)
	{
		return 0;
	}

	//kprintf("kb_open called\n");
	
	kb_init();
	g_tty_fd = devfp;
	return 0;
}
void kb_deinit(void);

__local int kb_close(dev_prvi_t* devfp)
{
	nrefs--;

	if (nrefs!=0)
	{
		return 0;
	}

	if (kb_inited==0)
	{
		return 0;
	}

	g_tty_fd=NULL;
	kb_deinit();
	kb_inited=0;
	return 0;
}

 int con_read(dev_prvi_t* devfp, off_t  pos, void * buf,int count)
{
	count=tty_readbuf (buf, count);

	if (1 && g_tty_fd)
	{
		clear_io_event(&g_tty_fd->iob, IOEVT_READ);
		//kprintf("clear_io_event kbd called\n");
	}

	//clear_io_event(&devfp->iob, IOEVT_READ);
	return count;
}

int con_write(dev_prvi_t* devfp, off_t  pos, void * pbuf,int count)
{
	int i=0;
	u8_t *buf=pbuf;

	while(i<count&&buf[i]!='\0'){
		tty_putchar(buf[i]);
		i++;}
	return count;
}


int con_ioctl(dev_prvi_t* devfp, int cmd, void* args,int size, int kernel_mode)
{	
	int ret=0;
	unsigned short *addr=(unsigned long*)args;
	struct termios *tty = &current_tty->termios;
	
	//kprintf("con_ioctl is %x\n", cmd);

	switch (cmd)
	{		
		case TCGETS:
			return tty_gettermios((struct termios *) current_proc_vir2phys(args), tty);
		case TCSETSF:
			kb_flush();
		case TCSETSW:
			//wait_until_sent(tty); /* fallthrough */
		case TCSETS:
			return tty_settermios((struct termios *) current_proc_vir2phys(args), tty);
		case TCGETA:
			return tty_gettermios((struct termio *) current_proc_vir2phys(args), tty);
		case TCSETAF:
			kb_flush();
		case TCSETAW:
			//wait_until_sent(tty); /* fallthrough */
		case TCSETA:
			return tty_settermios((struct termio *) current_proc_vir2phys(args), tty);
		case TCSBRK:
			if (!args) {
				//wait_until_sent(tty);
				//send_break(tty);
			}
			return 0;
		case TCXONC:
			return -EINVAL; /* not implemented */
		case TCFLSH:
			if (args==0){
				kb_flush();
			}
			else if (args==1){
				//flush(&tty->write_q);
			}
			else if (args==2) {
				kb_flush();
				//flush(&tty->write_q);
			} else
				return -EINVAL;
			return 0;
		case TIOCEXCL:
			return -EINVAL; /* not implemented */
		case TIOCNXCL:
			return -EINVAL; /* not implemented */
		case TIOCSCTTY:
			return -EINVAL; /* set controlling term NI */
		case TIOCGPGRP:
			//verify_area((void *) args,4);
			//put_fs_long(tty->pgrp,(unsigned long *) args);
			return 0;
		case TIOCSPGRP:
			//tty->pgrp=get_fs_long((unsigned long *) args);
			return 0;
		case TIOCOUTQ:
			//verify_area((void *) args,4);
			//put_fs_long(CHARS(tty->write_q),(unsigned long *) args);
			return 0;
		case TIOCSTI:
			return -EINVAL; /* not implemented */
		case TIOCGWINSZ:
			return -EINVAL; /* not implemented */
		case TIOCSWINSZ:
			return -EINVAL; /* not implemented */
		case TIOCMGET:
			return -EINVAL; /* not implemented */
		case TIOCMBIS:
			return -EINVAL; /* not implemented */
		case TIOCMBIC:
			return -EINVAL; /* not implemented */
		case TIOCMSET:
			return -EINVAL; /* not implemented */
		case TIOCGSOFTCAR:
			return -EINVAL; /* not implemented */
		case TIOCSSOFTCAR:
			return -EINVAL; /* not implemented */
		case TRAWON:
			kb_raw_mode=1;
		break;
		case TRAWOFF:
			kb_raw_mode=0;
		break;

		default:
			return -EINVAL;
	
	}
	return ret;
}

