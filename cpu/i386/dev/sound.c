
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------
#include <arch/x86/io.h>
void mdelay(int msec);


#define S1   24
#define S2   12
#define S4    6
#define S8    3
#define S16  S1/16

enum FREQDEF
{
L1=131,L2=147,L3=165,L4=175,L5=196,L6=220,L7=247,

N1=262,N2=296,N3=330,N4=349,N5=392,N6=440,N7=494,

H1=523,H2=587,H3=659,H4=698,H5=784,H6=880,H7=988,

I1=1047,I2=1175,I3=1319,I4=1397,I5=1568,I6=1760,I7=1976
}song[]=   //Music speak at 1,3,5.But time mdelay in 2,4,6
{
   N2,  6,  N2,  3,  N3,  1,  N2,  1,  N1,  6,  L6,  6,  L5,  3,
   L3,  3,  L5,  3,  L6,  3,  N1,  12,  L6,  6,  L6,  3,  N1,  3,
   N5,  3,  N6,  3,  N3,  3,  N5,  3,  N2,S1,  N3,  6,  N3,  3,
   N2,  3,  N3,  3,  N5,  6,  N3,  6,  L5,  3,  L3,  3,  L5,  3,
   L6,  3,  N1,  12,  L6,  6,  L6,  3,  N1,  3,  L6,  3,  L6,  3,
   L2,  3,  L3,  3,  L5,S1,  N2,  6,  N2,  6,  N5,  6,  N6,  3,
   N5,  3,  N4,  12,  N5,  12,  N6,  6,  N5,  3,  N3,  3,  N2,  3,
   N3,  1,  N2,  1,  N1,  3,  L6,  1,  N1,  1,  N2,S1,  N3,  6,
   N2,  3,  N3,  3,  N5,  6,  N3,  6,  L5,  3,  L3,  3,  L5,  3,
   L6,  3,  N1,  12,  L6,  6,  L6,  3,  N1,  3,  N2,  3,  L6,  3,
   N1,  3,  N3,  3,  N2,  24
};

__local void begin_sound (const unsigned short freq) 
	{
  unsigned short freqdiv;
  unsigned char tmp;

  freqdiv = 1193182 / freq;    ////////1193180 参数为16位
  outb(0x43, 0xB6); ////初始化定时器

  outb(0x42, (unsigned char)freqdiv); ///////将其低位写入寄存器
  outb(0x42, (unsigned char)(freqdiv >> 8)); //////将高位写入寄存器
  tmp = inb (0x61);    //////读入控制数据

  if (tmp != (tmp | 3)) /////////如果位没有正确设定则只能输出
    outb (0x61, (tmp | 3));  ///低2位置1,开始发声
}


__local void stop_sound ()
{
  unsigned char tmp;
  tmp = inb (0x61); ///////键盘控制寄存器
  tmp &= 252; /* disable speaker + clock 2 gate */
  outb (0x61, tmp); /* output */
}

void speak (u16_t freq, unsigned int time) 
{
    begin_sound (freq);
    mdelay (time);
    stop_sound ();
}

void mysong(void)
{
  int i;

    for(i=0;song[i];i+=2)
       speak(song[i],song[i+1]);
}

#define CON_BEEP_FREQ	0x533
#define CON_TAPE_A_FREQ	0x580
#define CON_TAPE_B_FREQ	0x420

#define CON_BEEP_TIME	20

void beep()
{
	begin_sound(CON_BEEP_FREQ);
	mdelay(CON_BEEP_TIME);
	stop_sound();
}

