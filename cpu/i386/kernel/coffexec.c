/*
 **     (R)Jicama OS
**     Coff Executable Format Support
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/coff.h>
#include <unistd.h>
#include <string.h>


void dump_coff_head(const struct coff_hdr* hdr)
{
	 kprintf("Coff magic: %x \n", hdr->f_magic);
	 kprintf("Header f_num_sections: %d \n", hdr->f_num_sections);
	 kprintf("Header time: %d \n", hdr->f_time);
	 kprintf("Offset of symbol table: %x \n", hdr->f_sym_ptr);
	 kprintf("Symbol table entries: %d \n", hdr->f_num_syms);
	 kprintf("struct (coff_aout) size: %d \n", hdr->f_opthdr);
	 kprintf("Header flag: %x \n", hdr->f_flag);     
}


int coff_check(const struct coff_hdr* hdr)
{	
	 if(I386BADMAG(hdr)){
			trace("COFF I386BADMAG**");
			return NO_EXEC;
	   }

	if((hdr->f_flag & 0x02) == 0) {
		if(hdr->f_flag & F_RELFLG){
			trace("Relocations have been stripped from COFF file(0x%x)\n", hdr->f_flag);
			return NO_EXEC;
		}else{
			//kprintf("COFF DLL**");
			return COFF;
		}
	}

//kprintf("COFF**");
return (COFF);
}

__asmlink u32_t djgpp_pecoff_optional_header( char *image);

exec_t pe_check( char *ident)
{
	u32_t mzptr;

	if(ident[0] != 'M' || ident[1] != 'Z'){
		  return NO_EXEC;
	  }

	mzptr=djgpp_pecoff_optional_header(ident);

	if(mzptr==0)
		return PE;
	else
		return DJMZ;

	return PE;
}

void dump_coff_aout(const struct coff_aout* aout)
{
	 kprintf("a.out magic: %x \n", aout->magic);
	 kprintf("a.out f_version: %d \n", aout->version);
	 kprintf("a.out text: %d \n", aout->tsize);
	 kprintf("a.out data: %d \n", aout->dsize);
	 kprintf("a.out bbs: %d \n", aout->bsize);
	 kprintf("a.out entry: %x \n", aout->entry);
	 kprintf("a.out text base: %x \n", aout->t_base);
	 kprintf("a.out data base: %x \n", aout->d_base);
}

int coff_read_aout(const struct coff_aout* aout)
{
	if(aout->t_base > USER_SPACE_SIZE)
		return FAILED;

    if(aout->magic != 0x10b) {/*For 32 bit a.out magic: 0x10b*/
	 trace("Bad COFF Magic: %X\n", aout->magic);
	 return ENOEXEC;
	}

	/*entry out of ldt limit.*/
	if(aout->entry > USER_SPACE_SIZE)
		return FAILED;

	return (OK);
}

void dump_coff_section(const struct coff_section* scn)
{
	kprintf("SECTION name: %s\t", scn->s_name);
	kprintf("-> size: %d\t", scn->s_raw_size);
	kprintf("-> ptr : %d\n", scn->s_raw_data_addr); /* file ptr to raw data for section */
	kprintf("-> vaddr : %x\t", scn->s_vis_run_addr);
	kprintf("-> paddr : %x\t", scn->s_paddr);
	kprintf("-> s_flags : 0x%x\n", scn->s_flags);
}



//////////////////////////////////



int coff_read_section(int nr_sect, struct filp* __fp, proc_t *rp, int __peff)
{
	int tasksize,  i, flag;
	/*for .test, .data, .bss ... (num:5)segment*/
	 struct coff_section section[nr_sect] ;

	u32_t cs_base = get_desc_base(&(rp->p_ldt[CS_LDT_INDEX]));
	u32_t ds_base = proc_phy_addr(rp);

	 if(nr_sect>MAX_CPPSECTIONS){
		 panic("COFF>MAX_SECTIONS");
		 return -1;
	 }

	 for (i=0; i<nr_sect; i++) {
	  fread((void*)&section[i], sizeof(struct coff_section),1,__fp);
	 }

	 for (i=0; i<nr_sect; i++)
	{
		char *load_point=(char *)(ds_base+section[i].s_vis_run_addr);
		 //kprintf("s_flags-%x-%s-%x\n", section[i].s_flags, section[i].s_name,section[i].s_raw_size);
		 flag=section[i].s_flags & 0xE0;

		 if (flag!=STYP_TEXT&&flag!=STYP_DATA&&flag!=STYP_BSS)
			 continue;

		 rp->p_bss_code+=section[i].s_raw_size;

		if(flag==STYP_BSS){
			memset(load_point,0,section[i].s_raw_size);
			 continue;
		}

		 /*other we unknow seg, may a c++ program*/
      if (fseek(__fp, section[i].s_raw_data_addr+__peff, SEEK_SET) < 0){
	         trace ("seek to %s: %x failed! \n", section[i].s_name, section[i].s_raw_data_addr);
		     return ENOEXEC;
	   }	   

        /*read program data */
	    fread(load_point , section[i].s_raw_size, 1, __fp);	
	 }/*end for*/
	
	 rp->p_bss_code += (rp->p_bss_code%4096);
	 //kprintf("end_code:0x%x\n", rp->end_code);
	 return (OK);

}

int read_coff(struct filp*  fp,  thread_t *ethread, unsigned  *entry,int pos)
{
	struct coff_hdr hdr;
	struct coff_aout aout ;
	int  i,err;
	proc_t *rp = ethread->plink;
	//size_t cs_base=0, ds_base=0;

	fseek(fp, pos,SEEK_SET);

	/*start read coff header part*/
	fread(&hdr, sizeof(struct coff_hdr), 1, fp);
	fread(&aout, sizeof(struct coff_aout), 1, fp);

	if (coff_check(&hdr) != COFF)
	ERROR: {
	trace ("read_coff(): Not an Executable COFF Format Program!\n");
	return ENOEXEC;
	}
	/*read aout*/
	if (coff_read_aout(&aout) < 0){
	goto ERROR;
	return ENOEXEC;
	}

	*entry = aout.entry;
	rp->p_bss_code = aout.entry;

	err=coff_read_section(hdr.f_num_sections, fp,rp, pos);	
	return (err);
}
//////////////////////////////////////////////////////////////
