#ifndef MEMORY_PAGING_H
#define MEMORY_PAGING_H

#ifdef __IA32__
#include <arch/x86/paging.h>
#elif defined(__ARM__)  //arm9
#include <arch/arm/page.h>
#else
#error bad platform
#endif


void proc_copy_pages(unsigned long from,unsigned long to,long size);

int mem_writeable(void* address,int size);
void flush_tbl(void);

int map_share_page(u32_t form_addr,u32_t to_addr,int pages);
int attach_share_page(u32_t form_addr,u32_t to_addr,int pages);
void memory_init(u32_t);
int kernel_heap_init(u32_t start, unsigned int len);
int	memory_map_init(void *mem,int);
int unmap_user_pages(u32_t form_addr, int pages);

pte_t get_page(void);
void clts(void);

pte_t * fix_pagedir_not_present(pte_t *pdir,pte_t vaddr);
int map_high_memory(pte_t vaddr, int size,const char*);
int map_physical_memory_nocache(pte_t vaddr, int size,const char *name);
pte_t page_vtophys(pte_t page_table);
pte_t virt2phys(void *vaddr);


#define PAGESIZE 4096
#define PAGEBITS 12
#define PAGE0011 (PAGESIZE - 1)
#define PAGE1100 (~PAGE0011)

/* round page up/down */
#define cast(type, value) ((type)(value))

#define pageup(addr) \
cast(typeof(addr), (cast(u32_t, addr) + PAGESIZE - 1) & PAGE1100)

#define pagedown(addr) cast(typeof(addr), cast(u32_t, addr) & PAGE1100)
#define pagemul(addr) cast(typeof(addr), cast(u32_t, addr) << PAGEBITS)
#define pagediv(addr) cast(typeof(addr), cast(u32_t, addr) >> PAGEBITS)
#define pagemod(addr) cast(typeof(addr), cast(u32_t, addr) & PAGE0011) 

#endif
