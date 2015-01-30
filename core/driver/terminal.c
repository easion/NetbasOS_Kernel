
// ------------------------------------------------------------------------------------------
//Jicama OS  
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//---------------------------------------------------------------------------------------------

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/spin.h>
#include <string.h>
#include <clist.h>

CREATE_SPINLOCK( kbd_lock_sem );

int clist_size(struct cblock *c);

/*flush input*/
void kb_flush(void)
{
	key_queue_t *q =&current_tty->keystrokes;
	q->out_ptr = q->in_ptr=0;
}

bool iskbhit()
{
	key_queue_t *q;
	
	q=&current_tty->keystrokes;
	if(q->out_ptr == q->in_ptr)
		return FALSE;

	return TRUE;
}

int kb_del_char(unsigned char *tmp)
{
	unsigned flags=save_eflags(NULL);
	deq(&current_tty->keystrokes, tmp);
	restore_eflags(flags);
	return 0;
}

int kb_add_char(unsigned char tmp)
{
	unsigned flags=save_eflags(NULL);
	inq(&current_tty->keystrokes, tmp);
	restore_eflags(flags);
	return 0;
}

 int kb_read_ch()
{
	int ch = 0;
	unsigned  empty, flags;
	console_t *con;
	unsigned num;
	time_t t=0;

	con = current_tty;

		do
		{
			/* atomic test for empty queue */
			 save_eflags(&flags);
			 t = INFINITE;
			empty = (con->keystrokes.out_ptr ==
				con->keystrokes.in_ptr);
			num = 10;
			restore_eflags(flags);
			if(empty){
			/* if empty, wait on it forever (no timeout) */
				thread_sleep_on(&con->wait);
				continue;
			}
			
		} while(deq(&con->keystrokes, &ch));

		//thread_wakeup(&con->wait);

	return ch;
}
#include <conio.h> 
#define TTY_JUST_GET_ONE 0x1000

#define KEYCODES	27

#define KEY_RETURN KEY_ENTER 
#define	KEY_ESCAPE		0x1b
#define	KEY_TAB	    '\t' //
#define	KEY_ENTER	    '\n' //
#define	KEY_BACKSPACE	    '\b' //
static unsigned char keycodes_netbas[KEYCODES] = {KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
					      KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
                                              KEY_F11, KEY_F12, KEY_TAB, KEY_END, KEY_HOME,
                                              KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_RETURN,
                                              KEY_BACKSPACE, KEY_INS,  KEY_DEL, KEY_ESCAPE,
                                              KEY_PGUP, KEY_PGDN, KEY_ENTER};


static char *keycodes_s[KEYCODES] = {"\033[[A", "\033[[B", "\033[[C", "\033[[D", "\033[[E",
	"\033[17~", "\033[18~",     "\033[19~", "\033[20~", "\033[21~",
	"\033[23~", "\033[24~", "\t", "\033[4~",    "\033[1~", 
	"\033[A", "\033[B", "\033[D", "\033[C", "\n",
	"\010", "\033[2~",    "\033[3~", "\033",
	"\033[5~", "\033[6~", "\n"};

extern int kb_raw_mode;

int vt100_conv (int ch, char *buf)
{
	int i;
	int n=1;

	if (kb_raw_mode)
	{
		kprintf("kb_raw_mode = %x\n", ch);
		goto raw;
	}
	

	for (i=0; i<KEYCODES; i++)
	{
		if (keycodes_netbas[i] == ch)
		{
			n = strlen(keycodes_s[i]);
			//kprintf("kb_raw_mode found = %x\n", ch);
			memcpy(buf, keycodes_s[i],n);
			break;
		}
	}

	if (i!=KEYCODES)
	{
		return n;
	}

raw:
	memcpy(buf, &ch,1);
	return n;
}


/*read key*/
int kb_read(u8_t* buffer, int max_len)
 {
	int r,i=0;
	console_t *con;
	unsigned char key;
	static struct cblock cblock;
	long flag = 0;//0x000000ff & max_len;

	if (max_len==1)
	{
		flag |= TTY_JUST_GET_ONE;
	}

    spin_lock( &kbd_lock_sem );

	//V(&kb_sem);

	con = current_tty;
	con->lock_x1 = con->lock_x2 =con->cur_x; 
	init_clist(&cblock);

	 while(TRUE){
	 	/*delete from queue*/
		 key=kb_read_ch();
		 if (key==-1)
		 {
			 //kprintf("null read");
			 //continue;
		 }
		/*commit a key*/	
		con->lock_x2 += clist_input(&cblock, key, flag);	
		/*end*/	
		if(key == '\n' || key == '\r' || clist_size(&cblock) >= max_len){
			break;	
		}		
	 }
	 
	con->lock_x1 = con->lock_x2 = 0;
	max_len = clist_done(&cblock, buffer, 1);
	
	spin_unlock( &kbd_lock_sem );
	//P(&kb_sem);

	return max_len;
 }

struct key_func
{
	u8_t value;
	func_t ptr;
};

#define NR_KEYF 32

struct key_func key_list[NR_KEYF];

void reset_keylist()
{
	int i;
	for (i=0; i<NR_KEYF; i++)
	{
		key_list[i].value=0;
		key_list[i].ptr=(func_t)0;
	}
}


int key_cook(	int value,	func_t p)
{
	int i;
	for (i=0; i<NR_KEYF; i++)
	{
		if (key_list[i].value==value)
			return -1;
		if (key_list[i].ptr==NULL)
			break;
	}

	if (i==NR_KEYF)	{return -1;}

	key_list[i].value=value;
	key_list[i].ptr=p;

	return 0;
}

int key_message(u8_t key)
{
	int i;

	if (!key)return 0;

	for (i=0; i<NR_KEYF; i++)
	{
		if (key_list[i].value==key)
			break;	
	}
	if (i==NR_KEYF)	{return 0;}
	(*key_list[i].ptr)();
	return 1;
}

