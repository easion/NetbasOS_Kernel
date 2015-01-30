/*
 **     (R)Jicama OS
**      ELF Executable Format Support
**     Copyright (C) 2003 DengPingPing
*/


#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/elf.h>
#include <jicama/module.h>
#include <string.h>
#include <assert.h>


void dump_elf_section(elf_sect_t *sec)
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

void dump_elf_hdr(elf_file_t *header)
{
	kprintf("ELF File header:\n");
	
	kprintf("magic[]=%c%c%c%c class=%u data=%u\n",
		header->ident[ELFMAG0],
		header->ident[ELFMAG1],
		header->ident[ELFMAG2],
		header->ident[ELFMAG3],
		header->ident[EI_CLASS],
		header->ident[EI_DATA]
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

char *get_elf_sect_name_by_index( elf_file_t *elf32_hdr_ptr, u32_t index )
{
    char *name;
    elf_sect_t *elf32_sheader;
	
	elf32_sheader = (elf_sect_t *)((u32_t)elf32_hdr_ptr + (u32_t)elf32_hdr_ptr->shtab_offset);

    elf32_sheader += (elf32_hdr_ptr->shstrtab_index);
    name = (char *)((u32_t)elf32_hdr_ptr +(u32_t)elf32_sheader->offset + index);
    return name;
}


int get_elf_sect_index_by_name( elf_file_t *elf32_hdr_ptr, char *name) 
{
    elf_sect_t *elf32_sheader;
    char *str;
    int count = 0;
    int index = 0;

    elf32_sheader = (elf_sect_t *)((u32_t) elf32_hdr_ptr + (u32_t)elf32_hdr_ptr->shtab_offset );

    while ( count++ < elf32_hdr_ptr->num_sects )
	{
	str = get_elf_sect_name_by_index(elf32_hdr_ptr,
		elf32_sheader->sect_name);

	if ( str == NULL ) {
		ASSERT("str null");
		return -1;
		}
	if ( name == NULL ) {
		ASSERT("name null");
		return -1;
		}

	if ( strlen(str) == 0 ) {
	    elf32_sheader++;
	    index++;
	    continue;
	}

	if ( strlen(name) == 0 ) {
	    elf32_sheader++;
	    index++;
	    continue;
	}

    if (!stricmp(str, name)) {  return index;}

	elf32_sheader++;
	index++;
    }
    return -1;
}


elf_sect_t *get_elf_secthdr_by_name( elf_file_t *elf32_hdr_ptr, char *name )
{
    int index = 0;
    elf_sect_t *elf32_sheader;

	elf32_sheader = ( elf_sect_t * )((u32_t)elf32_hdr_ptr + (u32_t) elf32_hdr_ptr->shtab_offset );

    index = get_elf_sect_index_by_name( elf32_hdr_ptr, name );

	if (index<0)
	{
		return NULL;
	}
    elf32_sheader += index;
    return elf32_sheader;
}

elf_sym_t *get_elf_sym_by_index( elf_file_t *elf32_hdr_ptr,  int index ) 
{
    elf_sym_t *symtab;
	elf_sect_t *sectab;

	sectab=get_elf_secthdr_by_name(elf32_hdr_ptr, ".symtab");

	if (!sectab){
		return NULL;
	}

	symtab= (elf_sym_t *)((u32_t) elf32_hdr_ptr + sectab->offset );

    symtab += index;

    return symtab;
}


elf_sym_t *get_elf_sym_byname( elf_file_t *elf32_hdr_ptr,  char *name )
{
    int i, entries;
    elf_sect_t *symshdr ;
    elf_sym_t * symtab ;
    elf_sect_t *strhdr ;

	symshdr = get_elf_secthdr_by_name( elf32_hdr_ptr, ".symtab" );
	if (!symshdr){
		return NULL;
	}

	symtab= ( elf_sym_t * )	( ( u32_t ) elf32_hdr_ptr
		+ symshdr -> offset );
	strhdr= get_elf_secthdr_by_name( elf32_hdr_ptr, ".strtab" );

	if (!strhdr){
		return NULL;
	}

	entries = symshdr->size / sizeof( elf_sym_t );

    for ( i = 0; i < entries; i++ ) {
		if ( !symtab -> name ) {
	    	symtab++;
	    	continue;
		}
		if (  ( char * ) ( ( u32_t ) elf32_hdr_ptr +
		    strhdr->offset + symtab->name ) == NULL ) {
		    ASSERT("name null");
			return NULL;
		}
		if ( name == NULL ) {
		    ASSERT("name null");
			return NULL;
		}
		if (!stricmp( ( char * ) ( ( u32_t ) elf32_hdr_ptr +
		    strhdr -> offset + symtab -> name ), name ) ) {
		    return symtab;
		} else 	symtab++;
    }

    
	/*
	 * this means: we did not find the symbol within the symbol table ...
	 */
	//trace( "%s: symbol %s not found in symtab\n", __FUNCTION__, name );
    return (elf_sym_t *)0;
}

void elf_dump_sym(elf_sym_t *symbol)
{
	//kprintf("elf_dump_sym:%s ",(char*)symbol->name);
	kprintf("value:0x%x ",(char*)symbol->value);
	kprintf("size:0x%x ",(char*)symbol->size);
	kprintf("info:0x%x ",(char*)symbol->info);
	kprintf("zero:0x%x ",(char*)symbol->zero);
	kprintf("section:0x%x\n",(char*)symbol->section);
}

static void* get_elf_secton(kern_module_t *mod,const char *name, int *size)
{
	void *addr=NULL;
    elf_sect_t *sect_hdr ;

	sect_hdr = get_elf_secthdr_by_name( (elf_file_t*)mod->module_address, name );

	if (!sect_hdr)
	{
		//kprintf("get_elf_secton %s not exist\n",name);
		return NULL;
	}

	/*if (!sect_hdr->virt_adr)
	{
		kprintf("get_elf_secton %s on %p error\n",name,0);
		return NULL;
	}*/

	addr = sect_hdr->offset+mod->module_address; //offset?

	if(size)*size = sect_hdr->size;


	//kprintf("get_elf_secton %s on %p\n",name,addr);

	return addr;
}

int get_main_elf_sym(void *image, kern_module_t *mod)
  {
	int i;
	elf_file_t *elf_hdr=(elf_file_t *)image;
	elf_sym_t *main_sym[NR_DLL_OPS];

	memset(&mod->opaddr, 0, sizeof(mod->opaddr));	
	memset(&main_sym, 0, sizeof(main_sym));	

	main_sym[0]=get_elf_sym_byname( elf_hdr, "dll_main" );
	main_sym[1]=get_elf_sym_byname( elf_hdr, "dll_destroy" );
	main_sym[2]=NULL;
	main_sym[3]=get_elf_sym_byname( elf_hdr, "__static_initialization_and_destruction_0" );

	for (i=0; i<NR_OK_OPS; i++)
	{
	   if (ELF32_ST_BIND(main_sym[i]->info)!=STB_GLOBAL)
			kprintf("warn: bind %d, not use STB_GLOBAL\n", ELF32_ST_BIND(main_sym[i]->info));

	   if (ELF32_ST_TYPE(main_sym[i]->info)!=STT_FUNC)
			kprintf("warn: type%d, not use STT_FUNC\n", ELF32_ST_TYPE(main_sym[i]->info));

	   if (main_sym[i]==NULL)
	   {
		   //¼ì²é·ûºÅÊÇ·ñ´æÔÚ
		kprintf("%s not found! \n", get_module_sym_name(i));
		return -1;
	   }
	   else{
		   //elf_dump_sym(main_sym[i]);
	   }
	}


	mod->opaddr.dll_main = (unsigned)main_sym[0]->value;
	mod->opaddr.dll_destroy = (unsigned)main_sym[1]->value;
	mod->opaddr.dll_find_secton  = &get_elf_secton;
	if(main_sym[3])mod->opaddr.dll_static_init  = (unsigned)main_sym[3]->value;

	return OK;
}


