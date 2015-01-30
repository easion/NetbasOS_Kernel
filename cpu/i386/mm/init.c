
#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <assert.h>
#include "utils.h"

#define PG_TABLE_ADDR    (KERN_PG_DIR_ADDR+PTES_PER_PAGE*PTE_SIZE) /////////页表地址0-1g

__asmlink void page_dir_init(void);
__asmlink void page_table_init(void);
void flush_tbl();
void enable_paging();
extern u32_t __HIGH_MEM,__LOW_MEM;


void memory_init(u32_t mem_size)
{
	u32_t  num_blks ;
	u32_t pages_start=0,mm_pool=0 ;
	extern unsigned long __LOW_MEM;
	int SLAB_MALLOC_SIZE=check_kmem();

	__HIGH_MEM = mem_size;
	num_blks = (mem_size)>>PT_PFNSHIFT;
	pages_start = (KERN_PG_DIR_ADDR+PAGE_SIZE+num_blks*sizeof(u32_t)+4095)&PT_PFNMASK;


	mm_pool =(pages_start + (num_blks*sizeof(struct pgmaps_struct)+4095))&PT_PFNMASK;
	if (mem_size<SLAB_MALLOC_SIZE+mm_pool)
	{
		panic("memory_init():no memory to run netbas os");
	}


#ifdef __STATIC_SLAB__

	__LOW_MEM = (mm_pool+SLAB_MALLOC_SIZE+4095)&PT_PFNMASK;
	num_blks -= SLAB_MALLOC_SIZE>>12;
#else
	__LOW_MEM = mm_pool;

#endif

	page_dir_init();
	page_table_init();
	//page_dir_high(mm_pool);
	memory_slot_init((void*)pages_start, num_blks);

	enable_paging(); //必须在内存初始后进行

#ifndef __STATIC_SLAB__
	mm_pool = get_pages((SLAB_MALLOC_SIZE+PAGE_SIZE-1)/PAGE_SIZE);
#endif

	ASSERT(mm_pool);

	kernel_heap_init(mm_pool, SLAB_MALLOC_SIZE);
}


#define PAGING_MEMORY (15*1024*1024)

#define MAX_MEMORY_USEABLE  (256*1024*1024)  //256M
#define MEM_4M_SIZE  (PAGE_SIZE * 1024)



void page_dir_init(void)
{
	int i,j;
    pte_t *pgdir = (pte_t*)KERN_PG_DIR_ADDR;

	pgdir[ 0 ]=PG_TABLE_ADDR|PAGE_PUW;

		//初始化目录表
   	for (i=1;i<256;i++)          //初始化前面1G目录,用于内核心
	  pgdir[ i ]  = (PG_TABLE_ADDR+i*PAGE_SIZE) |  PAGE_PUW;

   	for (i=256;i<PTES_PER_PAGE;i++)
		pgdir[ i ]=0;
}

void page_dir_high(unsigned addr)
{
	int i,j;
    pte_t *pgdir = (pte_t*)KERN_PG_DIR_ADDR;
	pte_t *pgtab = (pte_t *)addr;
	int SLAB_MALLOC_SIZE=check_kmem();

	pgdir[ 0 ]=PG_TABLE_ADDR|PAGE_PUW;

	
   	for (i=768,j=0;i<PTES_PER_PAGE;i++,j++)
		pgdir[ i ]=(addr+j*PAGE_SIZE) |  PAGE_PUW;

	for (i=0; i<SLAB_MALLOC_SIZE/4096; i++)
	{
		pgtab[i] = 0xc0000000+ (i * PAGE_SIZE) | PAGE_PRESENT|PAGE_WRITABLE;
	}
}


void	page_table_init()
{
	int i;
	int count = (__HIGH_MEM>>PT_PFNSHIFT)-1;
    pte_t *pgtab = (pte_t *)PG_TABLE_ADDR;
	
	for (i=0;i<256;i++)	//用于 8086 程序，最下面的1M
         pgtab[i] = (i * PAGE_SIZE) | PAGE_PUW;

	for (i=256;i<count;i++)	//only kernel write able,by mm_alloc used
         pgtab[i] = (i * PAGE_SIZE) | PAGE_PRESENT|PAGE_WRITABLE;

//	flush_tbl();
}

int paging_pgs(pte_t useable)
{
	int pdirs, pgs;
	pte_t total, start = 0;

   if (useable > MAX_MEMORY_USEABLE) 
	   total = MAX_MEMORY_USEABLE;
   else 
	   total = useable;


   pgs = (total >> 12);
   pdirs =  (pgs>>10) + (((pgs%PTES_PER_PAGE) > 0) ? 1 : 0);

	kprintf("page table entries: %d \n", pgs);
   kprintf("page dir entries: %d \n", pdirs);
   return pgs;
}

int proc_mem_count(proc_t *rp)
{
	ASSERT(__HIGH_MEM);
	ASSERT(rp);
	u32_t endmem = rp->p_brk_code;
	endmem += 4096;
	//endmem = 0x1000000;
	return (int)(endmem*100/__HIGH_MEM);
}
