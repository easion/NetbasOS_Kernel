
#include <jicama/system.h>
#include <jicama/pe.h>
#include <jicama/coff.h>
#include <jicama/module.h>
#include <string.h>


int get_coff_sym(unsigned char *image, unsigned i,
		unsigned *sym_val, unsigned *sect_num);

static int 
do_pecoff_reloc(unsigned char *image,struct coff_section *sect, unsigned r)
{
	unsigned sym_sect, sym_val, t_adr = 0;
	coff_reloc_t *reloc;
	struct coff_hdr *file;
	uint32_t *where;
	int r_delta, err;

	/* r_delta = delta (i.e. LMA - VMA) for section containing this relocation */
	r_delta = (int)image + sect->s_raw_data_addr - sect->s_vis_run_addr;

	/* point to relocation */
	reloc = (coff_reloc_t *)(image + sect->s_relptr) + r;
	where = (uint32_t *)(reloc->adr + r_delta);
/* get symbol */
	err = get_coff_sym(image, reloc->symtab_index, &sym_val, &sym_sect);
	if(err != 0)
		return err;
	/* t_adr = address of target section (section in symbol table entry) */
	file = (struct coff_hdr *)image;
	if(sym_sect != 0) /* internal symbol */
	{
		sect = (struct coff_section *)(image + sizeof(struct coff_hdr) +
			file->f_opthdr) + (sym_sect - 1);
		t_adr = (unsigned)image + sect->s_raw_data_addr;
	}
	switch(reloc->type)
	{
	/* absolute reference
	COFF.H calls this "RELOC_ADDR32"; objdump calls it "dir32" */
	case 6:
		if(sym_sect == 0) /* external symbol */
			*where = sym_val;
		else
			*where += (sym_val + t_adr);
		break;
	/* EIP-relative reference
	COFF.H calls this "RELOC_REL32"; objdump calls it "DISP32" */
	case 20:
		if(sym_sect == 0) /* external symbol */
	/* xxx - 4 = width of relocated item, in bytes. Why should this code
	have any knowledge of that? Maybe I'm doing something wrong... */
			*where = sym_val - ((unsigned)where + 4);
		else
			*where += (sym_val + t_adr - ((unsigned)where + 4));
		break;
	default:
		kprintf("unknown relocation type %u (must be 6 or 20)\n",
			reloc->type);
		return -1;
	}
	return 0;
}

 int get_main_coff_sym(void *image, kern_module_t *mod);

int 
load_pecoff_relocatable(unsigned char *image, kern_module_t *mod)
{
	unsigned start_point=0;
	unsigned char *bss;
	struct coff_section *sect;
	struct coff_hdr *file;
	unsigned s, r;
	int err;
	pehdr_t *pedll=(pehdr_t *)image;;
	if(pedll->magic[0] != 'M' ||
		pedll->magic[1] != 'Z' ||		/* not .EXE */
		pedll->hdr_size < 4 ||
		pedll->new_exe_offset == 0){	/* not new-style .EXE */
		kprintf("not new-style .EXE \n");
		return 1;
	}

	if (!(image[pedll->new_exe_offset+0]=='P'
		&&image[pedll->new_exe_offset+1]=='E'
		&&image[pedll->new_exe_offset+2]==0
		&&image[pedll->new_exe_offset+3]==0
		))
	{
		kprintf("NO PE SIG Found\n");
		return -3;
	}

	kprintf("hdr_size=%d %s\n", pedll->hdr_size, (char*)image+(pedll->new_exe_offset));
	kprintf("new_exe_offset at 0x%x magic%x\n", pedll->new_exe_offset,
		*(u16_t*)(image+pedll->new_exe_offset+4));

	image+=pedll->new_exe_offset+4;

/* validate */
	file = (struct coff_hdr *)(image);
	if(file->f_magic != 0x14C)
	{
		kprintf("File is not relocatable COFF; has bad magic value "
			"0x%X (should be 0x14C)\n", file->f_magic);
		return +1;
	}
	if(file->f_flag & 0x0001)
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
		r = sect->s_paddr;//sect->size;
		bss = (unsigned char *)kcalloc(r);
		if(bss == NULL)
		{
			kprintf("Can't allocate %u bytes for BSS\n",
				r);
			return -1;
		}
		sect->s_raw_data_addr = bss - image;
		break;
	}
	err=get_main_coff_sym(image, mod);
	if(err != 0)return err;

	kprintf("f_num_sections is %s\n", file->f_num_sections);

	/* for each section... */
	for(s = 0; s < file->f_num_sections; s++)
	{
		sect = (struct coff_section *)(image + sizeof(struct coff_hdr) +
			file->f_opthdr) + s;
		kprintf("sect name is %s\n", sect->s_name);
	/* for each relocation... */
		for(r = 0; r < sect->s_nreloc; r++)
		{
			err = do_pecoff_reloc(image, sect, r);
			if(err != 0)
				return err;
		}
		sect++;
	}


	/* find start of .text and make it the entry point */
	(start_point) = 0;
	for(s = 0; s < file->f_num_sections; s++)
	{
		sect = (struct coff_section *)(image + sizeof(struct coff_hdr) +
			file->f_opthdr) + s;
	
		if(strcmp(sect->s_name, ".text"))
			continue;
		(start_point) = (unsigned)image + sect->s_raw_data_addr;
		break;
	}
	if((start_point) == 0)
	{
		kprintf("Can't find section .text, so entry point is unknown\n");
		return -1;
	}

	relocatable_setup(&mod->opaddr, start_point);
	kprintf("pe dll loader ok\n");
	return 0;
}
