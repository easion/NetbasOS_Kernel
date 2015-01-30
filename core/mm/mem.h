#ifndef __MEM_MINIX_H__
#define __MEM_MINIX_H__


#include <jicama/system.h>
#include <jicama/process.h>

//take it from minix os


#define NR_HOLES  (20*NR_PROC)	/* max # entries in hole table */
#define NIL_HOLE (struct hole *) 0
#define NO_MEM (unsigned int) 0  /* returned by alloc_mem() with mem is up */




struct hole {
  struct hole *h_next;		/* pointer to next entry on the list */
  u32_t h_base;		/* where does the hole begin? */
  u32_t h_len;		/* how big is the hole? */
} ;

struct kmalloc_block
{
	u32_t base;
	u32_t size;
};

int kmalloc_init();

#endif
