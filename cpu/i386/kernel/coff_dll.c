/*
**     (R)Jicama OS
**     Program Args Setup
**     Copyright (C) 2005 DengPingPing
*/
#include <jicama/system.h>
#include <jicama/coff.h>
#include <jicama/pe.h>
#include <jicama/module.h>
#include <string.h>

//	#define __DLL_DEBUG__

struct driver_entry{
	int flags;
	char *sym_name;
	unsigned entry;
};


__local coff_sym_t *get_coff_sym_byname( void *image,  char *name )
{
	int  pos;
	char *sym_name, *strtab;
	struct coff_hdr *file;
	coff_sym_t *sym;
	
	file = (struct coff_hdr *)image;	

	for (pos=0; pos<(int)file->f_num_syms; pos++){

	sym = (coff_sym_t *)((long)image + file->f_sym_ptr) + pos;
	sym_name = sym->x.name;
	
	if(sym->x.x.zero == 0)
	{
		strtab = (char *)image + file->f_sym_ptr +
			file->f_num_syms * sizeof(coff_sym_t);
		sym_name = strtab + sym->x.x.strtab_index;
	}

	if((stricmp(name, sym_name)==0)){	
		return sym;
	}	
	else{
		//kprintf("%s|", sym_name);
	}

	}

	return (coff_sym_t *)0;
}

static void* get_coff_secton(kern_module_t *mod,const char *name, int *size)
{
	void *addr=NULL;;


	return addr;
}


__public  int get_main_coff_sym(void *image, kern_module_t *mod)
{
	int  pos;
	int i;
	char *sym_name, *strtab;
	struct coff_hdr *file;
	coff_sym_t *sym;
	u8_t __symf[NR_DLL_OPS];

	
	file = (struct coff_hdr *)image;
	memset(&mod->opaddr, 0, sizeof(mod->opaddr));

	bzero(__symf, NR_DLL_OPS);

	if (file->f_num_syms == 0)
	{
		pe_aouthdr_t *peaout = (pe_aouthdr_t*)((char*)(image)+sizeof(struct coff_hdr));
		mod->opaddr.dll_main=peaout->entry;
		kprintf("entry at %x\n", peaout->entry);
		return OK;
	}

	kprintf("sym num is %d\n",file->f_num_syms);

	for (pos=0; pos<(int)file->f_num_syms; pos++){

	sym = (coff_sym_t *)((long)image + file->f_sym_ptr) + pos;
	sym_name = sym->x.name;

	if (sym->sym_class!=EXTERNAL_CLASS)continue; /*skip all other class  type*/
	
	if(sym->x.x.zero == 0)
	{
		strtab = (char *)image + file->f_sym_ptr +
			file->f_num_syms * sizeof(coff_sym_t);
		sym_name = strtab + sym->x.x.strtab_index;
	}

	kprintf("%s  Found, Class %d, Address %x!\n",
		sym_name, sym->sym_class, sym->val);
	if((strcmp("_dll_main", sym_name)==0)
		||(strcmp("_DllMain", sym_name)==0)){	
		//kprintf("%s  Found, Class %d, Address %x!\n",
		//sym_name, sym->sym_class, sym->val);
		mod->opaddr.dll_main=sym->val;
		__symf[0]++;
	}
	else if(strcmp("_dll_destroy", sym_name)==0
		||strcmp("dll_destroy", sym_name)==0){		
		//kprintf("%s  Found, Class %d, Address %x!\n", sym_name, sym->sym_class, sym->val);
		mod->opaddr.dll_destroy=sym->val;
		__symf[1]++;
	}
	
	else if(strcmp("__static_initialization_and_destruction_0", sym_name)==0
		||strcmp("___static_initialization_and_destruction_0", sym_name)==0){		
		//kprintf("%s  Found, Class %d, Address %x!\n", sym_name, sym->sym_class, sym->val);
		mod->opaddr.dll_static_init=sym->val;
		__symf[3]++;
	}
	else{
		//kprintf("%s|", sym_name);
	}

	}

	mod->opaddr.dll_find_secton=&get_coff_secton;

	for (i=0; i<NR_OK_OPS-1; i++)
	{
		if (__symf[i]!=1){
		kprintf("sym %s error(defined %d times)\n", get_module_sym_name(i), __symf[i]);
		return -1;
		}
	}

	return OK;
}


__public int get_coff_sym(unsigned char *image, unsigned pos,
		unsigned *sym_val, unsigned *sect_num)
{
	char *sym_name, *strtab;
	struct coff_hdr *file;
	coff_sym_t *sym;
	int err;

	file = (struct coff_hdr *)image;

	/* number of symbol table entries */
	if(pos >= file->f_num_syms)
	{
		kprintf("index into symbol table (%d) is too large "
			"(%d max)\n", pos, file->f_num_syms);
		return ENOEXEC;
	}

	/* point to symtab entry, get name */
	sym = (coff_sym_t *)(image + file->f_sym_ptr) + pos;
	sym_name = sym->x.name;

	if(sym->x.x.zero == 0)
	{
		strtab = (char *)image + file->f_sym_ptr +
			file->f_num_syms * sizeof(coff_sym_t);
		sym_name = strtab + sym->x.x.strtab_index;
	}
	/* get section and check it */
	if(sym->sect_num > file->f_num_sections)
	{
		kprintf("symbol '%s' has bad section %d (max %u)\n",
			sym_name, sym->sect_num, file->f_num_sections);
		return ENOEXEC;
	}
	*sect_num = sym->sect_num;

	#ifdef __DLL_DEBUG__
	kprintf("sym: %s\n", sym_name);
	#endif

	if(*sect_num == 0)
	{/* external symbol */
		err = lookup_kernel_symbol(sym_name, sym_val, 1);
		if(err != 0)
			return err;
	}
	else{/* internal symbol */
		*sym_val = sym->val;
	}
	return 0;
}

__local int do_djcoff_reloc(unsigned char *image,
		struct coff_section *sect, unsigned r)
{
	int err, r_delta, t_delta = 0;
	unsigned  sym_val, sym_sect;
	coff_reloc_t *reloc;
	struct coff_hdr *file;
	uint32_t *where;

	/* r_delta = delta (i.e. LMA - VMA) for section containing this relocation */
	r_delta = (unsigned)image + sect->s_raw_data_addr - sect->s_vis_run_addr;

	/* point to the item to be relocated */
	reloc = (coff_reloc_t *)(image + sect->s_relptr) + r;
	where = (uint32_t *)(reloc->adr + r_delta);

	/* get symbol */
	err = get_coff_sym(image, reloc->symtab_index, &sym_val, &sym_sect);
	if(err != 0)return err;

	/* t_delta = delta for target section (section in symbol table entry) */
	file = (struct coff_hdr *)image;

	if(sym_sect != 0) /* internal symbol */
	{
		sect = (struct coff_section *)(image + sizeof(struct coff_hdr) +
			file->f_opthdr) + (sym_sect - 1);
		t_delta = (unsigned)image + sect->s_raw_data_addr - sect->s_vis_run_addr;
	}

	switch(reloc->type)
	{
	/* absolute reference
	COFF.H calls this "RELOC_ADDR32"; objdump calls it "dir32" */
	case RELOC_ADDR32:
		if(sym_sect == 0) /* external symbol */
			*where = sym_val;
		else
			*where += t_delta;
		break;

	/* EIP-relative reference
	COFF.H calls this "RELOC_REL32"; objdump calls it "DISP32" */
	case RELOC_REL32:
		if(sym_sect == 0) /* external symbol */
			*where += sym_val - r_delta;
		/* type 20 relocation for internal symbol
		does not occur with normal DJGPP code */
		else
			*where += t_delta - r_delta;
		break;

	default:
		kprintf("unknown relocation type %d (must be 6 or 20)\n",	reloc->type);
		return -1;
	}
	return 0;
}

__public int load_djcoff_relocatable(unsigned char *image,  kern_module_t *mod)
{
	unsigned start_point;
	unsigned char *bss;
	struct coff_section *sect;
	struct coff_hdr *file;
	unsigned s, r;
	int err;

	/* validate */
	file = (struct coff_hdr *)image;

	if(file->f_magic != I386MAGIC)
	{
		panic("load_djcoff_relocatable():Module %x is not COFF,"
		"Magic(%x)\n", (u32_t)image, file->f_magic);
		return ENOEXEC;
	}

	if(file->f_flag & F_RELFLG)
	{
		kprintf("Relocations have been stripped from COFF file\n");
		return +1;
	}

	/* find the BSS and allocate memory for it
	This must be done BEFORE doing any relocations */
	for(s = 0; s < file->f_num_sections; s++)
	{
		sect = (struct coff_section *)(image + sizeof(struct coff_hdr) +
			file->f_opthdr) + s;
		if((sect->s_flags & 0xE0) != 0x80)
			continue;
		r = sect->s_raw_size;
		bss = (unsigned char *)kcalloc(r);

		mod->bss = bss;
		mod->bss_size = r;

		//kprintf(" bss point is %x\n", bss);
		if(bss == NULL)
		{
			kprintf("Can't allocate %u bytes for BSS\n",r);
			return ENOEXEC;
		}

		sect->s_raw_data_addr = bss - image;
		break;
	}


	/* for each section... */
	for(s = 0; s < file->f_num_sections; s++)
	{
		sect = (struct coff_section *)(image + sizeof(struct coff_hdr) +
			file->f_opthdr) + s;
	/* for each relocation... */
		for(r = 0; r < sect->s_nreloc; r++)
		{
			err = do_djcoff_reloc(image, sect, r);
			if(err != 0)
				return err;
		}
		sect++;
	}

	err=get_main_coff_sym(image, mod);
	if(err != 0)return err;

	/* find start of .text and make it the entry point */
	start_point = 0;

	for(s = 0; s < file->f_num_sections; s++)
	{
		sect = (struct coff_section *)(image + sizeof(struct coff_hdr) +
			file->f_opthdr) + s;
		if(strcmp(sect->s_name, ".text"))
			continue;
		start_point = (unsigned )image + sect->s_raw_data_addr;
		break;
	}

	if(start_point == 0)
	{
		kprintf("Can't find section .text, so entry point is unknown\n");
		return ENOEXEC;
	}

	#ifdef __DLL_DEBUG__
		kprintf(" entry point is %x\n", mod->opaddr.dll_start);

	for(s = 0; s < file->f_num_sections; s++)
	{
		sect = (struct coff_section *)(image + sizeof(struct coff_hdr) +
			file->f_opthdr) + s;
		kprintf("%s: %d bytes at s_raw_data_addr=0x%x, LMA=0x%x\n",
			sect->s_name, sect->s_raw_size, sect->s_raw_data_addr,
			image + sect->s_raw_data_addr);
	}
	#endif

	relocatable_setup(&mod->opaddr, start_point);
	return (OK);
}

