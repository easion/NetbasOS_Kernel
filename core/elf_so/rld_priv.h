/*
** Copyright 2002, Manuel J. Petit. All rights reserved.
** Distributed under the terms of the NewOS License.
*/

#ifndef __newos__run_time_linker__hh__
#define __newos__run_time_linker__hh__



//#define DEBUG_RLD 1
//typedef int int;      // vm region id

#define MAGIC_APPNAME	"__ELF_SO_APP__"
enum {
	REGION_ADDR_ANY_ADDRESS = 0,
	REGION_ADDR_EXACT_ADDRESS
};




void *dynamic_symbol(proc_t *rp, dynmodule_id imid, char const *symbol);



enum {
	REGION_WIRING_LAZY = 0,
	REGION_WIRING_WIRED,
	REGION_WIRING_WIRED_ALREADY,
	REGION_WIRING_WIRED_CONTIG
 };

typedef void *elf_vmid;

typedef
struct elf_region_t {
	elf_vmid id;
	addr_t start;
	addr_t size;
	addr_t vmstart;
	addr_t vmsize;
	addr_t fdstart;
	addr_t fdsize;
	long delta;
	unsigned flags;
} elf_region_t;

#define SYS_MAX_OS_NAME_LEN   128

typedef
struct image_t {
	/*
	 * image identification
	 */
	char     name[SYS_MAX_OS_NAME_LEN];
	dynmodule_id imageid;

	struct   image_t *next;
	struct   image_t *prev;
	int      refcount;
	unsigned flags;

	addr_t entry_point;
	addr_t dynamic_ptr; // pointer to the dynamic section


	// pointer to symbol participation data structures
	unsigned int      *symhash;
	elf_sym_t  *syms;
	char              *strtab;
	elf32_rel_t  *rel;
	int                rel_len;
	elf32_rela_t *rela;
	int                rela_len;
	elf32_rel_t  *pltrel;
	int                pltrel_len;
	int                pltrel_type; // DT_REL or DT_RELA

	unsigned           num_needed;
	struct image_t   **needed;

	// describes the text and data regions
	unsigned     num_regions;
	void *start_ctors, *end_ctors;
	//void *__ctor_list, *__ctor_end;
	void *e_init, *e_finit;
	elf_region_t regions[1];
} image_t;




elf_vmid _kern_vm_map_file(struct filp*fp, void **address, int addr_type,
	addr_t size,  proc_t *rp, off_t offset);
elf_vmid _kern_vm_create_anonymous_region(proc_t *rp, void **address, int addr_type,
	addr_t size, int attr);

int get_dependencies(image_t *img, bool init_head, void **arg);
#define	PAGE_MASK ((PAGE_SIZE)-1)
#define	PAGE_OFFS(y) ((y)&(PAGE_MASK))
#define	PAGE_BASE(y) ((y)&~(PAGE_MASK))

#define	ROUNDOWN(x,y) ((x)&~((y)-1))
#define	ROUNDUP(x,y) ROUNDOWN(x+y-1,y)

 image_t *
find_image(proc_t *rp,char const *name);
void enqueue_image(image_queue_t *queue, image_t *img);
image_t *
create_image(proc_t *rp, char const *name, int num_regions);
void *dynamic_get_symbol(image_t *iter, char const *symname);
image_t *load_container(proc_t *rp, struct filp*fp, char const *path, char const *name, bool fixed, long *bssend);
 int load_dependencies(proc_t *rp, image_t *img,char *basepath);
 bool relocate_image(proc_t *rp,image_t *image);
 elf_sym_t *find_symbol_xxx(image_t *img, const char *_symbol);






#endif

