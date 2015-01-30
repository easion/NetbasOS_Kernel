#ifndef SYSTEMTSS_H
#define SYSTEMTSS_H

#define _OLD_
#define FPU_CONTEXT_SIZE	108

//任务状态段结构定义
// 控制任务开关 ，并且保存数据在调度后恢复执行
 struct tss_struct {
  long backlink;  //连接字，保留前面任务的选择子
  long sp0;  //0级堆栈指针（特权最高）
  long ss0; /* 高16位为零 */
  long sp1;  //1级堆栈指针
  long ss1; /* 高16位为零 */
  long sp2;  //2级堆栈指针
  long ss2; /* 高16位为零 */
  long cr3;  //CR3，用于地址映射
  long eip;  //以下是寄存器保存区域
  long eflags;
  long eax;
  long ecx;
  long edx;
  long ebx;
  long esp;
  long ebp;
  long esi;
  long edi;
  long es; /* 高16位为零 */
  long cs; /* 高16位为零 */
  long ss; /* 高16位为零 */
  long ds; /* 高16位为零 */
  long fs; /* 高16位为零 */
  long gs; /* 高16位为零 */
  long ldt;  /*由LDTR来确定  ，高16位为零 */
  unsigned short trap;  //TTS的特别属性字
  unsigned short iobase;  //指向IO许可位图的指针
  unsigned char  iomap[8192]; //IO许可位图的结束字节
#if  0
  unsigned long  iomap[33]; //IO许可位图的结束字节
	unsigned long	tr;
	unsigned long	cr2, trap_no, error_code;
#endif
//unsigned long control __attr_packet;
   // unsigned char ctx_FPU[FPU_CONTEXT_SIZE] __attr_packet;
}__attr_packet;;

//#define IS_BAD_PROC_ENTRY(addr) ((u32_t)(addr)>=PROC_FIX_MEM_SIZE)
#define proc_phy_addr(rp) \
      get_desc_base(&(rp->p_ldt[DS_LDT_INDEX]))

#define proc_vir2phys(p, vir) \
(u32_t)(get_desc_base(&(p->p_ldt[DS_LDT_INDEX]))+ (size_t) (vir))

#endif
