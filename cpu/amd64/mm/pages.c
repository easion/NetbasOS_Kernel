

#include <jicama/system.h>
#include <jicama/process.h>
#include <string.h>

int mem_writeable(void* address,int size)
{
	return 0;
}

int free_proc_mem_space(proc_t *rp)
{
	return -1;
}

void cleanmmap(proc_t *rp)
{
}

unsigned long get_page()
{
	return 0;
}


/* Using 2 MByte page translation */
 u64_t pae_pde[512];
 u64_t pae_pdp[512]; 
 u64_t pae_pml4[512];

#define FIELD_P	   (1<<0)   /* Present */
#define FIELD_RW   (1<<1)	/* Read/Write */
#define FIELD_US   (1<<2)	/* user */
#define FIELD_PS   (1<<7)   /* 2Mb entries */

unsigned short* const VIDMEM = (unsigned short*) 0x000b8480;

void print( const char* str )
{
	static unsigned short* vp = VIDMEM;

    while (*str != '\0')
    {
        *vp++ = (0x0700 | ((unsigned short) *str++));
    }	
}

void pae_setup()
{
	int x;

	print(">> pae_setup()");

	for( x = 0; x< 512; x++)
	{
		pae_pde[x] =  pae_pdp[x] = pae_pde[x] = 0;
	}

/* map first 6mb of memory */

	pae_pde[0] = (u64_t)(0x0) << 21;
 	pae_pde[0] |= (FIELD_P | FIELD_RW | FIELD_PS);

	pae_pde[1] = (u64_t)(0x200000) << 21;
 	pae_pde[1] |= (FIELD_P | FIELD_RW | FIELD_PS);

	pae_pde[2] = (u64_t)(0x400000) << 21;
 	pae_pde[2] |= (FIELD_P | FIELD_RW | FIELD_PS);

	pae_pde[3] = (u64_t)(0x600000) << 21;
 	pae_pde[3] |= (FIELD_P | FIELD_RW | FIELD_PS);

	pae_pdp[0]  = (u64_t)(pae_pde) << 12;
 	pae_pdp[0]  |= (FIELD_P | FIELD_RW);

	pae_pml4[0] =  (u64_t)(pae_pdp) << 12;
	pae_pml4[0] |= (FIELD_P | FIELD_RW);

	print("<< pae_setup()");
	
}


void flush_tbl()
{
	unsigned long addr=0x200000;
	//__asm__ __volatile__  ("movq %%rax,%%cr3\n"::"a" ( 0x200000 ));
	//__asm  ("invlpg %0": :"m" (0x200000));
	__asm__ __volatile__("invlpg %0": :"m" (*(char *) addr));
}

#define PAGING64_FIRST_TSK_PAGES 0x8000 +(0x400000>>12)

void	page_table_init()
{
}

void memory_init()
{

}

