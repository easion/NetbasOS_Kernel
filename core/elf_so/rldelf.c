/*
** Copyright 2001, Travis Geiselbrecht. All rights reserved.
** Copyright 2002, Manuel J. Petit. All rights reserved.
** Distributed under the terms of the NewOS License.
*/

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/elf.h>
#include <jicama/module.h>
#include <string.h>
#include <assert.h>

#include "rld_priv.h"
typedef void (libinit_f)(unsigned, void const *);


#if ARCH_sh4
#define ELF_PREPEND_UNDERSCORE 1
#endif

#ifndef ELF_PREPEND_UNDERSCORE
#define ELF_PREPEND_UNDERSCORE 0
#endif


enum {
	RFLAG_RW             = 0x0001,
	RFLAG_ANON           = 0x0002,

	RFLAG_SORTED         = 0x0400,
	RFLAG_SYMBOLIC       = 0x0800,
	RFLAG_RELOCATED      = 0x1000,
	RFLAG_PROTECTED      = 0x2000,
	RFLAG_INITIALIZED    = 0x4000,
	RFLAG_NEEDAGIRLFRIEND= 0x8000
};




static int resolve_symbol(proc_t *rp, image_t *image, elf_sym_t *sym, addr_t *sym_addr);


#define STRING(image, offset) ((char *)(&(image)->strtab[(offset)]))
#define SYMNAME(image, sym) STRING(image, (sym)->name)
#define SYMBOL(image, num) ((elf_sym_t *)&(image)->syms[num])
#define HASHTABSIZE(image) ((image)->symhash[0])
#define HASHBUCKETS(image) ((unsigned int *)&(image)->symhash[2])
#define HASHCHAINS(image) ((unsigned int *)&(image)->symhash[2+HASHTABSIZE(image)])


/*
 * This macro is non ISO compliant, but a gcc extension
 */
#define	FATAL(x,n,y...) \
	if(x) { \
		kprintf("rld.so: " y); \
		return ((n)); \
	}

static
int
relocate_rel(proc_t *rp, image_t *image, elf32_rel_t *rel, int rel_len )
{
	int i;
	elf_sym_t *sym;
	int vlErr;
	addr_t S=0;
	addr_t final_val;
	addr_t base=0;
	int symidx;

	base = proc_phy_addr(rp);
	//(char*)rel += base;

# define P1         ((addr_t *)(base+image->regions[0].delta + rel[i].r_offset))
# define P         ((addr_t *)(image->regions[0].delta + rel[i].r_offset))
# define A         (*(P1))
# define B         (image->regions[0].delta)

	for(i = 0; i * (int)sizeof(elf32_rel_t) < rel_len; i++) {

		unsigned type= ELF32_R_TYPE(rel[i].r_info);

		switch(ELF32_R_TYPE(rel[i].r_info)) {
			case R_386_32:
			case R_386_PC32:
			case R_386_GLOB_DAT:
			case R_386_JMP_SLOT:
			case R_386_GOTOFF:
			case R_386_COPY:
				symidx = ELF32_R_SYM(rel[i].r_info);

				sym = SYMBOL(image,symidx );

				vlErr = resolve_symbol(rp,image, sym, &S);
				if(vlErr<0) {
					kprintf("resolve_symbol error\n");
					return vlErr;
				}
		}
		switch(type) {
			case R_386_NONE:
				continue;
			case R_386_32:
				final_val= S+A;
				break;
			case R_386_PC32:
				final_val=S+A-(addr_t)P;
				break;
#if 0
			case R_386_GOT32:
				final_val= G+A;
				break;
			case R_386_PLT32:
				final_val= L+A-(addr_t)P;
				break;
#endif
			case R_386_COPY:
			kprintf("R_386_COPY  processed %d bytes\n",sym->size);
			memcpy( P1, S+base, sym->size );
				/* what ? */
				continue;
			case R_386_GLOB_DAT:
				final_val= S;
				break;
			case R_386_JMP_SLOT:
				final_val= S;
				break;
			case R_386_RELATIVE:
				final_val= B+A;
				break;
#if 0
			case R_386_GOTOFF:
				final_val= S+A-GOT;
				break;
			case R_386_GOTPC:
				final_val= GOT+A-P;
				break;
#endif
			default:
				kprintf("unhandled relocation type %d\n", ELF32_R_TYPE(rel[i].r_info));
				return -1;
		}

		*P1= final_val;
	}

# undef P1
# undef P
# undef A
# undef B


	return 0;
}

/*
 * rldelf.c requires this function to be implemented on a per-cpu basis
 */

bool relocate_image(proc_t *rp,image_t *image)
{
	int res = 0;
	int i;

	if(image->flags & RFLAG_RELOCATED) {
		return true;
	}
	image->flags|= RFLAG_RELOCATED;

	// deal with the rels first
	if(image->rel) {
		res= relocate_rel( rp,image, image->rel, image->rel_len );

		if(res) {
			return false;
		}
	}

	if(image->pltrel) {
		res= relocate_rel(rp, image, image->pltrel, image->pltrel_len);

		if(res) {
			return false;
		}
	}

	if(image->rela) {
		kprintf("RELA relocations not supported\n");
		return false;
		for(i = 1; i * (int)sizeof(elf32_rela_t) < image->rela_len; i++) {
			kprintf("rela: type %d\n", ELF32_R_TYPE(image->rela[i].r_info));
		}
	}

	return true;
}


static
unsigned long
elf_sysv_hash(const unsigned char *name)
{
	unsigned long hash = 0;
	unsigned long temp;

	while(*name) {
		hash = (hash << 4) + *name++;
		if((temp = hash & 0xf0000000)) {
			hash ^= temp >> 24;
		}
		hash &= ~temp;
	}
	return hash;
}



static
int
parse_eheader(elf_file_t *eheader)
{
	if (
		(eheader->ident[EI_MAG0] != ELFMAG0) ||
		(eheader->ident[EI_MAG1] != ELFMAG1) ||
		(eheader->ident[EI_MAG2] != ELFMAG2) ||
		(eheader->ident[EI_MAG3] != ELFMAG3)
	)
		return -1;

	if(eheader->ident[EI_CLASS] != ELF_CLASS32)
		return -1;

	if(eheader->phtab_offset == 0)
		return -1;

	if(eheader->phtab_ent_size < sizeof(elfphdr_t))
		return -1;

	return eheader->phtab_ent_size*eheader->num_phtab_ents;
}

static
int
count_regions(char const *buff, int phnum, int phentsize)
{
	int i;
	int retval;
	elfphdr_t *pheaders;

	retval= 0;
	for(i= 0; i< phnum; i++) {
		pheaders= (elfphdr_t *)(buff+i*phentsize);

		switch(pheaders->type) {
			case PT_NULL:
				/* NOP header */
				break;
			case PT_LOAD:
				retval+= 1;
				if(pheaders->memsz!= pheaders->filesz) {
					unsigned A= pheaders->vaddr+pheaders->memsz-1;
					unsigned B= pheaders->vaddr+pheaders->filesz-1;

					A= PAGE_BASE(A);
					B= PAGE_BASE(B);

					if(A!= B) {
						retval+= 1;
					}
				}
				break;
			case PT_DYNAMIC:
				/* will be handled at some other place */
				break;
			case PT_INTERP:
				/* should check here for appropiate interpreter */
				break;
			case PT_NOTE:
				/* unsupported */
				break;
			case PT_SHLIB:
				/* undefined semantics */
				break;
			case PT_PHDR:
				/* we don't use it */
				break;
			default:
				FATAL(true, -1, "unhandled pheader type 0x%x\n", pheaders[i].type);
				break;
		}
	}

	

	return retval;
}



static
long 
parse_program_sects(proc_t *rp, image_t *image, char *buff, int phnum)
{
	int i;
	int regcount;
	elf_sect_t *sect;
	unsigned endbss=0;


	regcount= 0;
	for(i= 0; i< phnum; i++) {
		sect= (elfphdr_t *)(buff+i*sizeof(elf_sect_t));

		if ( sect->type == SHT_NULL ) {
		  continue;
		}

		switch (sect->type)
		{
		case SHT_PROGBITS:
			//kprintf("endbss SHT_PROGBITS = %x\n",sect->size);
			break;

			case SHT_DYNSYM :
				//kprintf("endbss SHT_DYNSYM = %x\n",sect->size);
        //nDynSymSec = i;
        break;

      case SHT_DYNAMIC :
		  //kprintf("endbss SHT_DYNAMIC = %x\n",sect->size);
        //nDynamicSec = i;
        break;

      case SHT_HASH :
		 // kprintf("endbss SHT_HASH = %x\n",sect->size);
        //nHashSec = i;
        break;
		
		}

		if (sect->flags & SHF_ALLOC)
		{
			if (sect->type == SHT_NOBITS )
			{
				//kprintf("endbss 000 = %x\n",sect->size);
				//endbss = sect->virt_adr + sect->size - 1;
				endbss =  sect->size ;
				break;
			}else{
				//endbss = sect->virt_adr + sect->size - 1;
			}
		}		
	}

	

	return endbss;
}

static
int 
parse_program_headers(proc_t *rp, image_t *image, char *buff, int phnum, int phentsize,unsigned bss)
{
	int i;
	int regcount;
	elfphdr_t *pheaders;
	int left=0;
	char *program_interpreter;


	regcount= 0;
	for(i= 0; i< phnum; i++) {
		pheaders= (elfphdr_t *)(buff+i*phentsize);

		switch(pheaders->type) {
			case PT_NULL:
				/* NOP header */
				break;
			case PT_LOAD:
				if(pheaders->memsz== pheaders->filesz) {
					/*
					 * everything in one area
					 */
					image->regions[regcount].start  = pheaders->vaddr;
					image->regions[regcount].size   = pheaders->memsz;
					image->regions[regcount].vmstart= ROUNDOWN(pheaders->vaddr, PAGE_SIZE);
					image->regions[regcount].vmsize = ROUNDUP (pheaders->memsz + (pheaders->vaddr % PAGE_SIZE), PAGE_SIZE);
					image->regions[regcount].fdstart= pheaders->offset;
					image->regions[regcount].fdsize = pheaders->filesz;
					image->regions[regcount].delta= 0;
					image->regions[regcount].flags= 0;
					if(pheaders->flags & PF_W) {
						// this is a writable segment
						image->regions[regcount].flags|= RFLAG_RW;
					}
				} else {
					/*
					 * may require splitting
					 */
					unsigned A= pheaders->vaddr+pheaders->memsz-1;
					unsigned B= pheaders->vaddr+pheaders->filesz-1;

					A= PAGE_BASE(A);
					B= PAGE_BASE(B);

					image->regions[regcount].start  = pheaders->vaddr;
					image->regions[regcount].size   = pheaders->filesz;
					image->regions[regcount].vmstart= ROUNDOWN(pheaders->vaddr, PAGE_SIZE);
					image->regions[regcount].vmsize = ROUNDUP (pheaders->filesz + (pheaders->vaddr % PAGE_SIZE), PAGE_SIZE);
					image->regions[regcount].fdstart= pheaders->offset;
					image->regions[regcount].fdsize = pheaders->filesz;
					image->regions[regcount].delta= 0;
					image->regions[regcount].flags= 0;
					if(pheaders->flags & PF_W) {
						// this is a writable segment
						image->regions[regcount].flags|= RFLAG_RW;
					}

					if(A!= B) {
						/*
						 * yeah, it requires splitting
						 */
						regcount+= 1;
						image->regions[regcount].start  = pheaders->vaddr;
						image->regions[regcount].size   = pheaders->memsz - pheaders->filesz+bss;
						image->regions[regcount].vmstart= image->regions[regcount-1].vmstart + image->regions[regcount-1].vmsize;
						image->regions[regcount].vmsize = ROUNDUP (pheaders->memsz +bss+ (pheaders->vaddr % PAGE_SIZE), PAGE_SIZE) - image->regions[regcount-1].vmsize;
						image->regions[regcount].fdstart= 0;
						image->regions[regcount].fdsize = 0;
						image->regions[regcount].delta= 0;
						image->regions[regcount].flags= RFLAG_ANON;
						if(pheaders->flags & PF_W) {
							// this is a writable segment
							image->regions[regcount].flags|= RFLAG_RW;
						}
					}
				}

			
				regcount+= 1;
				break;
			case PT_DYNAMIC:
				//kprintf("set dynamic_ptr to %x\n", pheaders->vaddr);
				image->dynamic_ptr = pheaders->vaddr;
				break;
			case PT_INTERP:
				//-Wl,-dynamic-linker=/opt/diet/lib-i386/libdl.so
				//program_interpreter = (char *) (file_contents + pheaders->offset);
			//kprintf("program_interpreter=%s\n",program_interpreter);
				/* should check here for appropiate interpreter */
				break;
			case PT_NOTE:
				/* unsupported */
				break;
			case PT_SHLIB:
				/* undefined semantics */
				break;
			case PT_PHDR:
				/* we don't use it */
				break;
			default:
				FATAL(true,-1, "unhandled pheader type 0x%x\n", pheaders[i].type);
				break;
		}
	}
	

	return 0;
}

bool
assert_dynamic_loadable(image_t *image)
{
	unsigned i;

	if(!image->dynamic_ptr) {
		return true;
	}

	for(i= 0; i< image->num_regions; i++) {
		if(image->dynamic_ptr>= image->regions[i].start) {
			if(image->dynamic_ptr< image->regions[i].start+image->regions[i].size) {
				return true;
			}
		}
	}

	return false;
}



static bool
map_image(proc_t *rp, void* fd, char const *path, image_t *image, bool fixed)
{
	unsigned i,dsize=0;

	addr_t base=0;

	base = proc_phy_addr(rp);

	for(i= 0; i< image->num_regions; i++) {
		char     region_name[256];
		addr_t     load_address;
		unsigned addr_specifier;

		sprintf(
			region_name,
			"%s:seg_%d(%s)",
			path,
			i,
			(image->regions[i].flags&RFLAG_RW)?"RW":"RO"
		);

		//kprintf("region_name=%s\n", region_name);

		if(image->dynamic_ptr && !fixed) {
			/*
			 * relocatable image... we can afford to place wherever
			 */
			if(i== 0) {
				/*
				 * but only the first segment gets a free ride
				 */
				load_address= 0;
				addr_specifier= REGION_ADDR_ANY_ADDRESS;
			} else {
				load_address= image->regions[i].vmstart + image->regions[i-1].delta;
				addr_specifier= REGION_ADDR_EXACT_ADDRESS;
			}
		} else {
			/*
			 * not relocatable, put it where it asks or die trying
			 */
			load_address= image->regions[i].vmstart;
			addr_specifier= REGION_ADDR_EXACT_ADDRESS;
		}

		if(image->regions[i].flags & RFLAG_ANON) {
#if DEBUG_RLD
			kprintf("rld map_image: creating anon region: name '%s' address 0x%x specifier 0x%x size 0x%x\n",
				region_name, load_address, addr_specifier, image->regions[i].vmsize);
#endif
			image->regions[i].id= _kern_vm_create_anonymous_region(
				rp,
				(void **)&load_address,
				addr_specifier,
				image->regions[i].vmsize,
				REGION_WIRING_LAZY
			);

			if(image->regions[i].id < 0) {
				kprintf("rld map_image: err %d from create_anon_region\n", image->regions[i].id);
				goto error;
			}
			image->regions[i].delta  = load_address - image->regions[i].vmstart;
			image->regions[i].vmstart= load_address;
		} else {
#if DEBUG_RLD
			kprintf("rld map_image: mapping file: name '%s' address 0x%x specifier 0x%x size 0x%x path '%s' offset 0x%Lx\n",
				region_name, load_address, addr_specifier, image->regions[i].vmsize,
				path, ROUNDOWN(image->regions[i].fdstart, PAGE_SIZE));
#endif
			image->regions[i].id= _kern_vm_map_file(
				fd,
				(void **)&load_address,
				addr_specifier,
				image->regions[i].vmsize,
				rp,
				ROUNDOWN(image->regions[i].fdstart, PAGE_SIZE)
			);



			if(image->regions[i].id < 0) {
				kprintf("rld map_image: err %d from map_file (address 0x%x)\n", image->regions[i].id, load_address);
				goto error;
			}
			image->regions[i].delta  = load_address - image->regions[i].vmstart;
			image->regions[i].vmstart= load_address;

			/*
			 * handle trailer bits in data segment
			 */
			if(image->regions[i].flags & RFLAG_RW) {
				unsigned start_clearing;
				unsigned to_clear;

				start_clearing=
					image->regions[i].vmstart
					+ PAGE_OFFS(image->regions[i].start)
					+ image->regions[i].size;
				to_clear=
					image->regions[i].vmsize
					- PAGE_OFFS(image->regions[i].start)
					- image->regions[i].size;
				memset((void*)start_clearing+base, 0, to_clear);
			}
		}
	}

	if(image->dynamic_ptr) {
		//kprintf("change dynamic_ptr + %x\n",  image->regions[0].delta);
		image->dynamic_ptr+=( image->regions[0].delta+base);
	}

	return true;

error:
	return false;
}


static bool parse_dynamic_segment(proc_t *rp, image_t *image)
{
	struct Elf32_Dyn *d;
	int i;
	addr_t base=0;

	image->symhash = 0;
	image->syms = 0;
	image->strtab = 0;


	if(!image->dynamic_ptr) {
		//kprintf("parse_dynamic_segment error\n");
		return true;
	}

	base = proc_phy_addr(rp);
	d = (struct Elf32_Dyn *)(image->dynamic_ptr);

	//kprintf("Elf32_Dyn at 0x%x tag-%d\n",(long)d,d[0].d_tag);

	for(i=0; d[i].d_tag != DT_NULL; i++) {
		switch(d[i].d_tag) {
			case DT_NEEDED:
				image->num_needed+= 1;
				break;
			case DT_HASH:
				image->symhash = (unsigned int *)(base+d[i].d_un.d_ptr + image->regions[0].delta);
				break;
			case DT_STRTAB:
				image->strtab = (char *)(base+d[i].d_un.d_ptr + image->regions[0].delta);
				break;
			case DT_SYMTAB:
				image->syms = (elf_sym_t *)(base+d[i].d_un.d_ptr + image->regions[0].delta);
				break;
			case DT_REL:
				image->rel = (elf32_rel_t *)(base+d[i].d_un.d_ptr + image->regions[0].delta);
				break;
			case DT_RELSZ:
				image->rel_len = d[i].d_un.d_val;
				break;
			case DT_RELA:
				image->rela = (elf32_rela_t *)(base+d[i].d_un.d_ptr + image->regions[0].delta);
				break;
			case DT_RELASZ:
				image->rela_len = d[i].d_un.d_val;
			//kprintf("image->pltrel_len = %d\n", image->pltrel_len);
				break;
			// TK: procedure linkage table
			case DT_JMPREL:
				image->pltrel = (elf32_rel_t *)(base+d[i].d_un.d_ptr + image->regions[0].delta);
			//kprintf("image->pltrel_len = %p addr=%p \n", image->pltrel, d[i].d_un.d_ptr + image->regions[0].delta);
				break;

				case DT_RELENT :
       // nDynRelEntSize = d[i].d_un.d_val;
				//kprintf("DT_RELENT image->DT_RELENT = %d\n", d[i].d_un.d_val);
        break;

			case DT_PLTRELSZ:
				image->pltrel_len = d[i].d_un.d_val;
				break;
			case DT_PLTREL:
				image->pltrel_type = d[i].d_un.d_val;
				break;

			case DT_INIT :
				image->e_init =(void*) (d[i].d_un.d_ptr + image->regions[0].delta);
				break;

			case DT_FINI :
				image->e_finit =(void*)  (d[i].d_un.d_ptr + image->regions[0].delta);
				break;

			default:
				continue;
		}
	}

	//kprintf("parse_dynamic_segment found %d,%d entrys\n",(long)i,image->pltrel_len);

	// lets make sure we found all the required sections
	if(!image->symhash || !image->syms || !image->strtab) {
		kprintf("error: hash=%x syms%x strtab=%x\n", image->symhash, image->syms,image->strtab);
		return false;
	}

	return true;
}

elf_sym_t *find_symbol_xxx(image_t *img, const char *_symbol)
{
	unsigned int hash;
	unsigned int i;
	const char *symbol;

	/* some architectures prepend a '_' to symbols, so lets do it here for lookups */
#if ELF_PREPEND_UNDERSCORE
	char new_symbol[SYS_MAX_NAME_LEN];

	new_symbol[0] = '_';
	new_symbol[1] = 0;
	strlcat(new_symbol, _symbol, SYS_MAX_NAME_LEN);
	symbol = new_symbol;
#else
	symbol = _symbol;
#endif

	if(img->dynamic_ptr) {
		hash = elf_sysv_hash(symbol) % HASHTABSIZE(img);
		for(i = HASHBUCKETS(img)[hash]; i != STN_UNDEF; i = HASHCHAINS(img)[i]) {
			if(img->syms[i].section!= SHN_UNDEF) {
				if((ELF32_ST_BIND(img->syms[i].info)== STB_GLOBAL) || (ELF32_ST_BIND(img->syms[i].info)== STB_WEAK)) {
					if(!strcmp(SYMNAME(img, &img->syms[i]), symbol)) {
						return &img->syms[i];
					}
				}
			}
		}
	}

	return NULL;
}

static elf_sym_t *
find_symbol(proc_t *rp, image_t **shimg, const char *name)
{
	image_t *iter;
	unsigned int hash;
	unsigned int i;

	iter= rp->dynamic_module_list.head;
	while(iter) {
		if(iter->dynamic_ptr) {
			hash = elf_sysv_hash(name) % HASHTABSIZE(iter);
			for(i = HASHBUCKETS(iter)[hash]; i != STN_UNDEF; i = HASHCHAINS(iter)[i]) {
				if(iter->syms[i].section!= SHN_UNDEF) {
					if((ELF32_ST_BIND(iter->syms[i].info)== STB_GLOBAL) || (ELF32_ST_BIND(iter->syms[i].info)== STB_WEAK)) {
						if(!strcmp(SYMNAME(iter, &iter->syms[i]), name)) {
							*shimg= iter;
							return &iter->syms[i];
						}
					}
				}
			}
		}

		iter= iter->next;
	}

	return NULL;
}

static int
resolve_symbol(proc_t *rp, image_t *image, elf_sym_t *sym, addr_t *sym_addr)
{
	elf_sym_t *sym2;
	char             *symname;
	image_t          *shimg;	

	switch(sym->section) {
		case SHN_UNDEF:
			// patch the symbol name
			symname= SYMNAME(image, sym);

			//kprintf("resolve_symbol = %s\n", symname);

			// it's undefined, must be outside this image, try the other image
			sym2 = find_symbol(rp,&shimg, symname);
			if(!sym2) {
				kprintf("elf_resolve_symbol: could not resolve symbol '%s'\n", symname);
				return -1;
			}

			// make sure they're the same type
			if(ELF32_ST_TYPE(sym->info)!= STT_NOTYPE) {
				if(ELF32_ST_TYPE(sym->info) != ELF32_ST_TYPE(sym2->info)) {
					kprintf("elf_resolve_symbol: found symbol '%s' in shared image but wrong type\n", symname);
					return -1;
				}
			}

			if(ELF32_ST_BIND(sym2->info) != STB_GLOBAL && ELF32_ST_BIND(sym2->info) != STB_WEAK) {
				kprintf("elf_resolve_symbol: found symbol '%s' but not exported\n", symname);
				return -1;
			}

			*sym_addr = sym2->value + shimg->regions[0].delta;
			return 0;
		case SHN_ABS:
			*sym_addr = sym->value + image->regions[0].delta;
			return 0;
		case SHN_COMMON:
			// XXX finish this
			kprintf("elf_resolve_symbol: COMMON symbol, finish me!\n");
			return -2;
		default:
			//kprintf("resolve_symbol sym->section= %d\n", sym->section);
			// standard symbol
			*sym_addr = sym->value + image->regions[0].delta;
			return 0;
	}
}


//#include "arch/rldreloc.inc"


image_t *load_container(proc_t *rp, struct filp*fp, char const *path, char const *name, bool fixed, long *bssend)
{
	//int      fd;
	int      len;
	int      ph_len;
	char     ph_buff[40960];
	int      num_regions;
	bool     map_success;
	bool     dynamic_success;
	image_t *found;
	image_t *image;
	unsigned bss=0;

	elf_file_t eheader;

	found= find_image(rp,name);
	if(found) {
		found->refcount+= 1;
		return found;
	}

#if DEBUG_RLD
	kprintf("rld: load_container: path '%s', name '%s' entry\n", path, name);
#endif

	FATAL((fp== 0), NULL,"cannot open file %s\n", path);
	//fd= vfs_open(path, O_RDONLY,0);
	fseek(fp,0,0);

	fread((void*)&eheader, sizeof(elf_file_t), 1, fp);

	//len= read(fd, &eheader,  sizeof(eheader));
	//FATAL((len!= sizeof(eheader)),NULL, "troubles reading ELF header\n");

	ph_len= parse_eheader(&eheader);
	if(ph_len<= 0){
		kprintf("incorrect ELF header\n");
		return NULL;
	}

	if(ph_len> (int)sizeof(ph_buff)){
		kprintf("cannot handle Program headers bigger than %lu\n", (long unsigned)sizeof(ph_buff));
		return NULL;
	}

	//lseek(fd,eheader.phtab_offset,SEEK_SET);
	fseek(fp,eheader.shtab_offset,SEEK_SET);

	len= fread((void*)ph_buff, eheader.num_sects*sizeof(elf_sect_t), 1, fp);
	//len= read(fd, ph_buff,  ph_len);
	if(len!= eheader.num_sects*sizeof(elf_sect_t)){
		kprintf("troubles reading Program headers\n");
		return NULL;
	}
//kprintf("eheader.num_sects*sizeof(elf_sect_t) = %d\n",eheader.num_sects*sizeof(elf_sect_t));
	fseek(fp,eheader.phtab_offset,SEEK_SET);

	len= fread((void*)ph_buff, ph_len, 1, fp);
	//len= read(fd, ph_buff,  ph_len);
	if(len!= ph_len){
		kprintf("troubles reading Program headers\n");
		return NULL;
	}

	bss = parse_program_sects(rp,image, ph_buff, eheader.num_sects);
	if(bssend)
		*bssend = bss;

	num_regions= count_regions(ph_buff, eheader.num_phtab_ents, eheader.phtab_ent_size);
	if(num_regions<= 0){
		kprintf("troubles parsing Program headers, num_regions= %d\n", num_regions);
		return NULL;
	}

	//kprintf("num_regions=%d\n", num_regions);

	image= create_image(rp,name, num_regions);
	if(!image){
		kprintf( "failed to allocate image_t control block\n");
		return NULL;
	}

	parse_program_headers(rp,image, ph_buff, eheader.num_phtab_ents, eheader.phtab_ent_size,bss);
	FATAL(!assert_dynamic_loadable(image), NULL,"dynamic segment must be loadable (implementation restriction)\n");

	map_success= map_image(rp,fp, path, image, fixed);
	FATAL(!map_success, NULL,"troubles reading image\n");

	dynamic_success= parse_dynamic_segment(rp,image);
	FATAL(!dynamic_success, NULL,"troubles handling dynamic section\n");

	if (eheader.entry_pt)
	{
	image->entry_point= eheader.entry_pt + image->regions[0].delta;
	}
	else
		image->entry_point= NULL;


#if 0//DEBUG_RLD
	{
		int i;

		kprintf("rld: load_container: path '%s', name '%s' loaded:\n", path, name);
		kprintf("\tregions:\n");
		for(i=0; i<image->num_regions; i++) {
			kprintf("\t\tid %d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
				image->regions[i].id, image->regions[i].start, image->regions[i].size,
				image->regions[i].vmstart, image->regions[i].vmsize, image->regions[i].fdstart,
				image->regions[i].fdsize, image->regions[i].delta, image->regions[i].flags);
		}
	}
#endif

	enqueue_image(&rp->dynamic_module_list, image);

	image->start_ctors=dynamic_get_symbol(image,"start_ctors");
	image->end_ctors=dynamic_get_symbol(image,"end_ctors");

	if (!image->start_ctors)
	{

	image->start_ctors=dynamic_get_symbol(image,"__ctor_list");
	image->end_ctors=dynamic_get_symbol(image,"__ctor_end");
	//kprintf("__ctor_list 000 %x %x\n", image->start_ctors,image->end_ctors);
	}
	else{
	//kprintf("__ctor_list 111 %x %x\n", image->start_ctors,image->end_ctors);
	}





	
	return image;
}


 int load_dependencies(proc_t *rp, image_t *img,char *basepath)
{
	unsigned i;
	unsigned j;
	struct Elf32_Dyn *d;
	addr_t   needed_offset;
	long bss=0;
	char   path[256];
	//addr_t base=0;

	//base = proc_phy_addr(rp);
	d = (struct Elf32_Dyn *)img->dynamic_ptr;
	if(!d) {
		return;
	}

	img->needed= kmalloc(img->num_needed*sizeof(image_t*),0);
	FATAL((!img->needed),-1, "failed to allocate needed struct\n");
	memset(img->needed, 0, img->num_needed*sizeof(image_t*));

	for(i=0, j= 0; d[i].d_tag != DT_NULL; i++) {
		switch(d[i].d_tag) {
			case DT_NEEDED:{
				struct filp*fp;
				needed_offset = d[i].d_un.d_ptr;
				sprintf(path, "/system/lib/%s", STRING(img, needed_offset));

				/*try open if it loaded into mem*/
				fp = server_open(path, "r");

				if (fp == NIL_FLP)	{

					kprintf("load_dependencies open error %s\n",path);
					
					sprintf(path, "%s/%s",basepath?basepath:"/tmp", STRING(img, needed_offset));
					/*not in mem, try open it on disk*/
					fp = server_open(path, "r");	
				}
				else{
					trace("exec_file(): %s load on memory\n", path);
				}

				if(fp==NIL_FLP){
					/*file not exist*/
					kprintf("load library %s error\n", path);
					return ENOENT;
				}
				//kprintf("load library %s\n", path);

			
				img->needed[j]= load_container(rp,fp, path, STRING(img, needed_offset), false,&bss);
				j+= 1;
				user_malloc(rp,bss,0);

				fclose(fp);

				break;
			}
			default:
				/*
				 * ignore any other tag
				 */
				continue;
		}
	}

	FATAL((j!= img->num_needed), -1,"Internal error at load_dependencies()");

	return 0;
}

static
unsigned
topological_sort(image_t *img, unsigned slot, image_t **init_list)
{
	unsigned i;

	img->flags|= RFLAG_SORTED; /* make sure we don't visit this one */
	for(i= 0; i< img->num_needed; i++) {
		if(!(img->needed[i]->flags & RFLAG_SORTED)) {
			slot= topological_sort(img->needed[i], slot, init_list);
		}
	}

	init_list[slot]= img;
	return slot+1;
}


int
get_dependencies(image_t *img, bool init_head, void **arg)
{
	unsigned i;
	unsigned slot;
	image_t **init_list;

	init_list= kmalloc(255*sizeof(image_t*),0);
	FATAL((!init_list), -1,"memory shortage in init_dependencies()");
	memset(init_list, 0, 255*sizeof(image_t*));

	img->flags|= RFLAG_SORTED; /* make sure we don't visit this one */
	slot= 0;
	for(i= 0; i< img->num_needed; i++) {
		if(!(img->needed[i]->flags & RFLAG_SORTED)) {
			slot= topological_sort(img->needed[i], slot, init_list);
		}
	}

	if(init_head) {
		init_list[slot]= img;
		slot+= 1;
	}


	for(i= 0; i< slot; i++) {
		addr_t _initf= init_list[i]->entry_point;
		libinit_f *initf= (libinit_f *)(_initf);
		arg[i] = _initf;
	//kprintf("init_dependencies slot = %d, name=%s, %p\n", i,init_list[i]->name, _initf);
#if 0
		if(initf) {
			initf(init_list[i]->imageid, NULL);  //FIXME!!!
		}
#endif
	}

	kfree(init_list);
	return slot;
}

