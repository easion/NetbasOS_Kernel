
#ifndef  _setjmp_h_
#define _setjmp_h_



typedef struct {
	unsigned long  eip;
	//GP, used it by pusha
	unsigned long  edi;
	unsigned long  esi;
	unsigned long  ebp;
	unsigned long  esp;
	unsigned long  ebx;
	unsigned long  edx;
	unsigned long  ecx;
	unsigned long  eax;
} jmp_buf[1];


#endif


