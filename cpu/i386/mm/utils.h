

#ifndef _mm_utils_H_
#define _mm_utils_H_

#define PAGE_PRESENT   0x001
#define PAGE_WRITABLE  0x002
#define	PAGE_USER      0x004
#define PAGE_PCD  0x010
#define PAGE_ACCESSED  0x020
#define PAGE_DIRTY     0x040
#define PAGE_GUARD     0x200
#define PAGE_PUW			(PAGE_PRESENT|PAGE_WRITABLE|PAGE_USER)

#define PT_PFNMASK   0xfffff000
#define PT_LIMIT1M   0x0fffff+1
#define PT_LIMIT4M   0x3fffff
#define PT_PFNSHIFT  12
#define PTE_SIZE  sizeof(pte_t)

#define PTE_IDX(__addr) ( (pte_t) ( (__addr>>PT_PFNSHIFT) & 1023) )
#define PDE_IDX(__addr)  (((unsigned long) __addr) >> 22)
#define PGOFF(vaddr)   (((unsigned long) vaddr) & 4095) 

#define PAGES(x) (((unsigned long)(x) + (PAGE_SIZE - 1)) >> 12)
#define PAGEADDR(x) ((unsigned long)(x) & ~(PAGE_SIZE - 1))

#define PTOB(x) ((unsigned long)(x) << PT_PFNSHIFT)
#define PTAB2IDX(vaddr) (((unsigned long) vaddr) >> PT_PFNSHIFT)

#define PTES_PER_PAGE  (PAGE_SIZE / PTE_SIZE)

//#define pt_tabbase(__addr)  (pte_t*)( get_cr3()+((PDE_IDX(__addr))*PTE_SIZE) )
#define pt_tabbase2(cr3,__addr)  ((pte_t*) (cr3)+PDE_IDX(__addr) )


//mm所需要的常量定义

#define		HOLE_NULL 	-1     /////空页表

extern int PHI_BLOCKS;

struct pgmaps_struct
{
	int		count;		//num of users
	int		prev_hole;		//hole:unassigned phisical memory block
	int		next_hole;   //////next
};



#endif


