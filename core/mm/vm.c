#include <jicama/system.h>
#include <jicama/process.h>

#include <jicama/spin.h>
#include <string.h>


//#define PAGE_SIZE 4096
#define PAGE_ALIGN(x) (((x) + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1))
#define ALIGN(x)	(((x) + (sizeof(size_t) - 1)) & ~(sizeof(size_t) - 1))

// heap stuff
// ripped mostly from nujeffos




static struct heap_bin kbins[] = {
	{16, PAGE_SIZE, 0, 0, 0, 0, 0},
	{32, PAGE_SIZE, 0, 0, 0, 0, 0},
	{64, PAGE_SIZE, 0, 0, 0, 0, 0},
	{128, PAGE_SIZE, 0, 0, 0, 0, 0},
	{256, PAGE_SIZE, 0, 0, 0, 0, 0},
	{512, PAGE_SIZE, 0, 0, 0, 0, 0},
	{1024, PAGE_SIZE, 0, 0, 0, 0, 0},
	{2048, PAGE_SIZE, 0, 0, 0, 0, 0},
	{PAGE_SIZE, PAGE_SIZE, 0, 0, 0, 0, 0},
	{0x1000, 0x1000, 0, 0, 0, 0, 0},
	{0x2000, 0x2000, 0, 0, 0, 0, 0},
	{0x3000, 0x3000, 0, 0, 0, 0, 0},
	{0x4000, 0x4000, 0, 0, 0, 0, 0},
	{0x5000, 0x5000, 0, 0, 0, 0, 0},
	{0x6000, 0x6000, 0, 0, 0, 0, 0},
	{0x7000, 0x7000, 0, 0, 0, 0, 0},
	{0x8000, 0x8000, 0, 0, 0, 0, 0},
	{0x9000, 0x9000, 0, 0, 0, 0, 0},
	{0xa000, 0xa000, 0, 0, 0, 0, 0},
	{0xb000, 0xb000, 0, 0, 0, 0, 0},
	{0xc000, 0xc000, 0, 0, 0, 0, 0},
	{0xd000, 0xd000, 0, 0, 0, 0, 0},
	{0xe000, 0xe000, 0, 0, 0, 0, 0},
	{0xf000, 0xf000, 0, 0, 0, 0, 0},
	{0x10000, 0x10000, 0, 0, 0, 0, 0} ,// 64k
	{0x100000, 0x100000, 0, 0, 0, 0, 0}, // 1m
	{0x200000, 0x200000, 0, 0, 0, 0, 0}, // 2m
	{0x400000, 0x400000, 0, 0, 0, 0, 0}, // 4m


};

static struct heap_bin ubins[] = {

	{128, PAGE_SIZE, 0, 0, 0, 0, 0},
	{256, PAGE_SIZE, 0, 0, 0, 0, 0},
	{512, PAGE_SIZE, 0, 0, 0, 0, 0},
	{1024, PAGE_SIZE, 0, 0, 0, 0, 0},
	{2048, PAGE_SIZE, 0, 0, 0, 0, 0},
	{PAGE_SIZE, PAGE_SIZE, 0, 0, 0, 0, 0},
	{0x1000, 0x1000, 0, 0, 0, 0, 0},
	{0x2000, 0x2000, 0, 0, 0, 0, 0},
	{0x3000, 0x3000, 0, 0, 0, 0, 0},
	{0x4000, 0x4000, 0, 0, 0, 0, 0},
	{0x5000, 0x5000, 0, 0, 0, 0, 0},
	{0x6000, 0x6000, 0, 0, 0, 0, 0},
	{0x7000, 0x7000, 0, 0, 0, 0, 0},
	{0x8000, 0x8000, 0, 0, 0, 0, 0},
	{0x9000, 0x9000, 0, 0, 0, 0, 0},
	{0xa000, 0xa000, 0, 0, 0, 0, 0},
	{0xb000, 0xb000, 0, 0, 0, 0, 0},
	{0xc000, 0xc000, 0, 0, 0, 0, 0},
	{0xd000, 0xd000, 0, 0, 0, 0, 0},
	{0xe000, 0xe000, 0, 0, 0, 0, 0},
	{0xf000, 0xf000, 0, 0, 0, 0, 0},
	{0x10000, 0x10000, 0, 0, 0, 0, 0} ,// 64k
	{0x100000, 0x100000, 0, 0, 0, 0, 0}, // 1m

#if 1
	{0x200000, 0x200000, 0, 0, 0, 0, 0}, // 2m
	{0x300000, 0x300000, 0, 0, 0, 0, 0}, // 64k
	{0x400000, 0x400000, 0, 0, 0, 0, 0}, // 64k
#endif
};


CREATE_SPINLOCK( heap_lock);

static void dump_bin(int idx, struct heap_bin *bin)
{
	unsigned int *temp;

	kprintf("%d:\tesize %d\tgrow_size %d\talloc_count %d\tfree_count %d\traw_count %d\traw_list %p\n",
		idx, bin->element_size, bin->grow_size, bin->alloc_count, bin->free_count, bin->raw_count, bin->raw_list);
	kprintf("free_list: ");
	for(temp = bin->free_list; temp != NULL; temp = (unsigned int *)*temp) {
		kprintf("%p ", temp);
	}
	kprintf("NULL\n");
}



// called from vm_init. The heap should already be mapped in at this point, we just
// do a little housekeeping to set up the data structure.
static int heap_init(struct heap_list *heap_list, unsigned long kernel_addr,
	unsigned long logical_addr,  unsigned int new_heap_size,int kernel_mode)
{
	const unsigned int page_entries = PAGE_SIZE / sizeof(struct heap_page);

	// set some global pointers
	heap_list->heap_alloc_table = (struct heap_page *)kernel_addr;
	heap_list->user_base_ptr =logical_addr;

	if (kernel_mode)
	{
		memcpy(&heap_list->bins[0], &kbins[0], sizeof(kbins));
		heap_list->BIN_COUNT = sizeof(kbins) / sizeof(struct heap_bin);
	}
	else{
		memcpy(&heap_list->bins[0], &ubins[0], sizeof(ubins));
		heap_list->BIN_COUNT = sizeof(ubins) / sizeof(struct heap_bin);
	}


	heap_list->heap_size = new_heap_size - PAGE_SIZE;  // use above line instead if logical_addr > sqr(PAGE_SIZE)/2

	heap_list->heap_base = (unsigned int)logical_addr + PAGE_ALIGN(heap_list->heap_size / page_entries);
	heap_list->heap_base_ptr = heap_list->heap_base;
	//kprintf("heap_alloc_table = %p, heap_base = 0x%x, heap_size = 0x%x logical_addr=%x\n",
	//	heap_alloc_table, heap_base, heap_size, logical_addr);

	// zero out the heap alloc table at the base of the heap
	memset((void *)heap_list->heap_alloc_table, 0,
		(heap_list->heap_size / PAGE_SIZE) * sizeof(struct heap_page));

	heap_list->mode = kernel_mode;

	return 0;
}



inline struct heap_page *get_page_cb(struct heap_list *heap_list, void* address)
{
	struct heap_page *page;
	int idx;

	if ((unsigned long)address < heap_list->heap_base 
	|| (unsigned long)address >= (heap_list->heap_base + heap_list->heap_size)){
		kprintf("get_page_cb: %p,%x - %p\n", heap_list->heap_base, heap_list->heap_size,
			address);
		return NULL;
	}

	idx = ((unsigned)address - heap_list->heap_base) / PAGE_SIZE; //fixme
	page = &heap_list->heap_alloc_table[idx];
	return page;
}

static char *raw_alloc(struct heap_list *heap_list, unsigned int size, int index)
{
	unsigned int new_heap_ptr;
	char *retval;
	struct heap_page *page;
	unsigned int addr;

	new_heap_ptr = heap_list->heap_base_ptr + PAGE_ALIGN(size);
	if(new_heap_ptr > heap_list->heap_base + heap_list->heap_size) {
		kprintf("raw_alloc() heap overgrew itself(%x-%x)!\n",size,heap_list->heap_size);
	}

	for(addr = heap_list->heap_base_ptr; addr < new_heap_ptr; addr += PAGE_SIZE) {
		page = get_page_cb(heap_list,addr);
		page->in_use = 0;
		//page->cleaning = 0;
		page->bin_index = index;
		if (index <  heap_list->BIN_COUNT && heap_list->bins[index].element_size < PAGE_SIZE)
			page->free_items = PAGE_SIZE / heap_list->bins[index].element_size;
		else
			page->free_items = 1;
	}

	retval = (char *)heap_list->heap_base_ptr;
	heap_list->heap_base_ptr = new_heap_ptr;
	return retval;
}

void *kcache_alloc(struct heap_list *heap_list, unsigned int size)
{
	void *address = NULL;
	int idx;
	unsigned int i;
	struct heap_page *page;

#if MAKE_NOIZE
	kprintf("shm_malloc: asked to allocate size %d\n", size);
#endif

	spin_lock(&heap_lock);

	for (idx = 0; idx <  heap_list->BIN_COUNT; idx++)
		if (size <= heap_list->bins[idx].element_size)
			break;

	if (idx == heap_list-> BIN_COUNT) {
		// XXX fix the raw alloc later.
		//address = raw_alloc(size, idx);
		kprintf("shm_malloc: (size %x)asked to allocate too much for now!\n",size);
		spin_unlock(&heap_lock);
		return NULL;
	}
	
	if (heap_list->bins[idx].free_list != NULL) {
		address = heap_list->bins[idx].free_list;
		heap_list->bins[idx].free_list = (void *)(*(unsigned int *)heap_list->bins[idx].free_list);
		heap_list->bins[idx].free_count--;
	} 
	else {
		if (heap_list->bins[idx].raw_count == 0) {
			heap_list->bins[idx].raw_list = raw_alloc(heap_list, heap_list->bins[idx].grow_size, idx);
			heap_list->bins[idx].raw_count = heap_list->bins[idx].grow_size / heap_list->bins[idx].element_size;
		}

		heap_list->bins[idx].raw_count--;
		address = heap_list->bins[idx].raw_list;
		heap_list->bins[idx].raw_list += heap_list->bins[idx].element_size;
	}

	heap_list->bins[idx].alloc_count++;
	page = get_page_cb(heap_list,address);//
	if(!page){
		spin_unlock(&heap_lock);
		kprintf("kcache_alloc() internal error on address %p\n", address);
		return NULL;
	}
	page[0].free_items--;
	page[0].in_use++;

#if MAKE_NOIZE
	kprintf("kmalloc0: page 0x%x: bin_index %d, free_count %d\n", page, page->bin_index, page->free_items);
#endif

	for(i = 1; i < heap_list->bins[idx].element_size / PAGE_SIZE; i++) {
		page[i].free_items--;
		page[i].in_use++;
#if MAKE_NOIZE
		kprintf("kmalloc1: page 0x%x: bin_index %d, free_items %d\n", page[i], page[i].bin_index, page[i].free_items);
#endif
	}

	spin_unlock(&heap_lock);

#if MAKE_NOIZE
	kprintf("shm_malloc: asked to allocate size %d, returning ptr = %p\n", size, address);
#endif
	return address;
}

static void *slab_malloc_specifier(struct heap_list *heap_list, void *address, unsigned int size)
{
	address = kcache_alloc(heap_list,size);
	
	return address;
}



static int slab_free(struct heap_list *heap_list, void *address, unsigned user_space)
{
	struct heap_page *page;
	struct heap_bin *bin;
	unsigned int i;
	int need_release=1;

	if (address == NULL)
		return -1;

	if ((unsigned long)address < heap_list->heap_base 
		|| (unsigned long)address >= (heap_list->heap_base + heap_list->heap_size)){
		kprintf("shm_free: asked to free invalid address %p\n", address);
		return -1;
	}

	spin_lock(&heap_lock);

#if MAKE_NOIZE
	kprintf("shm_free: asked to free at ptr = %p\n", address);
#endif

	page = get_page_cb(heap_list,address);// 

	if(page[0].bin_index >=  heap_list->BIN_COUNT){
		kprintf("shm_free: page %p: invalid bin_index %d\n", page, page->bin_index);
	}

	bin = &heap_list->bins[page[0].bin_index];

	if(bin->element_size <= PAGE_SIZE && (unsigned long)address % bin->element_size != 0){
		kprintf("shm_free: passed invalid pointer %p! Supposed to be in bin for esize 0x%x\n", address, bin->element_size);
	}

	for(i = 0; i < bin->element_size / PAGE_SIZE; i++) {
		if(page[i].bin_index != page[0].bin_index)
			kprintf("shm_free: not all pages in allocation match bin_index\n");

		if(page[i].in_use == 0)
			continue;

		if(page[i].in_use--==1){
			page[i].free_items++;
		}
		else{
			need_release = 0; //返回,不释放后面的内存
			kprintf("page in use = %d\n", page[i].in_use);
		}
	}

#ifdef PARANOID_KFREE
	// walk the free list on this bin to make sure this address doesn't exist already
	{
		unsigned int *temp;
		for(temp = bin->free_list; temp != NULL; temp = (unsigned int *)*temp) {
			if(temp == (unsigned int *)address) {
				kprintf("shm_free: address %p already exists in bin free list\n", address);
			}
		}
	}
#endif

	if (need_release == 0){
		//kprintf("page 0x%x not free\n",address);
		spin_unlock(&heap_lock);
		return 0;
	}

	*(unsigned int *)((unsigned)address+user_space) = (unsigned int)bin->free_list;
	bin->free_list = address;
	bin->alloc_count--;
	bin->free_count++;

	spin_unlock(&heap_lock);
	return 1;
}




static int slab_get_size(struct heap_list *heap_list, void *address, unsigned user_space)
{
	struct heap_page *page;
	struct heap_bin *bin;
	unsigned int i;
	int size=0;

	if (address == NULL)
		return 0;

	if ((unsigned long)address < heap_list->heap_base 
		|| (unsigned long)address >= (heap_list->heap_base + heap_list->heap_size)){
		kprintf("shm_free: asked to free invalid address %p\n", address);
		return 0;
	}

	spin_lock(&heap_lock);

	page = get_page_cb(heap_list,address);// 

	if(page[0].bin_index >=  heap_list->BIN_COUNT){
		kprintf("shm_free: page %p: invalid bin_index %d\n", page, page->bin_index);
	}

	bin = &heap_list->bins[page[0].bin_index];

	if(bin->element_size <= PAGE_SIZE && (unsigned long)address % bin->element_size != 0){
		kprintf("shm_free: passed invalid pointer %p! Supposed to be in bin for esize 0x%x\n", address, bin->element_size);
	}

	size= bin->element_size;
	spin_unlock(&heap_lock);
	return size;
}


/********************************************************/
static struct heap_list kernel_heap_list;

void *kmalloc(unsigned int size,long flag)
{
	void *addr=NULL;
	switch (flag)
	{
	case SLAB_ATOMIC:
		addr = kcache_alloc(&kernel_heap_list,size);
		break;
	case SLAB_GETPAGE:
		//addr = get_page();
		addr = kcache_alloc(&kernel_heap_list,size);
		break;
	default:
		addr = kcache_alloc(&kernel_heap_list,size);
		break;	
	}

	

	return addr;
}

int ksize( void *address)
{
	//return 0;
	return slab_get_size(&kernel_heap_list,address,0);
}

int kfree( void *address)
{
	return slab_free(&kernel_heap_list,address,0);
}

int kernel_heap_init(unsigned long kernel_addr, unsigned int new_heap_size)
{
	int err;

	memset(&kernel_heap_list, 0,sizeof(kernel_heap_list));
	err=heap_init(&kernel_heap_list, kernel_addr, kernel_addr,new_heap_size,1);


	return err;
}




/********************************************************/

void *user_malloc(proc_t *rp, unsigned int size,long flag)
{
	void *addr=NULL;

	
	addr = kcache_alloc(&rp->vm_heap_list,size);	

	return addr;
}

void *user_specifier_malloc(proc_t *rp,void *address, unsigned int size)
{
	void *addr=NULL;

	
	addr = slab_malloc_specifier(&rp->vm_heap_list,address,size);	

	return addr;
}



int user_free( proc_t *rp, void *address)
{
	return slab_free(&rp->vm_heap_list,address,proc_vir2phys(rp, (size_t)0));
}

int user_heap_init(proc_t *rp, unsigned long logical_addr, unsigned int new_heap_size)
{
	unsigned long kernel_addr;
	
	//logical_addr+=0x4000000;
	//new_heap_size-=0x4000000;
	kernel_addr = (void*)proc_vir2phys(rp, (size_t)logical_addr);

	if (!rp)
	{
		kprintf("user_heap_init error\n");
		return -1;
	}


	return heap_init(&rp->vm_heap_list, kernel_addr,logical_addr, new_heap_size,0);
}

/********************************************************/

 void mm_free(u32_t base, size_t clicks)
 {
	 kfree(base);
 }

u32_t mm_malloc(u32_t clicks)
{
	return kmalloc(clicks,0);
}


__public void *kcalloc(size_t elsize)
{
	register char *p;
	register size_t *q;
	size_t size = ALIGN(elsize);
	struct kmalloc_block * pkb;

	p = (char *)mm_malloc(size);
	if (p == NULL){
		return NULL;
	}
	q = (size_t *) (p + size);
	while ((char *) q > p) *--q = 0;
	return p;
}


int mem_dump(const char * buf, int size)
{
	int cnt=0,c;
	
	return cnt;
}

/********************************************************/
