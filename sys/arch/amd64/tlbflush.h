#ifndef _X8664_TLBFLUSH_H
#define _X8664_TLBFLUSH_H

#define __flush_tlb()							\
	do {								\
		unsigned long tmpreg;					\
									\
		__asm__ __volatile__(					\
			"movq %%cr3, %0;  # flush TLB \n"		\
			"movq %0, %%cr3;              \n"		\
			: "=r" (tmpreg)					\
			:: "memory");					\
	} while (0)

/*
 * Global pages have to be flushed a bit differently. Not a real
 * performance problem because this does not happen often.
 */
#define __flush_tlb_global()						\
	do {								\
		unsigned long tmpreg;					\
									\
		__asm__ __volatile__(					\
			"movq %1, %%cr4;  # turn off PGE     \n"	\
			"movq %%cr3, %0;  # flush TLB        \n"	\
			"movq %0, %%cr3;                     \n"	\
			"movq %2, %%cr4;  # turn PGE back on \n"	\
			: "=&r" (tmpreg)				\
			: "r" (mmu_cr4_features & ~X86_CR4_PGE),	\
			  "r" (mmu_cr4_features)			\
			: "memory");					\
	} while (0)

extern unsigned long pgkern_mask;

#define __flush_tlb_all() __flush_tlb_global()

#define __flush_tlb_one(addr) \
	__asm__ __volatile__("invlpg %0": :"m" (*(char *) addr))


/*
 * TLB flushing:
 *
 *  - flush_tlb() flushes the current mm struct TLBs
 *  - flush_tlb_all() flushes all processes TLBs
 *  - flush_tlb_mm(mm) flushes the specified mm context TLB's
 *  - flush_tlb_page(vma, vmaddr) flushes one page
 *  - flush_tlb_range(vma, start, end) flushes a range of pages
 *  - flush_tlb_kernel_range(start, end) flushes a range of kernel pages
 *  - flush_tlb_pgtables(mm, start, end) flushes a range of page tables
 *
 * ..but the x86_64 has somewhat limited tlb flushing capabilities,
 * and page-granular flushes are available only on i486 and up.
 */



#endif /* _X8664_TLBFLUSH_H */
