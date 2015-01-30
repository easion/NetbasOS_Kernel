
// ------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//-------------------------------------------------------------------------------------
#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <arch/x86/io.h>
#include <string.h>
#include <assert.h>
#include "utils.h"
u32_t __HIGH_MEM,__LOW_MEM;
#if 1
#define MAP_NR(addr) (((addr)-__LOW_MEM)>>12)
int PHI_BLOCKS;
__local int		hole_head_pos		= 0x00;
__local int		hole_tail_pos	,hole_max	;
//__local struct pgmaps_struct phy_usable_map[PHI_BLOCKS];
__local struct pgmaps_struct *phy_usable_map;



static inline int CHECK_PHY_PAGE(u32_t page)
{
	if((page < __LOW_MEM) || (page >= __HIGH_MEM)){
		return -1;
	}
	return 0;
}

int memory_slot_init(void* start, int size)
{
	int i;
	//int size = PHI_BLOCKS*sizeof(struct pgmaps_struct);
	phy_usable_map = start;
	PHI_BLOCKS = size;	

	hole_max = hole_tail_pos		= PHI_BLOCKS-1;
	ASSERT(phy_usable_map);

	phy_usable_map[0].prev_hole = HOLE_NULL;
	phy_usable_map[0].next_hole = 1;
	phy_usable_map[0].count = 0;
	for (i=1;i<PHI_BLOCKS;i++)
	{
		phy_usable_map[i].count = 0; ///////使用数
		phy_usable_map[i].prev_hole = i-1;
		phy_usable_map[i].next_hole = i+1;
	}
	phy_usable_map[hole_tail_pos].next_hole = HOLE_NULL;

	return size;
}

int get_pages_index(int x)
{
	int i,cnt=0,start=0;

	for (i=1;i<PHI_BLOCKS;i++)
	{
		if (phy_usable_map[i].count)
		{
			start=0;
			cnt=0;
			continue;
		}  

		if (phy_usable_map[i].prev_hole != i-1)
		{
			start=0;
			cnt=0;
			continue;
		}

		if(phy_usable_map[i].next_hole != i+1){
			start=0;
			cnt=0;
			continue;
		}

		if(!start)start = i;

		cnt ++;

		if (cnt==x)
		{
			break;
		}
	}

	if (cnt!=x)
	{
		return 0;
	}

	return start;
}

int dump_pages(char *buf, int len)
{
	int cnt=0,c;
	int i, ifree=0,iused=0;

	for (i=1;i<PHI_BLOCKS;i++)
	{
		if(phy_usable_map[i].count == 0)
			ifree++; ///////使用数
		else
			iused++;
	}

	c = sprintf(buf+cnt, "page: start at 0x%08x num %d, free %d, used %d.\n",
		(u32_t)phy_usable_map, PHI_BLOCKS, ifree, iused);
	cnt += c;
	return cnt;
}


pte_t get_pages(int counts)
{
	unsigned flags;
	int i, s,p=0;

	if (!counts)
	{
		return NULL;
	}
	
	save_eflags(&flags);
	s = get_pages_index(counts);
	if (!s)
	{
		restore_eflags(flags);
		return NULL;
	}

	for (i=s; i<counts; i++)
	{
		//tmp = hole_head_pos; ///指向空闲头
		hole_head_pos = phy_usable_map[i].next_hole; /////空闲头指向下面一个位置
		phy_usable_map[i].prev_hole = HOLE_NULL; /////使最后一个为空
		phy_usable_map[i].count = 1;    ////标志已使用
	}

	restore_eflags(flags);

	return (PTOB( s) + __LOW_MEM);
}

pte_t get_page()
{
	pte_t tmp;
	unsigned flags;
	
	save_eflags(&flags);
	if ( hole_head_pos == hole_tail_pos ) ////如果这是最后一个可用内存
	{
		hole_head_pos = 0;  /////标志内存用尽
		phy_usable_map[hole_head_pos].count = 1; /////标志使用
		restore_eflags(flags);
		return (PTOB( hole_tail_pos ) + __LOW_MEM); //////因为最后12位为空，所以左移12位
	}
	if ( hole_head_pos == HOLE_NULL )  ////////没有内存
	{
		restore_eflags(flags);
		panic("get_page: Not enough memory!" );
		return 0;
	}

	tmp = hole_head_pos; ///指向空闲头
	hole_head_pos = phy_usable_map[tmp].next_hole; /////空闲头指向下面一个位置
	phy_usable_map[hole_head_pos].prev_hole = HOLE_NULL; /////使最后一个为空
	phy_usable_map[tmp].count = 1;    ////标志已使用
	restore_eflags(flags);
	//kprintf("get_page() succ %p!!\n",PTOB( tmp) + __LOW_MEM);

	return (PTOB( tmp) + __LOW_MEM);  ////返回页地址
}


int free_page(pte_t addr)
{
	unsigned fl;
	int idx = MAP_NR(addr);

	fl = save_eflags(NULL);

	if (CHECK_PHY_PAGE(addr)) { ////不存在的地址
		trace("trying to free nonexistent page: %x\n",addr);
		restore_eflags(fl);
		return -1;
	}

	if (idx>hole_max)
	{
		restore_eflags(fl);
		return -1;
	}

	if (phy_usable_map[idx].count == 0) {    //////本身就是空闲页
		trace("trying to free free page: %x",addr);
		restore_eflags(fl);
		return -1;
	}


	if (phy_usable_map[idx].count-- == 1) {     /////应该把1该成0？
		if (hole_head_pos == HOLE_NULL)     ////头为空
		{
			hole_head_pos = hole_tail_pos = idx;
			phy_usable_map[hole_head_pos].next_hole = HOLE_NULL;
			phy_usable_map[hole_head_pos].prev_hole = HOLE_NULL;
		}
		else {    ////////插入尾部
			phy_usable_map[hole_tail_pos].next_hole = idx;
			phy_usable_map[idx].prev_hole = hole_tail_pos;
			phy_usable_map[idx].next_hole = HOLE_NULL;
			hole_tail_pos = idx;
		}
	}
	restore_eflags(fl);
	return 0;
}
#else


#endif

void free_pages(void *ptr, int counts)
{
	int i;

	for (i=0; i<counts; i++)
	{
		free_page(ptr+PAGE_SIZE*i);
	}
}


static bool is_user_mem(u32_t vaddr)
{
	if (vaddr>USER_SPACE_ADDR)
	{
		return true;
	}
	return false;
}


static int put_one_page(pte_t *cr3, pte_t page,pte_t vaddr)
{
	pte_t *pgtab;
	pte_t tmp_pg;
	int count;

	//kprintf("put_one_page Trying to put  page %x at %x\n", page, vaddr); 

	if (CHECK_PHY_PAGE(page)){
		kprintf("put_one_page Trying to put incorrect page %x at %x\n", page, vaddr); 
		/////////页地址错误
		return -1;
	}

	count = phy_usable_map[PTAB2IDX(page-__LOW_MEM)].count;
	if (count != 1){
		kprintf("put_one_page phy_usable_map disagrees with %x,%d %d,%x\n",page,count,PTAB2IDX(page-__LOW_MEM),__LOW_MEM);
		return -2;
	}

	pgtab = pt_tabbase2(cr3,vaddr);
	//kprintf("pgtab 00= %p,%p\n",pgtab,  cr3[PDE_IDX(vaddr)]);

	if ((*pgtab)&PAGE_PRESENT) {
		pgtab = (pte_t *) (PT_PFNMASK & *pgtab);
		//kprintf("pgtab 11= %p\n",pgtab );
	}
	else { 
		if(is_user_mem(vaddr)){		
			pgtab = (pte_t*)fix_pagedir_not_present(cr3,vaddr);
			//kprintf("pgtab 22= %p\n",pgtab );
			if(!pgtab){
				kprintf( "dir %x not exist\n", vaddr);
			}
		}
		else{
			//if page not exist
			kprintf("Page not Present: address 0x%x [pagetab 0x%x:0x%x]\n",
				vaddr,(long)pgtab, *pgtab);
			return -3; //(14<<8);
		}
	}

	tmp_pg = (pte_t )pgtab;

	poke32(( tmp_pg + (PTE_IDX(vaddr)*PTE_SIZE ) ) , ( page | PAGE_PUW ));
	flush_tbl();
	//kprintf("pgtab 33= %p\n",pgtab );
	return 0;
}

void copy_pagedir(proc_t *current_task, proc_t *new_rp,pte_t from,pte_t to)
{
	pte_t nr;
	pte_t this_page;
	pte_t * from_pgtab,	 * to_pgtab;
	pte_t * from_pgdir, * to_pgdir;

	from_pgdir = pt_tabbase2(current_task->proc_cr3,from);
	to_pgdir =pt_tabbase2(new_rp->proc_cr3,to);

	if (!(PAGE_PRESENT & *from_pgdir)){
		//kprintf("%s():from_pgdir:0x%x not exist %x\n",
		//	__FUNCTION__, from,  *from_pgdir);
		return;
	}
	if ( !(PAGE_PRESENT& *to_pgdir) ){	
		to_pgdir = fix_pagedir_not_present(new_rp->proc_cr3,to);
		kprintf("%s(): to_pgdir:0x%x not exist %x\n",
			__FUNCTION__,
			to, *to_pgdir);
	}
	from_pgtab = (pte_t *)(PT_PFNMASK & (*from_pgdir));
	to_pgtab = (pte_t *)(PT_PFNMASK & (*to_pgdir));	//page_dir

	for ( nr = PTES_PER_PAGE; nr-- > 0 ; from_pgtab++,to_pgtab++)
	{
		u32_t new_page;
		this_page = *from_pgtab;
		if (!(PAGE_PRESENT & this_page)) {
			continue;
		}

		this_page &= 0xffffff00;
		this_page |=  (PAGE_ACCESSED |  PAGE_USER |  PAGE_PRESENT); //page read only

		*to_pgtab = this_page;
		*from_pgtab = this_page;
		phy_usable_map[MAP_NR(this_page)].count++;
	}
}


inline void copy_page(pte_t *to,pte_t *from, int n)
{
	int i;
	for(i=0;i<n;i++)
       to[i] = from[i];
}

__local int un_swap_page(int err_type, pte_t * table_entry)
{
	pte_t oldpg,newpg;

	ASSERT(table_entry);

	oldpg = PT_PFNMASK & (*table_entry);
	if (oldpg >= __LOW_MEM && phy_usable_map[MAP_NR(oldpg)].count==1) {
		*table_entry |= PAGE_WRITABLE;
		if(*table_entry&PAGE_ACCESSED)
			*table_entry &= ~PAGE_ACCESSED;
		//kprintf("un_swap_page make write %d is %x\n", err_type, *table_entry);
		return 0;
	}
	newpg=get_page();

	if (oldpg >= __LOW_MEM)
		phy_usable_map[MAP_NR(oldpg)].count--;

	poke32(table_entry, newpg | PAGE_PUW);

	/*here is not debug*/
	copy_page((pte_t *)newpg,(pte_t *)oldpg,PTES_PER_PAGE);
	return 0;
}


__local int write_verify(pte_t vaddr)
{
	pte_t page;
	pte_t tmp;

	page = *pt_tabbase2 (get_cr3(),vaddr) ;

	if (!(page&PAGE_PRESENT))
	{
		return -1;
	}
	page &= PT_PFNMASK;
	page += ((vaddr>>10) & 0xffc);	//page_dir

	if ((3 & *(pte_t *) page) == PAGE_PRESENT)  /* non-writeable, present */
	{
		un_swap_page(0, (pte_t *) page);
	}

	return 0;
}

int mem_writeable(void* vaddr,int size)
{
	pte_t start;

	start = (pte_t) vaddr;
	size += start & 0xfff;
	start &= PT_PFNMASK;

	while (size>0) {
		size -= PAGE_SIZE;
		if(write_verify(start))
			return -1;
		start += PAGE_SIZE;
	}
	return 0;
}


__public int do_swap_page(pte_t error_code,pte_t address)
{
	int error;
	u32_t base = (PT_PFNMASK & peek32( (pte_t)pt_tabbase2(get_cr3(),address)) );
	u32_t offset = ( (address>>10) & 0xffc);
	pte_t * ptab;

	trace("do_swap_page at %d\n",error_code);

	ptab = (pte_t *) (offset + base);
	error = un_swap_page(error_code, ptab);
	flush_tbl();
	return error;
}

__public int do_no_page(pte_t error_code,pte_t vaddr)
{
	pte_t tmp;

	tmp = get_page();
	//kprintf("page fault on %p\n",vaddr );
	return put_one_page(get_cr3(),tmp,vaddr);
}

