#define 	PID_OFT		0
#define	STATE_OFT		4
#define	COUNT_OFT		8
#define	TIMER_OFT		12
#define	PRIORITY_OFT	16
#define	CONTENT_OFT		20
#define	KERNELSTACK_OFT	100
#define	SIZEOF_TASK_STRUCT	100+1024*4


#define	MAX_SWI_NUM		3	
	
@ 虚拟地址=物理地址	
#define	SDRAM_BASE		0x30000000
#define	SDRAM_SIZE		0x04000000		

.extern 	swi_table
.text	
.global HandleSWI
HandleSWI:
	stmdb	sp!,{ r0-r12,lr }	@ 保存使用到的寄存器和返回地址,不必保存那么多，以后要提高实时性时再改吧
	ldr	r4, [lr,#-4]	@ lr-4为指令"swi n"的地址,此指令低24位就是n
	bic	r4, r4,#0xff000000	
	cmp	r4, #3
	ldrls	r5, =swi_table	@ swi跳转表基址
	ldrls	lr, =swi_return	@ 返回地址
	ldrls	pc, [r5,r4,lsl #2]	@ 跳转到相应汇编处理函数
	mov	r0, #-1		@ 出错，返回-1
swi_return:		
	ldmia	sp!,{ r0-r12,pc }^	@ 中断返回, ^表示将spsr的值复制到cpsr

@ 进程退出处理函数,比如处理main函数退出
.text
.global sys_exit		
sys_exit:
	@ 将调用C函数do_exit(int error_code)
	@ r0=error_code
	ldr	pc, =do_exit	@ do_exit返回到swi_return

.text
.global sys_sleep		
sys_sleep:		
	@ 将调用C函数int sleep(int time)
	@ r0=sleep time
	ldr	pc, =do_sleep	@ do_exit返回到swi_return

.text
.global sys_write		
sys_write:		
	@ 将调用C函数int write(char *str)
	@ r0=sleep time
	ldr	pc, =do_write	@ do_exit返回到swi_return
