
// ---------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//----------------------------------------------------------------------------------

#include <jicama/system.h>
#include <arch/x86/regs.h>
#include <string.h>

 void lldt (unsigned short ldt_sel)
{
	 __asm("lldt %%ax"::"a"(ldt_sel));
}


 void ltr( int tss_nr )
{
    __asm ( "ltr %%ax" :: "a" ( tss_nr << 3 ) );
}

  unsigned long get_cr0 (void)
{
  register unsigned long x;
  __asm __volatile__ ("movl %%cr0,%0" : "=r" (x));
  return (x);
}

  void set_cr0(u32_t cr0)
{
  __asm volatile ("mov %0, %%cr0" : : "r" (cr0));
}

  void set_cr2(u32_t cr2)
{
  __asm volatile ("mov %0, %%cr2" : : "r" (cr2));
}


void set_cr3(u32_t cr3)
{
  __asm volatile ("mov %0, %%cr3" : : "r" (cr3));
}

  u32_t get_cr4()
{
  u32_t cr4;
  __asm volatile ("mov %%cr4, %0" : "=r" (cr4));
  return cr4;
}

void set_cr4(u32_t cr4)
{
  __asm volatile ("mov %0, %%cr4" : : "r" (cr4));
}

 unsigned long get_cr2 (void)
{
  unsigned long x;
  __asm __volatile__ ("movl %%cr2,%0" : "=r" (x));
  return (x);
}

 unsigned long get_cr3 (void)
{
  register unsigned long x;
  __asm __volatile__ ("movl %%cr3,%0" : "=r" (x));
  return (x);
}

 u16_t get_tr()
{
  uint16_t tr;
  __asm volatile ("str %0" : "=rm" (tr));
  return tr;
}

 u32_t get_sp()
{
  u32_t sp;
  __asm volatile ("mov %%esp, %0" : "=r" (sp));
  return sp;
}


#if 0

 unsigned short get_cs (void)
{
  register unsigned short x;
  __asm __volatile__ ("movl %%cs,%%ax" : "=a" (x));
  return (x);
}

  unsigned short get_ds (void)
{
  register unsigned short x;
  __asm __volatile__ ("movl %%ds,%%ax" : "=a"(x));
  return x;
}

 void get_gs(unsigned short *sel)
 {
	 __asm("movl %%gs, %0" :"=r"(*sel));
 }

 void put_gs(unsigned short sel)
 {
	 __asm("movl %0, %%gs" ::"g"(sel));
 }

unsigned char get_byte_bysel(unsigned short sel, const char *addr)
 {
	 unsigned char ret;

	 __asm("movl %1, %%gs\n\t"
	          "movb %%gs:%2, %0\n\t "
			  :"=r"(ret):"m"(sel),"m"(*addr)
		 );
			  return ret;
 }

 unsigned short get_word_bysel(unsigned short sel,  const unsigned short *addr)
 {
	 unsigned short ret;

	 __asm("movl %1, %%gs\n\t"
	          "movw %%gs:%2, %0\n\t "
			  :"=r"(ret):"m"(sel),"m"(*addr)
		 );
			  return ret;
 }

unsigned long get_long_bysel(unsigned short sel,  const unsigned long *addr)
 {
	 unsigned long ret;

	 __asm("movl %1, %%gs\n\t"
	          "movl %%gs:%2, %0\n\t "
			  :"=r"(ret):"m"(sel),"m"(*addr)
		 );
			  return ret;
 }
#endif

unsigned long read_user_dword(const u32_t *addr)
 {
	 unsigned long ret;

	 __asm("movl %%gs:%1, %0\n\t "
			  :"=r"(ret):"m"(addr)
		 );
			  return ret;
 }

unsigned short read_user_word(const u16_t *addr)
 {
	 unsigned long ret;

	 __asm("movw %%gs:%1, %0\n\t "
			  :"=r"(ret):"m"(addr)
		 );
	return ret;
 }

 unsigned char read_user_byte(const char *addr)
 {
	 unsigned char ret;

	 __asm("movb %%gs:%1, %0\n\t "
			  :"=r"(ret):"m"(addr)
		 );
			  return ret;
 }

 void write_user_byte(char val,char *addr)
{
    __asm ("movb %b0,%%gs:%1"::"q" (val),"m" (addr));
}


void write_user_word(unsigned short val,unsigned short * addr)
{
	__asm("movw %0,%%gs:%1"::"r" (val),"m" (addr));
}

 void write_user_dword(unsigned long val,unsigned long * addr)
{
  __asm("movl %0,%%gs:%1"::"r" (val),"m" (addr));
}

 void dump_regs(regs_t *reg)
{
	syslog(LOG_ERR,"|Trap error code:%d eflags: %x  |\n", reg->trap_index,reg->eflags);
 	syslog(LOG_ERR,"|eip:0X%X CS: 0X%X SS: 0X%X, DS: 0X%X ES:0X%X|\n",reg->eip, reg->cs, reg->user_ss, reg->ds, reg->es);
 	syslog(LOG_ERR,"|ebp: 0X%X user_esp 0X%X esi: 0X%X, edi:0X%X|\n", reg->ebp,  reg->user_esp, reg->esi, reg->edi);
	syslog(LOG_ERR,"|eax:0X%X ebx:0X%X ecx:0X%X edx:0X%X|\n",reg->eax_org, reg->ebx, reg->ecx, reg->edx);
	return;
}

 void* memcpy_from_user( void *dst_ptr, const void *src_ptr, unsigned long count )
{
	void *ret_val = dst_ptr;
	const char *src = (const char *)src_ptr;
	char *dst = (char *)dst_ptr;

	for(; count != 0; count--){
		*dst++ = read_user_byte(src++);
	}

	return ret_val;
}

 void* memcpy_to_user( void *dst_ptr, const void *src_ptr, unsigned long count )
{
	void *ret_val = dst_ptr;
	const char *src = (const char *)src_ptr;
	char *dst = (char *)dst_ptr;

	for(; count != 0; count--){
		write_user_byte(*src++, (char *)dst++);
	}

	return ret_val;
}

 char *strncpy_from_user(char *src_char, const char *add_char, size_t nr)
{
	int i = 0;
	char *ret = src_char;

	while (*add_char != '\0' && i<nr){
		*src_char++ = read_user_byte(add_char++);
		i++;
	}

	*src_char++ = '\0';

	return ret;
}

 char *strncpy_to_user(char *src_char, const char *add_char, size_t nr)
{
	int i = 0;
	char *ret = src_char;

	while (*add_char != '\0' && i<nr){
		//*src_char++ = *add_char++;
		write_user_byte(*src_char++, (char *)add_char++);
		i++;
	}

	*src_char++ = '\0';

	return ret;
}

 void* strcpy_to_user( char* pzDst, const char* pzSrc )
{
    return( memcpy_to_user( pzDst, pzSrc, (size_t)strlen( pzSrc ) ) );
}
//#include <jicama/process.h>

