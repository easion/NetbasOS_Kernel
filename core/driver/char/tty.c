
// -------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//--------------------------------------------------------------------------------------
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include<string.h>

static input_func *inputmanager=NULL;


int null_putchar( void *con, unsigned char ch, bool vt)
{
	/*呵呵 无事可作 */
	return 0;
}

 

__local struct tty_ops null_ops = {
	name:"null",
	putchar: null_putchar,
};

__local unsigned char g_vc0_buf[KBD_BUF_SIZE];


int unregster_console_device(console_t *con)
{
	register int i;

	if (con==(console_t *)0)
		return -1;

	for(i = 0; i <NR_TTY ; i++)
	{      
		if(tty[i] == con)
			break;
	}
	if (i==NR_TTY)
		return -2;

	if (con->vga_ops->exit)
	{
		con->vga_ops->exit();
	}

	tty[i] = NULL;
	return 0;
}


int regster_console_device(console_t *con)
{
	register int i;

	if (con==(console_t *)0)
		return -1;

	for(i = 0; i <NR_TTY ; i++)
	{      
		if(tty[i] == NULL)
			break;
	}


	if (i==NR_TTY)
		return -2;

	tty[i] =con;

	if (!con->vga_ops->readbuf)
	{
		/*default, input from keyboard*/
		con->vga_ops->readbuf = kb_read;
	}
	con->magic =CONSOLE_MAGIC;//fix
	con->index = i;

	con->keystrokes.data=g_vc0_buf;
	con->keystrokes.size=KBD_BUF_SIZE;
	con->keystrokes.in_ptr=NULL;
	con->keystrokes.out_ptr=NULL;

	con->cur_x = con->cur_y = 0;
	thread_waitq_init(&con->wait);
	termios_init(&con->termios);
	return i;
}

void tty_init(void)
{
	int i;
      
	for(i = 0; i <NR_TTY ; i++)
	{      
		tty[i] = NULL;
	}
	
}

int submit_input_enent(struct Event*e)
{
	int err; //高16位为设备号
	
	//默认状态
	 /*nothing to do*/		
	if(!inputmanager)
		return -1;

	err =(*inputmanager)(e);
	return err;
}


int set_input_manager(input_func *inputfuc)
{
	if(inputmanager)
		return EBUSY;
	
	//设置事件控制
	if(!inputfuc)
		return EINVAL;
	inputmanager=inputfuc;
	return 0;
}

int reset_input_manager()
{
	//设置事件控制
	inputmanager=NULL;
}

