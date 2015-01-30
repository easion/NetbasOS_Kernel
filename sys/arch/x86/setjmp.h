#ifndef __TL_SETJMP_H
#define	__TL_SETJMP_H


typedef struct __jmp_buf
{
	unsigned edi, esi, ebp, esp, ebx, edx, ecx, eax;
	unsigned esp2, eip;
} jmp_buf[1];

static inline int setjmp(jmp_buf buf)
{
 __asm__ __volatile__("movl %%edi,(%0)\n"
		      "movl %%esi,4(%0)\n"
		      "movl %%ebp,8(%0)\n"
  		      "movl %%ebx,16(%0)\n"
		      "movl %%edx,20(%0)\n"
		      "movl %%ecx,24(%0)\n"
		      "leal 8(%%ebp),%%eax\n"
		      "movl %%eax,32(%0)\n"
		      "movl 4(%%ebp),%%eax\n"
		      "movl %%eax,36(%0)\n"
		      :: "b"(buf));
 return 0;
}

static inline void longjmp(jmp_buf buf, int ret_val)
{
 unsigned *esp2;
 if(ret_val == 0) ret_val++;
 buf->eax = ret_val;
 esp2 = (unsigned *)buf->esp2;
 esp2--;
 *esp2 = buf->eip;
 buf->esp2 = (unsigned)esp2;
 __asm__ __volatile__("movl %0,%%esp\n"
   		      "popa\n"
 		      "popl %%esp\n"
 		      "ret\n"::"m"(buf));
}




#endif
