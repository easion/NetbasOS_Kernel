/*
 **     (R)Jicama OS
**     Coff Executable Format Support
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/coff.h>
#include <jicama/pe.h>
#include <string.h>


void dump_coff_head(const struct coff_hdr* hdr);
void dump_coff_aout(const struct coff_aout* aout);

/* flags used to describe task memory sections */
#define	SF_ZERO		0x10	/* BSS */
#define	SF_LOAD		0x08	/* load from file */
#define	SF_READ		0x04	/* readable */
#define	SF_WRITE	0x02	/* writable */
#define	SF_EXEC		0x01	/* executable */
#define	SF_BSS		(SF_ZERO | SF_READ | SF_WRITE)

__public u32_t djgpp_pecoff_optional_header(unsigned char *image)
{
	u16_t magic;
	u32_t dj_coffhdr_start;
	struct dos_header *hsp;

	hsp = (struct dos_header *)image;

	if (strncmp((char *)hsp->e_mz_magic, "MZ", 2)!=0)
		return 0;

	  dj_coffhdr_start = hsp->e_cp * DOS_MZBLK_SZ;

  if (hsp->e_cblp!=0) {/*last blk size*/
		dj_coffhdr_start -= (DOS_MZBLK_SZ - hsp->e_cblp);
		//kprintf("dj_coffhdr_start coff found at %x!\n", dj_coffhdr_start);
	  }

	  magic=peek16(image+dj_coffhdr_start);

	 if (magic!=0x014c)
	  { 
		kprintf("djgpp 0x014c coff not found!");
		return 0;
	 }
	 //kprintf("djgpp coff end at %x!", dj_coffhdr_start);
	 return dj_coffhdr_start;  
  }

u32_t winnt_pecoff_optional_header(unsigned char *image)
{
	u16_t magic;
	u32_t dj_coffhdr_start;
	struct dos_header *hsp;
	u32_t signature;

	hsp = (struct dos_header *)image;

	if (memcmp(hsp->e_mz_magic, "MZ", 2)!=0)
		return 0;

	  dj_coffhdr_start = hsp->e_cp * DOS_MZBLK_SZ; /*blksz * numblk*/

  if (hsp->e_cblp!=0) { dj_coffhdr_start -= (DOS_MZBLK_SZ - hsp->e_cblp);  }

	  signature=peek32(image+hsp->e_lfanew);
	  magic=peek16(image+hsp->e_lfanew+4);
	 //if (magic!=0x014c) { return 0; }

	  if (signature != IMAGE_PE_SIGNATURE) return 0;

	  return (long)image+hsp->e_lfanew+4;
}



int djmz_check(struct filp* _fp)
{
	int  headpos;
	struct dos_header dh;
	struct coff_hdr hdr;

	fread((void *)&dh, sizeof(struct dos_header), 1, _fp);

	if (strncmp((char *)dh.e_mz_magic, "MZ", 2)!=0)
		return 0;

	  headpos = dh.e_cp * DOS_MZBLK_SZ;

  if (dh.e_cblp!=0) {
	  /*last blk size*/
		headpos -= (DOS_MZBLK_SZ - dh.e_cblp);
	  }

	if (headpos==0)return 0;
	 fseek(_fp, headpos,SEEK_SET);
	fread((void *)&hdr, sizeof(struct coff_hdr), 1, _fp);
	
   if (coff_check(&hdr) != COFF)
     {
      kprintf ("read_coff(): Not an Executable COFF Format Program!\n");
	  return 0;
   }
   return headpos;
}

int read_djmz(struct filp*  f,  thread_t *pthread, unsigned  *_entry)
{
	int  hp;

	 fseek(f, 0,SEEK_SET);
		
	hp=djmz_check(f);

	if (!hp)return ENOEXEC;
	
	return read_coff(f,  pthread, _entry,hp);
}





int load_pe_exec(struct filp*  f,  thread_t *pthread, unsigned  *entry)
{
	uint32_t coffstart,bss_adr, bss_size;
	pehdr_t exe;
	int i, j, saw_bss = 0, flags;
	struct coff_hdr coff;
	struct coff_peaout aout;
	struct coff_section *sect;
	unsigned char magic_pe[4];
	struct coff_section section[MAX_SECTIONS] ;

	proc_t *task = THREAD_SPACE(pthread);

	fseek(f, 0,SEEK_SET);
	fread((void*)&exe, sizeof(pehdr_t),1,f);

	if(exe.magic[0] != 'M' ||
		exe.magic[1] != 'Z' ||		/* not .EXE */
		exe.hdr_size < 4 ||
		exe.new_exe_offset == 0){	/* not new-style .EXE */
		kprintf("not new-style .EXE \n");
			return 1;
		}

	kprintf("hdr_size=%d\n", exe.hdr_size);
	kprintf("new_exe_offset at 0x%x\n", exe.new_exe_offset);
	 fseek(f, exe.new_exe_offset,SEEK_SET);

	fread((void*)magic_pe, 4,1,f);

	if(magic_pe[0] == 'P' && magic_pe[1] == 'E' &&magic_pe[2] == 0 &&magic_pe[3] == 0){		/* new-style .EXE but not PE .EXE */
		coffstart=exe.new_exe_offset+4;
		fread((void*)&coff, sizeof(struct coff_hdr),1,f);
		fread((void*)&aout,sizeof(struct coff_peaout),1,f);
	}else{
		kprintf("new-style .EXE but not PE .EXE[off (%x)%c-%c-%c] \n",exe.new_exe_offset, magic_pe[0], magic_pe[1], magic_pe[2] );
		return ENOEXEC;
	}

	kprintf("%s IMAGE address %x TBASE %d\n", "COFF ", aout.image_base, aout.t_base);

	if(coff.f_magic != 0x014C ||
		coff.f_opthdr != sizeof(struct coff_peaout) ||
		aout.magic != 0x010B)
			return 1;
/* get entry point */
	(*entry) = aout.entry+aout.image_base;
   task->p_bss_code=aout.entry;

	 for (i=0; i<coff.f_num_sections; i++) {
	  fread((void*)&section[i], sizeof(struct coff_section),1,f);
	  //dump_coff_section(&section[i]);
	 }

/* seek to section headers */
	for(i = 0; i < coff.f_num_sections; i++)
	{
		sect = &section[i];

		if(!memcmp(sect->s_name, _TEXT_SECT, 5) &&	(sect->s_flags & 0xE0) == STYP_TEXT){
				flags = SF_LOAD | SF_READ | SF_EXEC;
		}
		else if(!memcmp(sect->s_name, _DATA_SECT, 5) &&
			(sect->s_flags & 0xE0) == 0x40)
		{
			flags = SF_LOAD | SF_READ | SF_WRITE;

			bss_adr = aout.image_base +sect->s_vis_run_addr + sect->s_raw_size;
			bss_size = sect->s_paddr - sect->s_raw_size;
		}

		else if(!memcmp(sect->s_name, _BSS_SECT, 4) &&
			(sect->s_flags & 0xE0) == 0x80)
		{
			flags = SF_ZERO | SF_READ | SF_WRITE;
			sect->s_raw_size = sect->s_paddr;
			saw_bss = 1;
		}

		else if(!memcmp(sect->s_name, ".idata", 6)){
			return ENOEXEC;}
		else{
			continue;
		}

	u32_t ds_base = proc_phy_addr(task);
	u8_t *load_point=(u8_t *)(ds_base+sect->s_vis_run_addr+aout.image_base);
	task->p_bss_code+=sect->s_raw_size;

      if (fseek(f, sect->s_raw_data_addr, SEEK_SET) < 0){
	         kprintf ("seek to %s: %x failed! \n", sect->s_name, sect->s_raw_data_addr);
		     return ENOEXEC;
	   }

        /*read program data */
	    fread(load_point , sect->s_raw_size, 1, f);	
		for(j=0;j<10;j++)
		kprintf("%x[%c]", load_point[j],load_point[j]);

		kprintf("\n");

		//err = add_section(as, aout->image_base + sect->virt_adr,
			//sect->raw_size, flags, sect->offset);
		//kprintf("%s at vaddr 0x%x paddr 0x%x size %d\n", 
			//sect->s_name, sect->s_vis_run_addr, sect->s_raw_data_addr, sect->s_raw_size);
		//memcpy((char*)(proc_vir2phys(task,sect->s_vis_run_addr)), 
		//(char *)(aout.image_base +sect->s_raw_data_addr), sect->s_raw_size);	
	}

	if(saw_bss==0)
	{
		u32_t __base = proc_phy_addr(task);
		memset((void *)(__base+bss_adr),0, bss_size);
		kprintf("BZERO 0x%x, len %d\n",(__base+bss_adr));
	}

	 task->start_stack = USER_STACK_ADDR_END&0xfffff000;
	 task->p_bss_code += (task->p_bss_code%4096);
	return 0;
}
