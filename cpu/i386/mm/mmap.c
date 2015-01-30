
// ------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//-------------------------------------------------------------------------------------

#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <arch/x86/io.h>
#include <assert.h>
#include "utils.h"



pte_t * fix_pagedir_not_present(pte_t *pdir, pte_t vaddr)
{
	int i;
	pte_t *pgtab;
	u32_t mypage ;

	mypage = pdir[PDE_IDX(vaddr)];
	if(mypage){
		kprintf("fix_pagedir_not_present found = %p,%p,%p\n",mypage,pdir, get_cr3());
		return (pte_t *)mypage;
	}

	mypage = get_page();
	if (!mypage)
	{
		return NULL;
	}
	pdir[PDE_IDX(vaddr)] = mypage|PAGE_PUW;
	pgtab = (pte_t*)mypage;
	for (i=0; i<PTES_PER_PAGE; i++)
	{
		pgtab[i] = 0L ;
	}
	flush_tbl();
	return pgtab;
}

pte_t * map_high_memory_internal(pte_t *pgdir,pte_t vaddr, int size)
{
	int i;
	pte_t *pgtab;
	int nsize=(size+PAGE_SIZE-1)/PAGE_SIZE; //
	pte_t tmp_pg;
	
	pgtab = fix_pagedir_not_present(pgdir,vaddr);

	if (!pgtab)
	{
		kprintf("map_physical_memory_nocache:fix_pagedir_not_present error\n");
		return NULL;
	}

	vaddr = vaddr & PT_PFNMASK;
	tmp_pg = (pte_t )pgtab;

	for (i=0; i<nsize; i++)
	{
		//put_one_page(vaddr,vaddr);
		//pgtab[i] = vaddr + i*PAGE_SIZE | PAGE_PUW|PAGE_PCD;
		poke32(( tmp_pg + (PTE_IDX(vaddr)*PTE_SIZE ) ) , ( vaddr | PAGE_PUW ));
		vaddr += PAGE_SIZE;
	}
	
	return pgtab;
}

int map_high_memory(pte_t vaddr, int size,const char *name)
{
	pte_t *pgdir;
	pte_t *pgtab;

	pgdir = get_cr3();
	pgtab = map_high_memory_internal(KERN_PG_DIR_ADDR,vaddr,size);
	if (!pgtab)
	{
		return -1;
	}

	pgdir[PDE_IDX(vaddr)] = (pte_t)pgtab | PAGE_PUW;
	//kprintf("map_high_memory pos on %d\n",PDE_IDX(vaddr));
	flush_tbl();
	return 0;
}

int map_physical_memory_nocache(pte_t vaddr, int size,const char *name)
{
	int i;
	pte_t *pgtab;
	int nsize=(size+PAGE_SIZE-1)/PAGE_SIZE; //
	pte_t tmp_pg;
	
	pgtab = fix_pagedir_not_present(get_cr3(),vaddr);

	if (!pgtab)
	{
		kprintf("map_physical_memory_nocache:fix_pagedir_not_present error\n");
		return -1;
	}

	vaddr = vaddr & PT_PFNMASK;
	tmp_pg = (pte_t )pgtab;

	for (i=0; i<nsize; i++)
	{
		//put_one_page(vaddr,vaddr);
		//pgtab[i] = vaddr + i*PAGE_SIZE | PAGE_PUW|PAGE_PCD;
		poke32(( tmp_pg + (PTE_IDX(vaddr)*PTE_SIZE ) ) , ( vaddr | PAGE_PUW|PAGE_PCD ));
		vaddr += PAGE_SIZE;
	}
	flush_tbl();
	return 0;
}

static pte_t * remove_pagedir(pte_t *cr3, pte_t vaddr, bool removedir)
{
	int i;
	pte_t *pgtab;
	pte_t *pgdir;

	pgdir =  (pt_tabbase2(cr3, vaddr)) ;
	//ASSERT(pgtab);

	if (!(PAGE_PRESENT & *pgdir)){  //? if page dir is null
		//kprintf("remove_pagedir(): null page table in pagedir:%x\n",		(pte_t)vaddr);
		return NULL;
	}

	pgtab = (pte_t *)(PT_PFNMASK & *pgdir);	//page_dir
	for (i=0; i<PTES_PER_PAGE; i++)
	{
		if(pgtab[i] & PAGE_PRESENT){
			free_page(pgtab[i] & PT_PFNMASK);//
			//kprintf(__FUNCTION__" free page 0x%x\n", pgtab[i] );
		}
		pgtab[i] = 0;
	}

	if(removedir)
	{
		free_page((u32_t)pgtab);
		cr3[PDE_IDX(vaddr)] = 0;
	}
	return pgtab;
}

void map_proc(proc_t *rp)
{
	u32_t i;
	u32_t base = proc_phy_addr(rp);

	ASSERT(rp->proc_cr3);	

	for (i=0; i<USER_SPACE_SIZE; i+=0x400000)
	{
		//kprintf("fix at %x\n", base+i);
		fix_pagedir_not_present(rp->proc_cr3, base+i);
	}
}
//此函数拷贝多个页表，入口地址必须在4M边界
//void proc_copy_pages(pte_t from,pte_t to,long size)
int copy_proc_code( proc_t *current_task, proc_t *new_rp)
{
	pte_t nr;
	pte_t this_page;
	pte_t * from_pgtab,	 * to_pgtab;
	pte_t * from_pgdir, * to_pgdir;
	long size;
	pte_t from, to;

	from = proc_phy_addr(current_task);
	to = proc_phy_addr(new_rp);

	if ((from&PT_LIMIT4M) || (to&PT_LIMIT4M))
		panic("proc_copy_pages called with wrong alignment -(%x-%x)",from,to);

	size = PDE_IDX((unsigned) (USER_SPACE_SIZE+PT_LIMIT4M));

	for( ; size-->0 ; from += 0x400000,to+=(0x400000))
	{
		copy_pagedir(current_task,new_rp,from,to);
	}

	flush_tbl();
	return OK;
}

void unmap_proc(proc_t *rp, bool removedir)
{
	u32_t i;
	u32_t base = proc_phy_addr(rp);	

	for (i=0; i<USER_SPACE_SIZE; i+=0x400000)
	{
		remove_pagedir(rp->proc_cr3,base+i, removedir);
	}

	if (removedir)
	{
	free_page(rp->proc_cr3);
	rp->proc_cr3 = NULL;
	//kprintf("unmap_proc called2\n");
	}

	flush_tbl();
}


//映射一个4M大小的页目录
static int map_page_dir(pte_t *pgdir,u32_t form_addr, int pages)
{
	int i;	
	u32_t *page=(unsigned long *)get_page();

	if (!page)
	{
		kprintf("%s() error1 0x%x\n",__FUNCTION__, page);
		return -1;
	}

	form_addr &= PT_PFNMASK;  
	if (pgdir[form_addr>>22] != 0)
	{
		kprintf("%s() error2 0x%x\n", __FUNCTION__, pgdir[form_addr>>22]);
	   return -1;
	}

	for (i=0; i<pages; i++)
	{
		page[i] = (form_addr+(i*PAGE_SIZE)|PAGE_PUW);
	}

	pgdir[PDE_IDX(form_addr)] = (pte_t)page | PAGE_PUW;
	flush_tbl();
	return 0;
}

//解除映射一个4M大小的页目录,在1G之外
int unmap_large_dir(u32_t form_addr)
{
	int i;	
	pte_t *pgdir = (pte_t*)get_cr3();
	pte_t page=(pte_t)pgdir[PDE_IDX(form_addr)]&PT_PFNMASK;    

	if (!(page&PAGE_PRESENT) )
	{
		return -1;
	}

	free_page(page);    

	pgdir[PDE_IDX(form_addr)] = 0;
	flush_tbl();
}

//映射一个4M大小的页目录,form_addr一般为vesa的io地址,to_addr为用户的空间地址
int map_to(u32_t form_addr, u32_t to_addr, int pages)
{
	int error;	
	pte_t *pgdir = (pte_t*)get_cr3();
	pte_t *pagedest;
	pte_t *pagesrc, *dir;

	form_addr=form_addr&PT_PFNMASK;
	to_addr=to_addr&PT_PFNMASK;

	dir = pt_tabbase2(pgdir,form_addr);

	if (!(PAGE_PRESENT & *dir))  //
	{
		error = map_page_dir(pgdir,form_addr,PTES_PER_PAGE);
		if (error)	{
			return error;
		}
	}

	pagedest = fix_pagedir_not_present(pgdir,to_addr);
	if (!pagedest)
	{
		return -1;
	}

	error=attach_share_page(form_addr,to_addr,pages);
	return error;
}

static inline void map_pages(pte_t *to,int count)
{
	int i;
	for(i=0;i<count;i++){
       to[i] = get_page();
	}
}

int map_share_page(u32_t form_addr,u32_t to_addr,int pages)
{
	int error=0;	
	pte_t *pgdir = (pte_t*)get_cr3();
	pte_t *pagedest;
	pte_t *pagesrc, *dir;

	if (form_addr)
	{
		return map_to(form_addr,to_addr,pages);
	}
	else{
		
	}
	flush_tbl();
	return error;
}


int attach_share_page(u32_t form_addr,u32_t to_addr,int pages)
{
	int error=0;	
	pte_t *pgdir = (pte_t*)get_cr3();
	pte_t *pagedest;
	pte_t *pagesrc, *dir;
	int left=0;

	while (left<pages)
	{
		int cnt=MIN(pages,1024-PTE_IDX(form_addr) );

	//form_addr=form_addr&PT_PFNMASK;
	//to_addr=to_addr&PT_PFNMASK;

	dir = pt_tabbase2(pgdir,form_addr);

	if (!(PAGE_PRESENT & *dir))  //
	{
		return -1;
	}	

	pagedest = (pte_t*)(pgdir[PDE_IDX(to_addr)] & PT_PFNMASK);  //user
	pagesrc = (pte_t*)(pgdir[PDE_IDX(form_addr)] & PT_PFNMASK); //high mem

	pagedest+=PTE_IDX(to_addr);
	pagesrc+=PTE_IDX(form_addr);

	copy_page(pagedest,  pagesrc,cnt); 
	left += cnt;

	to_addr+=cnt*PAGE_SIZE;
	form_addr+=cnt*PAGE_SIZE;
	}

	flush_tbl();
	return error;
}


//解除映射一个4M大小的页目录,在1G之内
int unmap_user_pages(u32_t form_addr, int pages)
{
	int i;	
	pte_t *pgdir = (pte_t*)get_cr3();
	pte_t *page=(pte_t)pgdir[PDE_IDX(form_addr)]&PT_PFNMASK;    

	form_addr &= PT_PFNMASK;

   if (pgdir[PDE_IDX(form_addr)] == 0 )
   {
	   return -1;
   }

	for (i=0; i<pages; i++)
	{
		if(page[i]){
			free_page(page[i]);
		}
		page[i]=0;
		//copy page
		//page[i] = (form_addr+(i*4096)|PAGE_PUW);
	}

	flush_tbl();
}

