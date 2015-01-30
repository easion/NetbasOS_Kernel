#ifndef SYSTEMTSS_H
#define SYSTEMTSS_H

#define _OLD_
#define FPU_CONTEXT_SIZE	108

//����״̬�νṹ����
// �������񿪹� �����ұ��������ڵ��Ⱥ�ָ�ִ��
 struct tss_struct {
  long backlink;  //�����֣�����ǰ�������ѡ����
  long sp0;  //0����ջָ�루��Ȩ��ߣ�
  long ss0; /* ��16λΪ�� */
  long sp1;  //1����ջָ��
  long ss1; /* ��16λΪ�� */
  long sp2;  //2����ջָ��
  long ss2; /* ��16λΪ�� */
  long cr3;  //CR3�����ڵ�ַӳ��
  long eip;  //�����ǼĴ�����������
  long eflags;
  long eax;
  long ecx;
  long edx;
  long ebx;
  long esp;
  long ebp;
  long esi;
  long edi;
  long es; /* ��16λΪ�� */
  long cs; /* ��16λΪ�� */
  long ss; /* ��16λΪ�� */
  long ds; /* ��16λΪ�� */
  long fs; /* ��16λΪ�� */
  long gs; /* ��16λΪ�� */
  long ldt;  /*��LDTR��ȷ��  ����16λΪ�� */
  unsigned short trap;  //TTS���ر�������
  unsigned short iobase;  //ָ��IO���λͼ��ָ��
  unsigned char  iomap[8192]; //IO���λͼ�Ľ����ֽ�
#if  0
  unsigned long  iomap[33]; //IO���λͼ�Ľ����ֽ�
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
