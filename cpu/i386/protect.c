/*
**     (R)Jicama OS
**     Protect Mode Function
**     Copyright (C) 2003 DengPingPing
*/
#include <jicama/system.h>
#include <jicama/paging.h>
#include <assert.h>
#include <arch/x86/protect.h>
#include <arch/x86/traps.h>
#include <string.h>
 int get_gdt_entry(void);

void set_tss_desc(int i, unsigned long tss_addr);
/*
段地址:偏移地址.......线性地址............... 使用情况 
0000:0000-0000:03FF 000000-0003FF 中断向量表interrupt vector table 
0040:0000-0040:00FF 000400-0004FF BIOS 数据区 
0050:0000-0050:76FF 000500-007BFF 空闲常规内存(CONVENTIONAL MEMORY ) 
0000:7C00-0000:7DFF 007C00-007DFF 引导扇区 
0000:7E00-9000:FBFF 007E00-09FBFF 空闲常规内存 
9000:FC00-9000:FFFF 09FC00-09FFFF 扩展BIOS 数据区(EBDA) 
A000:0000-F000:FFFF 0A0000-0FFFFF video memory 和BIOS ROMs 
FFFF:000F-FFFF:FFFF 100000-10FFEF 高端内存区 (HMA) 
10FFF0- 空闲扩展内存
*/



__asmlink short  vm86_current_ctx_sel;

void tss_init(void)
{  
  	unsigned ldt_selector;
	//every process is not include ldt descriptor
	//KERNEL_TSS defined 5

	
	//ktss.ss0 = 0x10;
	//iobase only for i386
	//ktss.iobase = sizeof ktss;	
	ltr(KERNEL_TSS);  //ltr:386TSS: loading tr.limit < 103
	vm86_current_ctx_sel = KERNEL_TSS*0x08;
	// lldt(0); //load ldt register

	ldt_selector = get_gdt_entry();
}

#define USE_GDT

int sel_to_ldt_index(u16_t sel, u16_t *Index)
{
  if (!(sel & 0x4))
  {
    sel /= 8;
    if ((sel < 256  ) &&
        (gdt[(sel)].access))
    {
      *Index = sel;
      return 0;
    }
  }
  return -1;
}

 int get_gdt_entry(void)
{
	int _entry;
	
	//从进程表最后开始查找
	for (_entry = FIRST_LDT; _entry<MAX_GDT; _entry++)
	{
	//if (!*(u32_t*)&(gdt[_entry]) && !(*(u32_t*)&(gdt[_entry])+1))
      if (gdt[_entry].access == 0)
			return _entry;
	}
	return 0;
}

 int free_gdt_entry(int _entry)
{
	if (_entry < FIRST_LDT || _entry>=MAX_GDT)
	{
		return -1;
	}

  if (gdt[_entry].access == 0)
		return 1;

	  gdt[_entry].access = 0;

	return 0;
}

int DPMI_get_descriptor(u16_t sel, u16_t BufferSelector, u32_t BufferOffset)
{
  u16_t i;
  int  err;

  err = sel_to_ldt_index(sel, &i);
  if (err < 0) return err;

  /* The following assembly code copies the descriptor into */
  /* the buffer pointed by the specified 48-bit pointer.    */
  __asm("push   %%es               \n"
      "mov    %%ax,%%es          \n"
      "mov    (%%ebx),%%edx      \n"
      "mov    %%edx,%%es:(%%edi) \n"
      "mov    4(%%ebx),%%edx     \n"
      "mov    %%edx,%%es:4(%%edi)\n"
      "pop    %%es               \n"
      :
      : "b" (&DESCRIPTOR_TABLE(i)), "a" (BufferSelector), "D" (BufferOffset)
      : "edx");
  return 0;
}

int DPMI_set_descriptor(u16_t sel, u16_t BufferSelector, u32_t BufferOffset)
{
  u16_t i;
  int  err;

  /* TO DO: CHECK_POINTER(tss_ptr->tss_es, tss_ptr->tss_edi); */

  err = sel_to_ldt_index(sel, &i);
  if (err < 0) return err;

  /* The following assembly code copies the buffer pointed */
  /* by the specified 48-bit pointer into the descriptor.  */
  __asm("push   %%es               \n"
      "mov    %%ax,%%es          \n"
      "mov    %%es:(%%ebx),%%edx \n"
      "mov    %%edx,(%%edi)      \n"
      "mov    %%es:4(%%ebx),%%edx\n"
      "mov    %%edx,4(%%edi)     \n"
      "pop    %%es               \n"
      :
      : "D" (&DESCRIPTOR_TABLE(i)), "a" (BufferSelector), "b" (BufferOffset)
      : "edx");
  DESCRIPTOR_TABLE(i).access |= 0x10; /* Make sure non-system, non-zero */
  return 0;
}

int DPMI_allocate_descriptors(u16_t num_sel)
{
  u16_t i, j;
  u16_t selector;

  for (i = DESCRIPTOR_TABLE_INF; (i + num_sel) < DESCRIPTOR_TABLE_SUP; i++)
  {
    for (j = 0; (j < num_sel) && (!DESCRIPTOR_TABLE(i + j).access); j++);
    if (j >= num_sel) break;
  }

  if ((i + num_sel) < DESCRIPTOR_TABLE_SUP)
  {
    for(j = 0; j < num_sel; j++, i++)
    {
      /*
       * Allocates a descriptor with 0 base and limit
       * Access: Present, data, r/w
       * Granularity: 32-bit, byte granularity, used
       */
      //GDT_place(INDEX_TO_SELECTOR(i), 0, 0, 0x92 | (RUN_RING << 5), 0x40);
		set_desc (&gdt[i], 0, 0, USER_DATA_SEG); //VIDEO: 0x38
    }
    /* Return the first selector allocated */
    selector = INDEX_TO_SELECTOR(i - num_sel);
    return selector;
  }
  return 0xFFFF8011;
}

int DPMI_segment_to_descriptor(u16_t rm_seg)
{
  u16_t i, selector;
  int  err;
  u16_t base_lo  = rm_seg << 4;
  u16_t base_med = rm_seg >> 12;

  /* The DPMI Specifications says that:
   * Multiple calls to this function with the same segment address
   * will return the same selector. So we search for same descriptor.
   */
  /*for(i = DESCRIPTOR_TABLE_INF; i < DESCRIPTOR_TABLE_SUP; i++)
  {
    if ((!DESCRIPTOR_TABLE(i).gran                ) &&
        (!DESCRIPTOR_TABLE(i).base_hi             ) &&
        ( DESCRIPTOR_TABLE(i).lim_lo   == 0xFFFF  ) &&
        ( DESCRIPTOR_TABLE(i).access              ) &&
        ( DESCRIPTOR_TABLE(i).base_lo  == base_lo ) &&
        ( DESCRIPTOR_TABLE(i).base_med == base_med))
    {
      selector = INDEX_TO_SELECTOR(i);
      return selector;
    }
  }*/

  /* A selector for that real mode segment is not allocated.
   * Allocate one now.
   */
  err = DPMI_allocate_descriptors(1);
  if (err < 0) return err;
  i = err;
  /* Return the selector just allocated */
  selector = i;

  /* Then convert that selector into a descriptor table index to
   * initialize our descriptor.
   * We can safely ignore the exit code of the following
   */
  sel_to_ldt_index(i,&i);

  //DESCRIPTOR_TABLE(i).lim_lo   = 0xFFFF;
  //DESCRIPTOR_TABLE(i).base_lo  = base_lo;
  //DESCRIPTOR_TABLE(i).base_med = base_med;
 // DESCRIPTOR_TABLE(i).gran     = 0; /* 16-bit, not 32 */

	set_desc (&gdt[i], 0, 0xFFFF, USER_DATA_SEG); //VIDEO: 0x38
  return selector;
}

int DPMI_free_descriptor(u16_t sel)
{
	u16_t _entry;
	
	if(sel_to_ldt_index(sel, &_entry)!=0)
		return -1;

	gdt[_entry].access = 0;
}

int DPMI_create_alias_descriptor(u16_t sel)
{
  u16_t Source, desc, new_sel;
  int  err;

  err = sel_to_ldt_index(sel, &Source);
  if (err < 0) return err;

  err = get_gdt_entry();
  if (err < 0) return err;
  desc = err;

  //sel_to_ldt_index(desc,&desc);

  new_sel = INDEX_TO_SELECTOR(desc);
  DESCRIPTOR_TABLE(desc) = DESCRIPTOR_TABLE(Source);
  /* Data, exp-up, wrt */
  DESCRIPTOR_TABLE(desc).access = 2 | (DESCRIPTOR_TABLE(desc).access & 0xF0);

  return new_sel;
}


void tss_movein(int nr, u32_t* from)
{
	ASSERT(from != NULL);
	memcpy((void*)&gdt[nr], from, DESC_SIZE);
	return;
}

void tss_copyto(struct tss_struct *to, struct tss_struct * from)
{
	ASSERT(to != NULL);
	ASSERT(from != NULL);
	memcpy((void*)to, (void*)from, sizeof(struct tss_struct));
	return;
}


//NOTE: If in Kernel Mode, Code Access Value is 0x9a,
                                      //But Data Access Value is 0x92.
void set_usercode(sys_desc *_seg, u32_t  base, u32_t  size)  
{
	//Initializtion Code segment 0xfa (0x9a:if kernel mode)
	set_desc (_seg, base, size, USER_CODE_SEG ); 
}

void set_userdata(sys_desc *_seg, u32_t  base,u32_t   size)  
{
	//Initializtion Data segment 0xf2(0x92:if kernel mode)
	set_desc (_seg, base, size, USER_DATA_SEG);
}

u32_t  get_base(u16_t sel)
{
	u32_t  base;
	sys_desc *_seg;

	int i = (sel >> 3);    
	_seg = &gdt[i];

	if(i>=8192)return 0;

	base = _seg->base_low | ((u32_t) _seg->base_middle << 16);
	base |= ((u32_t) _seg->base_high << 24);

	return base;
}




