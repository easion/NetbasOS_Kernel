#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <assert.h>
#include <ansi.h>
#include <string.h>



__local void erase(console_t *con, unsigned count)
{
	ASSERT(con);
	if(!con->vga_ops->char_erase)
		return ;
	con->vga_ops->char_erase(con,count);
	//vga03_erase(con, count);
}

__local void set_attrib(console_t *con, unsigned att)
{
	ASSERT(con);
	if(!con->vga_ops->char_attrib)
		return ;
	con->vga_ops->char_attrib(con,att);
	//vga03_erase(con, count);
}


enum{ST_NONE,ST_END_INPUT,ST_OTHER,ST_CURSOR}Con_status=ST_NONE;

__public int do_vt(console_t *con, unsigned ch)
{
	if(con->magic != CONSOLE_MAGIC)
		panic("do_vt: bad VC 0x%p", con);

	/* state machine to handle the escape sequences */
	switch(con->esc)
	{
	/* await 'ESC' */
	case 0:
		if(ch == 0x1B)
		{
		//kprintf("esc found\n");
			con->esc++;
			return 1; /* "I handled it" */
		}
		break;
	/* got 'ESC' -- await '[' */
	case 1:
		if(ch == '[')
		{
			con->esc1 = 0;
			con->esc++;
			return 1;
		}
		if(ch == '(')
		{
			//不清楚用途,过滤
			con->esc1 = 0;
			con->esc++;
			//kprintf("取得括号 ...\n");
			return 1;
		}
		if(ch == ')')
		{
			//不清楚用途,过滤
			con->esc1 = 0;
			con->esc++;
			Con_status = ST_END_INPUT;
			//kprintf("取得反括号 ...\n");
			return 1;
		}
		break;
	/* got 'ESC[' -- await digit or 'm' or 's' or 'u' or 'H' or 'J' or 'K'
	xxx - support ESC[A ESC[B ESC[C ESC[D */
	case 2:
		if(isdigit(ch))
		{
			con->esc1 = ch - '0';
			con->esc++;
			if (Con_status == ST_END_INPUT)
			{
				con->esc=0;
			}
			//kprintf("次序2数字为%d\n", con->esc1);	
			return 1;
		}
	/* ESC[m -- reset color scheme to default */
		else if(ch == 'm')
		{
			set_attrib(con, 0);
			con->esc = 0;
			return 1;
		}
	/* ESC[s -- save cursor position */
		else if(ch == 's')
		{
			con->save_x = con->cur_x;
			con->save_y = con->cur_y;
			con->esc = 0;
			return 1;
		}
	/* ESC[u -- restore saved cursor position */
		else if(ch == 'u')
		{
			if(con->save_x >= 0)
			{
				con->cur_x = con->save_x;
				con->cur_y = con->save_y;
				con->save_x = -1;
			}
			con->esc = 0;
			return 1;
		}
	/* ESC[H -- home cursor (does not clear screen) */
		else if(ch == 'H')
		{
			con->cur_x = con->cur_y = 0;
			con->esc = 0;
			return 1;
		}
	/* ESC[J -- erase to end of screen, including character under cursor.
	Does not move cursor. */
		else if(ch == 'J')
		{
			erase(con, (VC_HEIGHT - con->cur_y) * VC_WIDTH
				- con->cur_x);
			return 1;
		}
	/* ESC[K -- erase to end of line, including character under cursor.
	Does not move cursor. */
		else if(ch == 'K')
		{
			erase(con, VC_WIDTH - con->cur_x);
			return 1;
		}
		else if(ch == 'A')
		{
			if(0)
				con->cur_y -=1;
			//
			//kprintf("A call\n");
			con->esc = 0;
			return 1;
		}
		else if(ch == 'B')
		{
			if(0)
				con->cur_y += 1;
			//kprintf("b call\n");
			con->esc = 0;
			return 1;
		}
		else if(ch == 'C')
		{
			if(con->cur_x < con->lock_x2)
				con->cur_x += 1;
			con->esc = 0;
			//kprintf("c call\n");
			return 1;
		}
		else if(ch == 'D')
		{
			if(con->cur_x > con->lock_x1)
				con->cur_x -= 1;
			//kprintf("d call\n");
			con->esc = 0;
			return 1;
		}
		break;
	/* got 'ESC[num1' -- collect digits until ';' or 'J' or 'm'
	or 'A' or 'B' or 'C' or 'D' */
	case 3:
		if(isdigit(ch))
		{
			con->esc1 = con->esc1 * 10 + ch - '0';
			return 1;
		}
		else if(ch == 'h')
		{
			//未知的指令，丢弃
			//if(Con_status == ST_OTHER)
			con->esc = 0;
			//kprintf("未知的指令，丢弃%d\n", con->esc1);	
			return 1;
		}

		else if(ch == '?')
		{
			//kprintf("esc find ? in 3 .....\n");
			Con_status=ST_OTHER;
			con->esc2 = 0;
			con->esc++;
			return 1;
		}
		else if(ch == ';')
		{
			con->esc2 = 0;
			con->esc++;
			return 1;
		}
	/* ESC[2J -- clear screen */
		else if(ch == 'J')
		{
			if(con->esc1 == 2)
			{
				con->cur_x = con->cur_y = 0;
				erase(con, VC_HEIGHT * VC_WIDTH);
				con->esc = 0;
				return 1;
			}
		}
	/* ESC[num1m -- set attribute num1 */
		else if(ch == 'm')
		{
			set_attrib(con, con->esc1);
			con->esc = 0;
			return 1;
		}
	/* ESC[num1A -- move cursor up num1 rows */
		else if(ch == 'A')
		{
			if(con->esc1 > con->cur_y)
				con->cur_y = 0;
			else
				con->cur_y -= con->esc1;
			con->esc = 0;
			return 1;
		}
	/* ESC[num1B -- move cursor down num1 rows */
		else if(ch == 'B')
		{
			if(con->esc1 >= con->cur_y + VC_HEIGHT)
				con->cur_y = VC_HEIGHT - 1;
			else
				con->cur_y += con->esc1;
			con->esc = 0;
			return 1;
		}
	/* ESC[num1C -- move cursor right num1 columns */
		else if(ch == 'C')
		{
			if(con->esc1 >= con->cur_x + VC_WIDTH)
				con->cur_x = VC_WIDTH - 1;
			else
				con->cur_x += con->esc1;
			con->esc = 0;
			return 1;
		}
	/* ESC[num1D -- move cursor left num1 columns */
		else if(ch == 'D')
		{
			if(con->esc1 > con->cur_x)
				con->cur_x = 0;
			else
				con->cur_x -= con->esc1;
			con->esc = 0;
			return 1;
		}
		break;
	/* got 'ESC[num1;' -- collect digits until ';' or 'H' or 'f' or 'm' */
	case 4:
		if(isdigit(ch))
		{
			con->esc2 = con->esc2 * 10 + ch - '0';
			return 1;
		}
		else if(ch == ';')
		{
			con->esc3 = 0;
			con->esc++;
			return 1;
		}
	/* ESC[num1;num2H or ESC[num1;num2f -- move cursor to num1,num2 */
		else if(ch == 'H' || ch == 'f')
		{
			if(con->esc2 < 1)
				con->cur_x = 0;
			else if(con->esc2 > VC_WIDTH)
				con->cur_x = VC_WIDTH - 1;
			else
				con->cur_x = con->esc2 - 1;
			if(con->esc1 < 1)
				con->cur_y = 0;
			else if(con->esc1 > VC_HEIGHT)
				con->cur_y = VC_HEIGHT - 1;
			else
				con->cur_y = con->esc1 - 1;
			con->esc = 0;
			return 1;
		}
	/* ESC[num1;num2m -- set attributes num1,num2 */
		else if(ch == 'm')
		{
			set_attrib(con, con->esc1);
			set_attrib(con, con->esc2);
			con->esc = 0;
			return 1;
		}
		else if (ch == 'l' || ch == 'L')
		{
			//kprintf("次序4:参数 l arg%d\n", con->esc1);
			if (Con_status==ST_CURSOR)
			{
				//con->esc1;
			}
			con->esc = 0;
			return 1;
		}
		else if (ch == 'h' || ch == 'H')
		{
			if (Con_status==ST_CURSOR)
			{
				//con->esc1;
			}
			//kprintf("次序4:参数 h arg%d\n", con->esc1);
			con->esc = 0;
			return 1;
		}		
		else if (ch == 'J')
		{
			//kprintf("次序4: J Called\n");
			//ClearScr();
			con->esc = 0;
			con->cur_x = con->cur_y = 0;
			erase(con, VC_HEIGHT * VC_WIDTH);
			return 1;
		}
		else if(ch == 'r'){
			//kprintf("次序4:找到r***\n");
			con->esc = 0;
			return 1;
		}
		else if(ch == 0x0f){
			//kprintf("次序4: 找到0xf***\n");
			con->esc = 0;
			return 1;
		}
		break;
	/* got 'ESC[num1;num2;num3' -- collect digits until 'm' */
	case 5:
		if(isdigit(ch))
		{
			con->esc3 = con->esc3 * 10 + ch - '0';
			return 1;
		}
	/* ESC[num1;num2;num3m -- set attributes num1,num2,num3 */
		else if(ch == 'm')
		{
			set_attrib(con, con->esc1);
			set_attrib(con, con->esc2);
			set_attrib(con, con->esc3);
			con->esc = 0;
			return 1;
		}
		break;
	/* invalid state; reset
	default:
		con->esc = 0;
		break; */
	}
	con->esc = 0;
	return 0; /* "YOU handle it" */
}


