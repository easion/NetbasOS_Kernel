#include "mem.h"

#if 0
__local struct hole *hole_head;	/* pointer to first hole */
__local struct hole *free_slots;/* ptr to list of unused table slots */

#define swap_base ((u32_t) -1)

__local  void del_slot (struct hole *prev_ptr, struct hole *hp );
__local  void merge (struct hole *hp)			    ;

#define swap_out()	(0)

/*===========================================================================*
 *				alloc_mem				     *
 *===========================================================================*/
__public  u32_t mm_malloc(u32_t clicks)
		/* amount of memory requested */
{
  register struct hole *hp, *prev_ptr;
  u32_t old_base;

  do {
	hp = hole_head;
	while (hp != NIL_HOLE && hp->h_base < swap_base) {
		if (hp->h_len >= clicks) {
			/* We found a hole that is big enough.  Use it. */
			old_base = hp->h_base;	/* remember where it started */
			hp->h_base += clicks;	/* bite a piece off */
			hp->h_len -= clicks;	/* ditto */

			/* Delete the hole if used up completely. */
			if (hp->h_len == 0) del_slot(prev_ptr, hp);

			/* Return the start address of the acquired block. */
			return(old_base);
		}

		prev_ptr = hp;
		hp = hp->h_next;
	}
  } while (swap_out());		/* try to swap some other process out */
  return(NO_MEM);
}

/*===========================================================================*
 *				mm_free				     *
 *===========================================================================*/
 void mm_free(u32_t base,size_t clicks)
	/* base address of block to free */
		/* number of clicks to free */
{
/* Return a block of free memory to the hole list.  The parameters tell where
 * the block starts in physical memory and how big it is.  The block is added
 * to the hole list.  If it is contiguous with an existing hole on either end,
 * it is merged with the hole or holes.
 */

  register struct hole *hp, *new_ptr, *prev_ptr;

  //kprintf("free call%d", clicks);

  if (clicks == 0) return;
  if ( (new_ptr = free_slots) == NIL_HOLE)
	  panic("Hole table full");
  new_ptr->h_base = base;
  new_ptr->h_len = clicks;
  free_slots = new_ptr->h_next;
  hp = hole_head;

  /* If this block's address is numerically less than the lowest hole currently
   * available, or if no holes are currently available, put this hole on the
   * front of the hole list.
   */
  if (hp == NIL_HOLE || base <= hp->h_base) {
	/* Block to be freed goes on front of the hole list. */
	new_ptr->h_next = hp;
	hole_head = new_ptr;
	merge(new_ptr);
	return;
  }

  /* Block to be returned does not go on front of hole list. */
  while (hp != NIL_HOLE && base > hp->h_base) {
	prev_ptr = hp;
	hp = hp->h_next;
  }

  /* We found where it goes.  Insert block after 'prev_ptr'. */
  new_ptr->h_next = prev_ptr->h_next;
  prev_ptr->h_next = new_ptr;
  merge(prev_ptr);		/* sequence is 'prev_ptr', 'new_ptr', 'hp' */
}

/*===========================================================================*
 *				del_slot				     *
 *===========================================================================*/
__local void del_slot(register struct hole *prev_ptr, register struct hole *hp)
	/* pointer to hole entry just ahead of 'hp' */
/* pointer to hole entry to be removed */
{
  if (hp == hole_head)
	hole_head = hp->h_next;
  else
	prev_ptr->h_next = hp->h_next;

  hp->h_next = free_slots;
  free_slots = hp;
}

/*===========================================================================*
 *				merge					     *
 *===========================================================================*/
__local void merge(register struct hole *hp)
	/* ptr to hole to merge with its successors */
{
  register struct hole *next_ptr;

  /* If 'hp' points to the last hole, no merging is possible.  If it does not,
   * try to absorb its successor into it and free the successor's table entry.
   */
  if ( (next_ptr = hp->h_next) == NIL_HOLE) return;
  if (hp->h_base + hp->h_len == next_ptr->h_base) {
	hp->h_len += next_ptr->h_len;	/* first one gets second one's mem */
	del_slot(hp, next_ptr);
  } else {
	hp = next_ptr;
  }

  /* If 'hp' now points to the last hole, return; otherwise, try to absorb its
   * successor into it.
   */
  if ( (next_ptr = hp->h_next) == NIL_HOLE) return;
  if (hp->h_base + hp->h_len == next_ptr->h_base) {
	hp->h_len += next_ptr->h_len;
	del_slot(hp, next_ptr);
  }
}

/*===========================================================================*
 *				mem_init				     *
 *===========================================================================*/
__public  void kernel_heap_init(u32_t start, unsigned int len )
{
	__local struct hole hole[NR_HOLES];
	register struct hole *hp;
	u32_t base;		/* base address of chunk */
	u32_t size;		/* size of chunk */
	extern u32_t page_dir_length;

	/* Put all holes on the free list. */
	for (hp = &hole[0]; hp < &hole[NR_HOLES]; hp++) {
		hp->h_len = 0;
		hp->h_base = 0;
		hp->h_next = hp + 1;
	}
	hole[NR_HOLES-1].h_next = NIL_HOLE;
	hole_head = NIL_HOLE;

	hole_head = &hole[0];      /////指向已使用内存
	free_slots = &hole[1];

	hole_head->h_base = start;	
	hole_head->h_len = len;
	hole_head->h_next = free_slots;
	kmalloc_init();
}

int dump_pages(char *buf, int len);

int mem_alloc_used()
{
  struct hole *hp;
  int cnt = 0;

  hp = hole_head;

  while (hp!=NIL_HOLE && hp->h_base < swap_base)
  {
	  cnt++;
	  hp = hp->h_next;
	  if (cnt >= NR_HOLES)
	  {
		  break;
	  }
  }
  kprintf("cnt = %d\n", cnt);
  return cnt;
}

int mem_dump(const char * buf, int size)
{
	int cnt=0,c;
	c = sprintf(buf+cnt, "minix alloc: base 0x%08x, length %d used %d blocks\n", 
		hole_head->h_base, hole_head->h_len, mem_alloc_used());
	cnt += c;
	c = sprintf(buf+cnt,"free queue: base 0x%08x, length %d, holes %d\n",
		free_slots->h_base,free_slots->h_len,
		NR_HOLES );
	cnt += c;
	c = dump_pages(buf+cnt, size-cnt);
	cnt += c;
	return cnt;
}
#endif
