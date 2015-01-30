#ifndef __K_REGS_H__
#define __K_REGS_H__

#ifdef __AMD64__
typedef struct  {
	unsigned long r15;
	unsigned long r14;
	unsigned long r13;
	unsigned long r12;
	unsigned long rbp;
	unsigned long rbx;
/* arguments: non interrupts/non tracing syscalls only save upto here*/
 	unsigned long r11;
	unsigned long r10;	
	unsigned long r9;
	unsigned long r8;
	unsigned long rax;
	unsigned long rcx;
	unsigned long rdx;
	unsigned long rsi;
	unsigned long rdi;
	unsigned long orig_rax;
/* end of arguments */ 	
/* cpu exception frame or undefined */
	unsigned long rip;
	unsigned long cs;
	unsigned long eflags; 
	unsigned long rsp; 
	unsigned long ss;
/* top of stack page */ 
}regs_t;

#define user_mode(regs) (!!((regs)->cs & 3))
#define instruction_pointer(regs) ((regs)->rip)

//void signal_fault(struct pt_regs *regs, void __user *frame, char *where);

enum {
        EF_CF   = 0x00000001,
        EF_PF   = 0x00000004,
        EF_AF   = 0x00000010,
        EF_ZF   = 0x00000040,
        EF_SF   = 0x00000080,
        EF_TF   = 0x00000100,
        EF_IE   = 0x00000200,
        EF_DF   = 0x00000400,
        EF_OF   = 0x00000800,
        EF_IOPL = 0x00003000,
        EF_IOPL_RING0 = 0x00000000,
        EF_IOPL_RING1 = 0x00001000,
        EF_IOPL_RING2 = 0x00002000,
        EF_NT   = 0x00004000,   /* nested task */
        EF_RF   = 0x00010000,   /* resume */
        EF_VM   = 0x00020000,   /* virtual mode */
        EF_AC   = 0x00040000,   /* alignment */
        EF_VIF  = 0x00080000,   /* virtual interrupt */
        EF_VIP  = 0x00100000,   /* virtual interrupt pending */
        EF_ID   = 0x00200000,   /* id */
};

#endif


#ifdef __IA32__


typedef struct  {
	unsigned  ebx;
	unsigned  ecx;
	unsigned  edx;
	unsigned  esi;
	unsigned  edi;
	unsigned  ebp;
	unsigned  eax;

	unsigned gs, fs, es, ds;
	unsigned  trap_index;	
	unsigned  eax_org;	
	unsigned eip, cs, eflags, user_esp, user_ss;
}regs_t;


typedef struct
{
    //by pusha
	int irq;
	regs_t regs;
} interrupt_regs_t;


//68
#define UNIX_REGS_SIZE sizeof(regs_t)

#endif
typedef struct
{
	union{
   struct {
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long res;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
  } d;
  struct {
    unsigned short di, di_hi;
    unsigned short si, si_hi;
    unsigned short bp, bp_hi;
    unsigned short res, res_hi;
    unsigned short bx, bx_hi;
    unsigned short dx, dx_hi;
    unsigned short cx, cx_hi;
    unsigned short ax, ax_hi;
  } x;
  struct {
    unsigned char edi[4];
    unsigned char esi[4];
    unsigned char ebp[4];
    unsigned char res[4];
    unsigned char bl, bh, ebx_b2, ebx_b3;
    unsigned char dl, dh, edx_b2, edx_b3;
    unsigned char cl, ch, ecx_b2, ecx_b3;
    unsigned char al, ah, eax_b2, eax_b3;
  } h;
	}uregs;

	unsigned gs, fs, es, ds;
    unsigned trap_index, error_code;
	unsigned eip, cs, eflags, user_esp, user_ss;
} bios_regs_t;




typedef union vm86_regs{
  struct {
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long res;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
  } d;
  struct {
    unsigned short di, di_hi;
    unsigned short si, si_hi;
    unsigned short bp, bp_hi;
    unsigned short res, res_hi;
    unsigned short bx, bx_hi;
    unsigned short dx, dx_hi;
    unsigned short cx, cx_hi;
    unsigned short ax, ax_hi;
    unsigned short flags;
    unsigned short es;
    unsigned short ds;
    unsigned short fs;
    unsigned short gs;
    unsigned short ip;
    unsigned short cs;
    unsigned short sp;
    unsigned short ss;
  } x;
  struct {
    unsigned char edi[4];
    unsigned char esi[4];
    unsigned char ebp[4];
    unsigned char res[4];
    unsigned char bl, bh, ebx_b2, ebx_b3;
    unsigned char dl, dh, edx_b2, edx_b3;
    unsigned char cl, ch, ecx_b2, ecx_b3;
    unsigned char al, ah, eax_b2, eax_b3;
  } h;
} vm86regs_t;


int do_int86(int service,vm86regs_t *in,vm86regs_t *out);
void set_cr0(u32_t cr0);
 unsigned long get_cr0 (void);
u32_t get_int86_sp0(void);
__asmlink void read_real_memory(char *buf, int sz);
__asmlink void fill_real_memory(char *buf, int sz);
__asmlink void real_memory_addr(u32_t *addr, int *sz);
#define DISABLE_BEGIN 
#if 0
#define DISABLE_END 
#endif

#endif


