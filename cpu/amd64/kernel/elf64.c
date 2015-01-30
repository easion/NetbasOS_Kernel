

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/grub.h>
#include <jicama/elf.h>

void dump_elf_section(elf64_sect_t *sec)
{
		kprintf("SECT-name      = %s\n", sec->sect_name);
		kprintf("SECT-type      = %x\n", sec->type);
		kprintf("SECT-flags     = %x\n", sec->flags);
		kprintf("SECT-addr      = %x\n", sec->virt_adr);
		kprintf("SECT-offset    = %x\n", sec->offset);
		kprintf("SECT-size      = %x\n", sec->size);
		kprintf("SECT-link      = %x\n", sec->link);
		kprintf("SECT-info      = %x\n", sec->info);
		kprintf("SECT-addralign = %x\n", sec->align);
		kprintf("SECT-entsize   = %x\n", sec->ent_size);
}

void dump_elf64_hdr(elf64_file_t *header)
{
		kprintf("ELF File header:%s\n",header->ident);

		kprintf("magic[]=%c%c%c%c class=%u data=%u\n",
			header->ident[ELFMAG0],			header->ident[ELFMAG1],
			header->ident[ELFMAG2],			header->ident[ELFMAG3],
			header->ident[EI_CLASS],			header->ident[EI_DATA]
		);

	kprintf("ELF machine =%d\n", header->machine);
	kprintf("ELF elf_ver_2 =%d\n", header->elf_ver_2);
	kprintf("ELF entry_pt =%x\n", header->entry_pt);
	kprintf("ELF phtab_offset =%x\n", header->phtab_offset);
	kprintf("ELF shtab_offset =%x\n", header->shtab_offset);
	kprintf("ELF flags =%x\n", header->flags);
	kprintf("ELF file_hdr_size =%d\n", header->file_hdr_size);
	kprintf("ELF phtab_ent_size =%d\n", header->phtab_ent_size);
	kprintf("ELF num_phtab_ents =%d\n", header->num_phtab_ents);
	kprintf("ELF shtab_ent_size =%d\n", header->shtab_ent_size);
	kprintf("ELF num_sects =%d\n", header->num_sects);
	kprintf("ELF shstrtab_index =%d\n", header->shstrtab_index);

}

