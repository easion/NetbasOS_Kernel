
#include <jicama/process.h>
#include "s3c2410.h"
#include "interrupt.h"
#include "mmu.h"
char task[1024];

void IRQ_Handle()
{
	unsigned long oft = INTOFFSET;
	switch( oft )
	{
		case EINT1_OFT:	kprintf("EINT1, K1 pressed!\n\r");	break;
		case EINT2_OFT:	kprintf("EINT2, K2 pressed!\n\r");	break;
		case EINT3_OFT:	kprintf("EINT3, K3 pressed!\n\r");	break;
		case EINT4_7_OFT:	kprintf("EINT7, K4 pressed!\n\r");	break;
		case INT_TIMER0_OFT:
			//do_timer();	
		break;
		default:		kprintf("Interrupt unknown!\n\r");	break;
			
	}

	//DPRINTK(KERNEL_DEBUG,"kernel:clean the irq\n\r");
	//清中断
	if( oft == 4 ) EINTPEND = 1<<7;		//EINT4-7合用IRQ4，注意EINTPEND[3:0]保留未用，向这些位写入1可能导致未知结果
	SRCPND = 1<<oft;	
	INTPND	= INTPND;	 
}

/*上电后，WATCH DOG默认是开着的，要把它关掉 */
void disable_watch_dog(void)
{
	WTCON	= 0;
}

/**************************************************************************   
* 设置控制SDRAM的13个寄存器
* 使用位置无关代码
**************************************************************************/   
void memsetup_2(void)
{
	unsigned long *p = (unsigned long *)MEM_CTL_BASE;	
	p[0] = 0x22111110;		//BWSCON
	p[1] = 0x00000700;		//BANKCON0
	p[2] = 0x00000700;		//BANKCON1
	p[3] = 0x00000700;		//BANKCON2
	p[4] = 0x00000700;		//BANKCON3	
	p[5] = 0x00000700;		//BANKCON4
	p[6] = 0x00000700;		//BANKCON5
	p[7] = 0x00018005;		//BANKCON6
	p[8] = 0x00018005;		//BANKCON7
	p[9] = 0x008e04f4;		//REFRESH,HCLK=12MHz:0x008e07a3,HCLK=100MHz:0x008e04f4
	p[10] = 0x000000b2;		//BANKSIZE
	p[11] = 0x00000030;		//MRSRB6
	p[12] = 0x00000030;		//MRSRB7
}

#define MPLL_200MHz	(0x5c << 12)|(0x04 << 4)|(0x00)
#define MPLL_100MHz	(0x5c << 12)|(0x04 << 4)|(0x01)
/***************************************************************************
* 对于MPLLCON寄存器，[19:12]为MDIV，[9:4]为PDIV，[1:0]为SDIV
* 有如下计算公式：
*	MPLL(FCLK) = (m * Fin)/(p * 2^s)
*	其中: m = MDIV + 8, p = PDIV + 2, s = SDIV
* 对于本开发板，Fin = 12MHz,MPLLCON设为MPLL_200MHz，可以计算出FCLK=200MHz
***************************************************************************/
void reset_clock(void)
{
	LOCKTIME = 0x00ffffff;//0x00ffffff;
	CLKDIVN  = 0x03;	/*FCLK:HCLK:PCLK=1:2:4, HDIVN1=0,HDIVN=1,PDIVN=1 */
 
	/*If HDIVN = 1,the CPU bus mode has to be changed from the fast bus mode
	  to the asynchronous bus mod using following instructions.*/
__asm__(
	"mrc	p15, 0, r1, c1, c0, 0\n"		/* read ctrl register   */ 
	"orr	r1, r1, #0xc0000000\n"		/* Asynchronous         */
	"mcr	p15, 0, r1, c1, c0, 0\n"		/* write ctrl register  */
	:::"r1"
	);

	MPLLCON = MPLL_200MHz;	/*现在，FCLK=200MHz,HCLK=100MHz,PCLK=50MHz*/
}

/*************************************************************************
* Timer input clock Frequency = PCLK / {prescaler value+1} / {divider value}
* {prescaler value} = 0~255
* {divider value} = 2, 4, 8, 16
* 本实验的Timer0的时钟频率=50MHz/(49+1)/(16)=62500Hz
* 设置Timer0 10ms触发一次中断：TCNTB0[15:0]=625
*************************************************************************/
void Timer0_init(void)
{
	TCFG0 = 49;		//Prescaler0 = 49	  	 
	TCFG1 = 0x03;	//Select MUX input for PWM Timer0:divider=16
//	TCNTB0 = 62; 	//1ms触发一次中断
	TCNTB0 = 625;	//10ms触发一次中断
//	TCNTB0 = 6250;	//100ms触发一次中断
//	TCNTB0 = 62500;	//1s触发一次中断,用于调试	
	TCON |=  (1<<1);	//Timer 0 manual update
	TCON = 0x09;	/*Timer 0 auto reload on
			  Timer 0 output inverter off
			  清"Timer 0 manual update"
			  Timer 0 start */
}


#define EINT1		(2<<(1*2))
#define EINT2		(2<<(2*2))
#define EINT3		(2<<(3*2))
#define EINT7		(2<<(7*2))

void init_irq(void)
{	
	INTMSK &= (~(1<<10));	//INT_TIMER0使能
}


/* 在第一次实用NAND Flash前，复位一下NAND Flash */
void reset_nand(void)
{
	int i=0;
	NFCONF &= ~0x800;
	for(; i<10; i++);
	NFCMD = 0xff;	//reset command
	wait_idle();
}


/*
 * bit[15]:Enable Nand Flash Controller
 * bit[12]:Initialize ECC
 * bit[11]:Nand Flash nFCE = HIGH(inactive)
 * bit[10:8]:TACLS,Duration = HCLK*(TACLS+1)
 * bit[6:4]:TWRPH0,Duration = HCLK*(TWRPH0+1)
 * bit[2:0]:TWRPH1,Duration = HCLK*(TWRPH1+1) 
 */
//#define	NANDCONFIGVAL	((1<<15)|(1<<12)|(1<<11)|(0x0<<8)|(0x3<<4)|(0x0))
#define	NANDCONFIGVAL	((1<<15)|(1<<12)|(1<<11)|(0x7<<8)|(0x7<<4)|(0x7))
/* 初始化NAND Flash */
void init_nand(void)
{
	NFCONF = NANDCONFIGVAL;
	reset_nand();
}


/*************************************************************************
*	以下读NAND Flash的代码来自mizi公司的bootloader vivi
*************************************************************************/
#define BUSY 1
void wait_idle(void) {
//	int i;

	while(!(NFSTAT & BUSY));
//		for(i=0; i<10; i++);
}


/***************************************************************************
* 中断向量起始虚拟地址为0xffff0000，段是1M对齐的，所以0xfff00000虚拟地址对应
* 0x33f00000(VECTORS_PHY_BASE),而虚拟地址0xffff0000则对应0x33f00000+0xf0000
***************************************************************************/
void copy_vectors_from_nand_to_sdram(void)
{
	nand_read_ll((unsigned char*)(VECTORS_PHY_BASE+0xf0000), 0x0, 512);	
}

void copy_process_from_nand_to_sdram(void)
{
	nand_read_ll((unsigned char*)PROCESS0_BASE, 0x0, 0x100000-16*1024);//进程0空间为1M-16K页表
//	nand_read_ll((unsigned char*)(0x30100000), 15*1024, 1024);
//	nand_read_ll((unsigned char*)PROCESS2_BASE, 0x8000, 1024);	
}




#define NAND_SECTOR_SIZE	512
#define NAND_BLOCK_MASK		(NAND_SECTOR_SIZE - 1)

/* low level nand read function */
void nand_read_ll(unsigned char *buf, unsigned long start_addr, unsigned long size)
{
	int i, j;

	if ((start_addr & NAND_BLOCK_MASK) || (size & NAND_BLOCK_MASK)) {
		return ;	/* invalid alignment */
	}

	/* chip Enable */
	NFCONF &= ~0x800;
	for(i=0; i<10; i++);

	for(i=start_addr; i < (start_addr + size);) {
		/* READ0 */
		NFCMD = 0;

		/* Write Address */
		NFADDR = i & 0xff;
		NFADDR = (i >> 9) & 0xff;
		NFADDR = (i >> 17) & 0xff;
		NFADDR = (i >> 25) & 0xff;

		wait_idle();
//		while(!(NFSTAT & 1));

		for(j=0; j < NAND_SECTOR_SIZE; j++, i++) {
			*buf = (NFDATA & 0xff);
			buf++;
		}
	}

	/* chip Disable */
	NFCONF |= 0x800;	/* chip disable */
}



#if 0
__public int  arch_thread_create2(thread_t* th_new,void (*fn)(void *),
	void *args,  unsigned stack,  proc_t *rp)
{
	th_new->reg->content[0] = (unsigned long)th_new;//(&task[pid+1]);
	th_new->reg->content[1] = 0x5f;   	/*cpsr*/
	th_new->reg->content[2] = 0x100000-1024; /*usr/sys模式堆栈*/
	th_new->reg->content[3] = 0x13;	/*svc模式*/
	th_new->reg->content[18]= (unsigned long)fn;	/*pc*/
}
#endif

