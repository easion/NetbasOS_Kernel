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
#include <jicama/paging.h>

//#define ELF32_DEBUG


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
		syslog(LOG_WARNING,"elf Data encoding err!\n");
		return NO_EXEC;
	}

	if(f->machine != EM_386)
	{
		syslog(LOG_WARNING,"File is not i386 ELF\n");
		return NO_EXEC;
	}
	//kprintf("(f->flags = %x\n", f->flags);

	if (f->flags&HAS_SYMS) //头部标志
	{
		kprintf("File is  relocatable, DLL, or core file: %x\n",f->flags);
		return ELF_USER_SO;
	}	
	return ELF;
}

/*exec file format*/
exec_t exec_get_format(void *image,struct filp*fp)
{
	exec_t format=NO_EXEC;

	if (fp)
	{
		fseek(fp, 0, 0);
		format = elf_check_so(fp);
		fseek(fp, 0, 0);
	}

	if(format!=NO_EXEC)
		return format;

	format=elf32_check((elf_file_t *)image);

	if(format!=NO_EXEC)
		return format;

	format=coff_check((const struct coff_hdr*)image);

	if((format)!=NO_EXEC)
		return format;

	format=pe_check((char *)image);

	if(format!=NO_EXEC){
		return format;
	}

	return NO_EXEC;
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
	u32_t end_code=0;
	proc_t *rp = THREAD_SPACE(pthread);
	int nbytes_read;

	fseek(fp,0,0);

	nbytes_read=fread((void*)&header, sizeof(elf_file_t), 1, fp);

	if (nbytes_read!=sizeof(elf_file_t))
	{
		kprintf("read (elf_file_t)  error %d\n",nbytes_read);
		//return ENOEXEC;
	}

	if(header.file_type != ET_EXEC)
	{
		kprintf("File is not executable  ELF (%d could be relocatable, DLL, or core file)\n",header.file_type);
		return ENOEXEC;
	}
	//kprintf("(f->flags = %x\n", header.flags);

	if (header.flags&HAS_SYMS) //头部标志
	{
		kprintf("File is  relocatable, DLL, or core file\n",header.flags);
		return ENOEXEC;
	}

	*entry= header.entry_pt;

	
	//build_user_ldt_space(rp);

	if(header.num_phtab_ents==0){
		kprintf("ENOEXEC header.num_phtab_ents=%d\n", header.num_phtab_ents);
		return ENOEXEC;
	}

	trace("header.num_phtab_ents=%d ok\n", header.num_phtab_ents);

	for ( i = 0; i < header.num_phtab_ents; i++) 
	{	  	
	u8_t *addr_ptr ;


	fseek(fp, header.phtab_offset + ( header.phtab_ent_size * i), SEEK_SET);	
	nbytes_read = fread((void*)&phdrdr, sizeof(elfphdr_t), 1, fp);

	if (nbytes_read!=sizeof(elfphdr_t))
	{
		kprintf("read (elfphdr_t)  error %d\n",nbytes_read);
		//return ENOEXEC;
	}

	addr_ptr = (u8_t *)proc_vir2phys(rp,phdrdr.vaddr);
	if (phdrdr.type != PT_LOAD) {
		continue;
	}

	if(phdrdr.memsz == 0){
		continue;
	}




#ifdef ELF32_DEBUG
	 switch (phdrdr.type)
	 {
		 case PT_NULL:kprintf(" null");break;
		 case PT_LOAD:kprintf(" PT_LOAD");break;
		 case PT_DYNAMIC:kprintf(" PT_DYNAMIC");break;
		 case PT_INTERP:kprintf(" PT_INTERP");break;
		 case PT_NOTE:kprintf(" PT_NOTE");break;
		 case PT_SHLIB:kprintf(" PT_SHLIB");break;
		 case PT_PHDR:kprintf(" PT_PHDR");break;
		 case PT_LOPROC:kprintf(" PT_LOPROC");break;
		 case PT_HIPROC:kprintf(" PT_HIPROC");break;
		 default :kprintf(" ???");break;
	 }
	
	kprintf("Physical[offset:%x -%x] Vaddr[%x-%x]\n",
	phdrdr.paddr,
	phdrdr.paddr + phdrdr.filesz,
	phdrdr.vaddr, phdrdr.vaddr + phdrdr.memsz);
	kprintf("proc addr 0x%x - text %d\n", (long)addr_ptr, phdrdr.offset);
#endif

		size = phdrdr.filesz;

		fseek(fp,  phdrdr.offset, SEEK_SET);
		
		//kprintf("%s(): memsz %x\n", __FUNCTION__, phdrdr.memsz);
		nbytes_read = fread(addr_ptr, size,1, fp);
		if (nbytes_read!=size)
		{
			kprintf("read error %d\n",nbytes_read);
			//return ENOEXEC;
		}


		if (phdrdr.flags == (PF_R | PF_X))
		{
			//代码段
		}
		else{
			//数据bss段
		}
		end_code =MAX(end_code, (phdrdr.vaddr+phdrdr.memsz)); 
	

		if (phdrdr.type  > 0x6 ){
			kprintf("error type %d\n",phdrdr.type);
			break;
		}
	}

	rp->start_stack = USER_STACK_ADDR_END&0xfffff000; /*argc and argv space*/
	
		user_heap_init(rp,USER_MMAP_ADDR, USER_MMAP_SIZE);
	
	trace("%s(): end_code at %x\n", __FUNCTION__, end_code);

	rp->p_bss_code = pageup(end_code);
	 return OK;
}

int elf_check_so(struct filp*  fp)
{
	int i, size=0;
	elfphdr_t phdrdr;
	elf_file_t header;

	fseek(fp,0,0);
	fread((void*)&header, sizeof(elf_file_t), 1, fp);

	if (
		(header.ident[EI_MAG0] != ELFMAG0) ||
		(header.ident[EI_MAG1] != ELFMAG1) ||
		(header.ident[EI_MAG2] != ELFMAG2) ||
		(header.ident[EI_MAG3] != ELFMAG3)
	)
		return NO_EXEC;


	if(header.file_type != ET_EXEC)
	{
		kprintf("File is not executable  ELF (%d could be relocatable, DLL, or core file)\n",header.file_type);
		return NO_EXEC;
	}

	if(header.num_phtab_ents==0){
		kprintf("header.num_phtab_ents=%d\n", header.num_phtab_ents);
		return NO_EXEC;
	}

	for ( i = 0; i < header.num_phtab_ents; i++) 
	{	  	  
		fseek(fp, header.phtab_offset + ( header.phtab_ent_size * i), SEEK_SET);	
		fread((void*)&phdrdr, sizeof(elfphdr_t), 1, fp);

		if (phdrdr.type == PT_LOAD) {
			continue;
		}
		else if (phdrdr.type == PT_DYNAMIC)
		{
			//kprintf("PT_DYNAMIC not supported\n");
			return ELF_USER_SO;
		} 
		 if (phdrdr.type  > 0x6 ) break;
	}

	
	 return NO_EXEC;
}



