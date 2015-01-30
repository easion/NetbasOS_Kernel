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


///////////////////////////�ڴ沿��///////////////////////////////////////////////////////////
#define PAGE_SIZE			4096
#define PROC_KSTACK_SIZE     PAGE_SIZE

/*
** Jicama OSϵͳ�ڴ�ṹ��
** 1-12M  �ں�ʹ��
** 12-16M  ҳĿ¼��ҳ���ַ0-1g
** 16-40M  �ں˿ɷ�����ڴ��С��kmalloc��
** 40-128M �ɷ����ҳ�棨get_page��
** 2g(0x80000000L)��  Ӧ�ó�����ڴ�ռ�
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
