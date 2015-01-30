
#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <arch/x86/io.h>
#include "utils.h"

void flush_tbl()
{
	pte_t pgdir;
	__asm volatile ("movl %%cr3,%0\nmovl %0,%%cr3; jmp 1f; 1:\n"
		:"=r"(pgdir)
		::"memory");	
}

void flush_tbl_one(pte_t pgaddr)
{	
	__asm volatile ("invlpg %0"::"m"(*(char*)pgaddr)); 
}

void enable_paging(void)
{
   unsigned long cr0, cr4;

  // __asm  volatile ("movl %%cr0, %0" : "=r" (cr0));
  __asm  volatile ("movl %%cr4, %0" : "=r" (cr4));


   /* set most significant bit of CR0 */
  //cr0 |= 0x80000000;
  cr4|= 0x10;
  //cr0+=0x9FFFFFFF;

  __asm  volatile ("movl %%eax, %%cr3; jmp 1f; 1:"::"a" (KERN_PG_DIR_ADDR));
  //__asm volatile("movl %%eax, %%cr4"::"a" (cr4));
  //__asm("movl %%eax, %%cr0"::"a" (cr0));

  __asm ("mov %cr0,%eax;\n"
      //   "orl $0xc0010000,%eax;\n"
         "orl $0x80000000,%eax;\n"
		"and $0x9FFFFFFF,%eax;\n"
         "mov %eax,%cr0;\n"
		 "jmp 1f; "
		 "1:"
		 );

}


int page_mapped(pte_t *pdir,void *vaddr)
{
	//pte_t *pdir=(pte_t *)get_cr3();
	pte_t *ptab = pt_tabbase2(pdir,(pte_t)vaddr);


  if ((pdir[PDE_IDX(vaddr)] & PAGE_PRESENT) == 0) return 0;
  if ((ptab[PTAB2IDX(vaddr)] & PAGE_PRESENT) == 0) return 0;
  return 1;
}


pte_t virt2phys(void *vaddr)
{
	pte_t *ptab =  pt_tabbase2(get_cr3(), (pte_t)vaddr);
  return ((ptab[PTAB2IDX(vaddr)] & PT_PFNMASK) + PGOFF(vaddr));
}

pte_t page_vtophys(pte_t page_table)
{
	pte_t page = *pt_tabbase2(get_cr3(),page_table);

    page = peek32(page);
	page &= PT_PFNMASK;

	page = page + ( ((page_table>>10)) & 0xffc );

	page = peek32(page);

	page &= PT_PFNMASK;

	page |= (page_table & 0xfff);
	return page;
}




