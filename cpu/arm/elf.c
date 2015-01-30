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

	if(f->machine != EM_ARM)
	{
		kprintf("File is not arm ELF\n");
		return NO_EXEC;
	}


	if (f->flags&HAS_SYMS) //头部标志
	{
		kprintf("File is  relocatable, DLL, or core file: %x\n",f->flags);
		return ELF_USER_SO;
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



int read_elf_static(struct filp*  fp,  thread_t *pthread, unsigned *entry)
{
	int i, size=0;
	elfphdr_t phdrdr;
	elf_file_t header;
	u32_t end_code;
	proc_t *rp = THREAD_SPACE(pthread);

	fseek(fp,0,0);

	fread((void*)&header, sizeof(elf_file_t), 1, fp);

	if(header.file_type != ET_EXEC)
	{
		kprintf("File is not executable  ELF (%d could be relocatable, DLL, or core file)\n",header.file_type);
		return -1;
	}

	*entry= header.entry_pt;

	if(header.num_phtab_ents==0){
		kprintf("header.num_phtab_ents=%d\n", header.num_phtab_ents);
		return -1;
	}


	for ( i = 0; i < header.num_phtab_ents; i++) 
	{	  	  
	u8_t *addr_ptr ;

	fseek(fp, header.phtab_offset + ( header.phtab_ent_size * i), SEEK_SET);
	
	fread((void*)&phdrdr, sizeof(elfphdr_t), 1, fp);

	addr_ptr = (u8_t *)proc_vir2phys(rp,phdrdr.vaddr);
#ifdef ELF32_DEBUG
	 switch (phdrdr.type)
	 {
		 case PT_NULL:trace(" null");break;
		 case PT_LOAD:trace(" PT_LOAD");break;
		 case PT_DYNAMIC:trace(" PT_DYNAMIC");break;
		 case PT_INTERP:trace(" PT_INTERP");break;
		 case PT_NOTE:trace(" PT_NOTE");break;
		 case PT_SHLIB:trace(" PT_SHLIB");break;
		 case PT_PHDR:trace(" PT_PHDR");break;
		 case PT_LOPROC:trace(" PT_LOPROC");break;
		 case PT_HIPROC:trace(" PT_HIPROC");break;
		 default :trace(" ???");break;
	 }
	
	trace("Physical[offset:%x -%x] Vaddr[%x-%x]\n",
	phdrdr.paddr,
	phdrdr.paddr + phdrdr.filesz,
	phdrdr.vaddr, phdrdr.vaddr + phdrdr.memsz);
#endif
	trace("proc addr 0x%x - text %d\n", (long)addr_ptr, phdrdr.offset);

	if (phdrdr.type == PT_LOAD) {
		size = phdrdr.filesz;
		end_code += phdrdr.memsz;

		fseek(fp,  phdrdr.offset, SEEK_SET);

		fread(addr_ptr, size,1, fp);
	}
	else if (phdrdr.type == PT_DYNAMIC)
	{
		kprintf("PT_DYNAMIC not supported\n");
	}
	else{
		continue;
	}
 
	end_code =MAX(end_code, (phdrdr.vaddr+phdrdr.memsz));
	
	if (phdrdr.type  > 0x6 ) break;
	}

	rp->start_stack = USER_STACK_ADDR_END&PT_PFNMASK; /*argc and argv space*/
	rp->p_bss_code = pageup(end_code);

	trace(" done ..xxx\n");

	//rp->brk=end_code;
	 return OK;
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
		return -1;
	}
	sect = (elf_sect_t *)(image + file->shtab_offset +
		file->shtab_ent_size * symtab_sect);
	
	if(i >= sect->size)
	{
		/* get symbol */
		kprintf("offset into symbol table (%u) exceeds symbol table size (%lu)\n", i, sect->size);
		return -1;
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
	/* EIP-relative reference"R_ARM_PC24" 一般用于外部符号*/
	case R_ARM_PC24: {
		unsigned long low_bits;
		long new_val;
		u8_t top_code = (*reloc_addr >> 24) & 0xff;

		low_bits = *reloc_addr & 0x00ffffff;
		if (low_bits & 0x00800000){
			low_bits |= 0xff000000;
		}

		//26 bits
		low_bits <<= 2;

		new_val = sym_val - (unsigned long)reloc_addr + low_bits;
#ifdef __DEBUG__
		//check it
		long top_bits = new_val & 0xfe000000;
		if (top_bits != 0xfe000000 && top_bits != 0x00000000)	{
			panic("R_ARM_PC24 out of range: 0x%x\n", top_bits);
		}
#endif
		new_val >>= 2;
		new_val &= 0x00ffffff; //low 24 bits

		*reloc_addr = (top_code << 24) | new_val;
		}
		break;

	/* absolute reference */
	case R_ARM_ABS32: {/* S + A 一般只用于内部符号 */
		*reloc_addr += sym_val;
		//kprintf("R_ARM_ABS32 = 0x%x \n",*reloc_addr);
		}
		break;

	case R_ARM_GLOB_DAT:
	case R_ARM_JUMP_SLOT:
		*reloc_addr = sym_val;
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
		return 1;
	}

	if(file->ident[EI_CLASS]  != ELF_CLASS32)
	{
		kprintf("File is 64-bit ELF, not 32-bit\n");
		return -1;
	}

	if(file->ident[EI_DATA]  != ELFDATA2LSB)
	{
		kprintf("File is big endian ELF, not little\n");
		return -1;
	}

	if(file->ident[EI_VERSION]  != 1)
	{
		kprintf("File has bad ELF version %u\n", file->ident[EI_VERSION]);
		return -1;
	}

	if(file->file_type != ET_REL)
	{
		kprintf("File is not relocatable ELF (could be executable, DLL, or core file)\n");
		return -1;
	}

	if(file->elf_ver_2 != EV_CURRENT)
	{
		kprintf("File has bad ELF version %lu\n", file->elf_ver_2);
		return -1;
	}

	if(file->machine != EM_ARM)
	{
		kprintf("File is not arm ELF 0x%x\n",file->machine);
		return -1;
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
			return -1;
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

	//寻找主要符号
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
		return -1;
	}
	else{
		relocatable_setup(&mod->opaddr, start_point);
	}

	kprintf("start_point @ 0x%x -0x%x....\n", 
		(u32_t)start_point,mod->opaddr.dll_main);

	kprintf("load ELF dll ok\n");
	return 0;
}




