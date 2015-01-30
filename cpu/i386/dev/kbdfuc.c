/*
**     (R)Jicama OS
**     Keyboard IO Function
**     Copyright (C) 2003 DengPingPing
*/

#include <arch/x86/io.h>
#include <arch/x86/keyboard.h>
#include <jicama/system.h>

#define MAX_HOTKEY 64

__local u32_t hotkey[MAX_HOTKEY];

__asmlink unsigned char getchar(void);
__asmlink void disable();

int add_keyfun(int key, u32_t fun)
{
	return SUCCESS;
}

void waitkey(void)                      /* �ȴ�����          */
{
   unsigned char kbc_status;                    /* ���̿�����״̬              */
   do                                   /* ѭ��ֱ������Ϊֹ*/
   {                                    /* Including Shift, Ctrl etc.   */
      while(inb(0x64) & 0x02);      /* ��ȡ������״̬               */
      kbc_status = inb(0x64);
   }                                    /* ����ֱ��λ0���趨  */
   while(!(kbc_status & 1));            /* ����ǳ����㣬�������ܹ����� */
}

void disable_kb(void)
{
	waitempty();
    outb(0x64, 0xAD);
}

void kb_waitfull(void)
{  
	char status;
do{
     status = inb(0x64);
}while(status & 0x01); 
}

void do_reboot(void)
{
	unsigned char temp, answer = 0;

	kprintf ("\nReboot System... ...");

	disable();    /* flush the keyboard controller */
	do
	{
		temp = inb(0x64);
		if((temp & 0x01) != 0)
		{
			(void)inb(0x60);
			continue;
		}
	} while((temp & 0x02) != 0);   /* pulse the CPU reset line */
	outb(0x64, 0xFE);    /* ...and if that didn't work, just halt */
 
   while(1)
      __asm("cli;hlt");   /* CPU ��ʼ���� */
   }


