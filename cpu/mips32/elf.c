/*
 **     (R)Jicama OS
**      ELF Executable Format Support
**     Copyright (C) 2003 DengPingPing
*/


#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/coff.h>
#include <jicama/elf.h>
#include <jicama/module.h>
#include <string.h>

#define ELF32_DEBUG


exec_t elf32_check( elf_file_t *f )
{
	/*ELF32 Magic number check*/
	if (
		(f->ident[EI_MAG0] != ELFMAG0) ||
		(f->ident[EI_MAG1] != ELFMAG1) ||
		(f->ident[EI_MAG2] != ELFMAG2) ||
		(f->ident[EI_MAG3] != ELFMAG3)
	)
		return NO_EXEC;

	/*ELF Class check	*/
	if ( f->ident[EI_CLASS] != ELF_CLASS32 ){
		//kprintf("elf Class err!\n");
		return NO_EXEC;
	}

	/* Data encoding check*/
	if ( f->ident[EI_DATA] != ELFDATA2LSB ){
		kprintf("elf Data encoding err!\n");
		return NO_EXEC;
	}

	if(f->machine != EM_MIPS)
	{
		kprintf("File is not arm ELF\n");
		return NO_EXEC;
	}

	return ELF;
}



void copy_section(proc_t *rp, elf_file_t *f, elf_sect_t *sec)
{  
	// Skip NULL sections.
	if( (sec->virt_adr==0) || (sec->offset==0))return;

	// Copy this section into the current_proc() address space.
	memcpy( (void *)(proc_vir2phys(rp,sec->virt_adr)),(void *)((u32_t)f + sec->offset),sec->size );
}





__local int 
get_elf_sym(unsigned char *image, unsigned i,
		unsigned *sym_val, unsigned symtab_sect)
{
	elf_file_t *file;
	elf_sect_t *sect;
	elf_sym_t *sym;
	char *sym_name;
	unsigned adr;
	int err = 0;

	//kprintf("symtab_sect=%d\n", symtab_sect);

	/* point to symbol table */
	file = (elf_file_t *)image;
	if(symtab_sect >= file->num_sects)
	{
		kprintf("bad symbol table section number %d (max %u)\n",
			symtab_sect, file->num_sects - 1);
		return EFBIG;
	}
	sect = (elf_sect_t *)(image + file->shtab_offset +
		file->shtab_ent_size * symtab_sect);
	
	if(i >= sect->size)
	{
		/* get symbol */
		kprintf("offset into symbol table (%u) exceeds symbol table size (%lu)\n", i, sect->size);
		return EFBIG;
	}

	sym = (elf_sym_t *)(image + sect->offset) + i;
	
	if(sym->section == 0)
	{
		/* external symbol  point to string table for this symbol table */
		sect = (elf_sect_t *)(image + file->shtab_offset +
			file->shtab_ent_size * sect->link);

		/* get symbol name */
		sym_name = (char *)image + sect->offset + sym->name;

		err = lookup_kernel_symbol(sym_name, sym_val, 0);
		if(err != 0){
			kprintf("lookup_kernel_symbol(): %s not found!\n", sym_name);
			return err;
		}
	}
	else{
		/* internal symbol */
		sect = (elf_sect_t *)(image + file->shtab_offset +
			file->shtab_ent_size * sym->section);

		adr = (unsigned)image + sect->offset;
		*sym_val = sym->value + adr;
	}

	return 0;
}

__local int do_elf_reloc(unsigned char *image, elf32_rela_t *reloc,
		elf_sect_t *sect)
{
	unsigned t_adr, sym_val;
	elf_sect_t *t_sect;
	elf_file_t *file;
	u32_t *reloc_addr;
	int err;

	/* get address of target section */
	file = (elf_file_t *)image;

	t_sect = (elf_sect_t *)(image + file->shtab_offset +
		file->shtab_ent_size * sect->info);

	t_adr = (unsigned)image + t_sect->offset;

	/* point to relocation */
	reloc_addr = (u32_t *)(t_adr + reloc->r_offset);

	/* get symbol */
	err = get_elf_sym(image, ELF32_R_SYM(reloc->r_info),
		&sym_val, sect->link);
	if(err != 0)return err;

	switch(ELF32_R_TYPE(reloc->r_info))
	{
	case R_MIPS_REL32:
	case R_MIPS_NONE:
		break;
	
	default:
		kprintf("unsupported relocation type %u\n", 
		ELF32_R_TYPE(reloc->r_info));
		return -1;
	}
	return 0;
}

 
int load_elf_relocatable(unsigned char *image, kern_module_t *mod)
{
	unsigned i, j, reloc_size;
	unsigned char *bss;
	elf32_rela_t *reloc;
	elf_sect_t *sect;
	elf_file_t *file;
	int err = 0;
	unsigned start_point;

	/* validate */
	file = (elf_file_t *)image;

	if ((file->ident[EI_MAG0] != ELFMAG0) ||
		(file->ident[EI_MAG1] != ELFMAG1) ||
		(file->ident[EI_MAG2] != ELFMAG2) ||
		(file->ident[EI_MAG3] != ELFMAG3)){
		kprintf("Module(Magic 0x%x) Not Relocatable ELF!\n", (long)file->ident);
		return ENOEXEC;
	}

	if(file->ident[EI_CLASS]  != ELF_CLASS32)
	{
		kprintf("File is 64-bit ELF, not 32-bit\n");
		return ENOEXEC;
	}

	if(file->ident[EI_DATA]  != ELFDATA2LSB)
	{
		kprintf("File is big endian ELF, not little\n");
		return ENOEXEC;
	}

	if(file->ident[EI_VERSION]  != 1)
	{
		kprintf("File has bad ELF version %u\n", file->ident[EI_VERSION]);
		return ENOEXEC;
	}

	if(file->file_type != ET_REL)
	{
		kprintf("File is not relocatable ELF (could be executable, DLL, or core file)\n");
		return ENOEXEC;
	}

	if(file->elf_ver_2 != EV_CURRENT)
	{
		kprintf("File has bad ELF version %lu\n", file->elf_ver_2);
		return ENOEXEC;
	}

	if(file->machine != EM_MIPS)
	{
		kprintf("File is not arm ELF 0x%x\n",file->machine);
		return ENOEXEC;
	}

	kprintf("find bbs ....\n");

	/* find the BSS and allocate memory for it
	This must be done BEFORE doing any relocations */
	for(i = 0; i < file->num_sects; i++)
	{
		sect = (elf_sect_t *)(image + file->shtab_offset +
				file->shtab_ent_size * i);

		if(sect->type != SHT_NOBITS)continue;	/* NOBITS */

		j = sect->size;
		bss = (unsigned char *)kcalloc(j);

		mod->bss_size =j;
		mod->bss = bss;

		if(bss == NULL)
		{
			kprintf("Can't allocate %u bytes for BSS\n",j);
			return ENOEXEC;
		}

		sect->offset = bss - image;
		break;
	}


	/* for each section... */
	for(i = 0; i < file->num_sects; i++)
	{
		sect = (elf_sect_t *)(image + file->shtab_offset +
				file->shtab_ent_size * i);

		if(sect->type == SHT_RELA)	{/* RELA */
			//kprintf("SHT_RELA %d\n", sect->size);
			reloc_size = sizeof(elf32_rela_t);//12
		}
		else if(sect->type == SHT_REL){/* REL */
			//kprintf("SHT_REL %d\n",sect->size);
			reloc_size = sizeof( elf32_rel_t); //8
		}
		else
			continue;

	/* for each relocation... */
		for(j = 0; j < sect->size / reloc_size; j++)
		{
			int res;
			reloc = (elf32_rela_t *)(image + sect->offset +reloc_size * j);

			res = do_elf_reloc(image, reloc, sect);
			if (res)
			{
				err = res;
			}
		}
	}

	if (err){
		return err;
	}

	//Ñ°ÕÒÖ÷Òª·ûºÅ
	err = get_main_elf_sym(image, mod);
	if(err != 0)return err;

	/* find start of .text and make it the entry point */
	(start_point) = 0;
	for(i = 0; i < file->num_sects; i++)
	{
		sect = (elf_sect_t *)(image + file->shtab_offset + file->shtab_ent_size * i);

		if((sect->flags & 0x0004) == 0)continue;		
		(start_point) = (unsigned)image + sect->offset;
		break;
	}


	if((start_point) == 0)
	{
		kprintf("Can't find section .text, so entry point is unknown\n");
		return ENOEXEC;
	}
	else{
		relocatable_setup(&mod->opaddr, start_point);
	}

	kprintf("start_point @ 0x%x -0x%x....\n", 
		(u32_t)start_point,mod->opaddr.dll_main);

	return 0;
}




