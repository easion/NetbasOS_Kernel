#ifndef X86_MEMORY_PAGING_H
#define X86_MEMORY_PAGING_H

struct heap_page {
	unsigned long bin_index : 5;
	unsigned long free_items : 9;
	unsigned long cleaning : 2;
	unsigned long in_use : 16;
};

struct heap_bin {
	unsigned int element_size;
	unsigned int grow_size;
	unsigned int alloc_count;
	void *free_list;
	unsigned int free_count;
	char *raw_list;
	unsigned int raw_count;

};

struct heap_list
{
    struct heap_page *heap_alloc_table;
    unsigned long heap_base_ptr;
    unsigned long user_base_ptr;
    unsigned long heap_base;
    unsigned long heap_size;
	int mode;
	struct heap_bin bins[32];
	int BIN_COUNT;
};


enum{
	SLAB_NORMAL=0,
	SLAB_ATOMIC=0x1,
	SLAB_GETPAGE=0x2,
};

inline struct heap_page *get_page_cb(struct heap_list *heap_list, void* address);


///////////////////////////内存部分///////////////////////////////////////////////////////////
#define PAGE_SIZE			4096
#define PROC_KSTACK_SIZE     PAGE_SIZE

/*
** Jicama OS系统内存结构：
** 1-12M  内核使用
** 12-16M  页目录和页表地址0-1g
** 16-40M  内核可分配的内存大小（kmalloc）
** 40-128M 可分配的页面（get_page）
** 2g(0x80000000L)上  应用程序的内存空间
** 
*/

//long get_stack_top();

extern unsigned long KERN_PG_DIR_ADDR;

int check_kmem(void);
int get_pages_index(int x);
int memory_slot_init(void* start, int size);

pte_t get_pages(int counts);
inline void copy_page(pte_t *to,pte_t *from, int n);

void set_cr3(u32_t cr3);
void set_cr4(u32_t cr4);


//#define SLAB_MALLOC_SIZE         0x1000000////


#define USER_SPACE_ADDR 0x80000000


#define USER_SPACE_SIZE 0x60000000 //1.5g
#define USER_STACK_ADDR_END (0x60000000-0x1000) //


#define USER_STACK_ADDR (USER_SPACE_SIZE - 0x800000) //
#define USER_MMAP_SIZE (USER_STACK_ADDR - USER_MMAP_ADDR) //
#define USER_MMAP_ADDR 0x40000000 //



#endif
