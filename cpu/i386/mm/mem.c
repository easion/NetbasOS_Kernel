
#include <jicama/system.h>
#include <jicama/grub.h>

#define LOW_BLKS 50

struct dos_mem{
	char 	 used;              
	void * addr;
	u16_t 	 size;                     
} ;
static struct dos_mem mem_table[LOW_BLKS];

 void low_mem_init(grub_info_t* mbi)
{
    register int i;
  
	mem_table[0].addr = (void *)0x4000;

	mem_table[0].size = (int)(0x100000-0x4000); //mbi->mem_upbase;
    mem_table[0].used = TRUE;

     for (i = 1; i < LOW_BLKS; i++)
			mem_table[i].used = FALSE;
 }

 void * low_alloc(u16_t len)
{
    void * p = 0;
    int i = 0;

    while (i < LOW_BLKS && p == NULL) {
	if (mem_table[i].used && (mem_table[i].size >= len))
		p = mem_table[i].addr;
	else i++;
    }
    if (p != 0) {
	if (mem_table[i].size > len) {
	    mem_table[i].size -= len;
	    mem_table[i].addr += len;
	}
	else mem_table[i].used = FALSE;
    }
    return(p);
}

 int low_free(void* p,u32_t len)
{
    register int i = 1;
    int i1 = 0;
	int i2 = 0;

    while (i < LOW_BLKS && ((i1 == 0) || (i2 == 0)) ) {
	if (mem_table[i].used) {
	    if ((long)mem_table[i].addr + mem_table[i].size == (long)p) 
			i1 = i;
	    if ((char *)mem_table[i].addr ==((char *)p + len))
			i2 = i;
	}
	i++;
    }
    if (i1 != 0 && i2 != 0) {
	mem_table[i1].size += mem_table[i2].size + len;
	mem_table[i2].used = FALSE;
    }
    else if (i1 == 0 && i2 != 0) {
	mem_table[i2].addr = p;
	mem_table[i2].size += len;
    }
    else if (i1 != 0 && i2 == 0) mem_table[i1].size += len;
    else {
	i = 0;
	while (i < LOW_BLKS && (mem_table[i].used == TRUE)) i++;
	if (i == LOW_BLKS) return(FALSE);
	mem_table[i].addr = p;
	mem_table[i].size = len;
	mem_table[i].used = TRUE;
    }
    return(TRUE);
}

void low_dump_mem(void)
{
    register int i;
    for (i = 0; i < LOW_BLKS; i++) {
	if (mem_table[i].used) 
		kprintf("(%d) Addr : %x   Size : %d/%d\n",
			i, mem_table[i].addr,
			mem_table[i].size, mem_table[i].size);
    }
}


