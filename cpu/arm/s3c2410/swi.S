#define 	PID_OFT		0
#define	STATE_OFT		4
#define	COUNT_OFT		8
#define	TIMER_OFT		12
#define	PRIORITY_OFT	16
#define	CONTENT_OFT		20
#define	KERNELSTACK_OFT	100
#define	SIZEOF_TASK_STRUCT	100+1024*4


#define	MAX_SWI_NUM		3	
	
@ �����ַ=������ַ	
#define	SDRAM_BASE		0x30000000
#define	SDRAM_SIZE		0x04000000		

.extern 	swi_table
.text	
.global HandleSWI
HandleSWI:
	stmdb	sp!,{ r0-r12,lr }	@ ����ʹ�õ��ļĴ����ͷ��ص�ַ,���ر�����ô�࣬�Ժ�Ҫ���ʵʱ��ʱ�ٸİ�
	ldr	r4, [lr,#-4]	@ lr-4Ϊָ��"swi n"�ĵ�ַ,��ָ���24λ����n
	bic	r4, r4,#0xff000000	
	cmp	r4, #3
	ldrls	r5, =swi_table	@ swi��ת����ַ
	ldrls	lr, =swi_return	@ ���ص�ַ
	ldrls	pc, [r5,r4,lsl #2]	@ ��ת����Ӧ��ദ������
	mov	r0, #-1		@ ����������-1
swi_return:		
	ldmia	sp!,{ r0-r12,pc }^	@ �жϷ���, ^��ʾ��spsr��ֵ���Ƶ�cpsr

@ �����˳���������,���紦��main�����˳�
.text
.global sys_exit		
sys_exit:
	@ ������C����do_exit(int error_code)
	@ r0=error_code
	ldr	pc, =do_exit	@ do_exit���ص�swi_return

.text
.global sys_sleep		
sys_sleep:		
	@ ������C����int sleep(int time)
	@ r0=sleep time
	ldr	pc, =do_sleep	@ do_exit���ص�swi_return

.text
.global sys_write		
sys_write:		
	@ ������C����int write(char *str)
	@ r0=sleep time
	ldr	pc, =do_write	@ do_exit���ص�swi_return