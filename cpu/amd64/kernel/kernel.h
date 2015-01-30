#if !defined(_KERNEL_H_INCLUDED_)
#define _KERNEL_H_INCLUDED_
//	Native types


#define _cs_base      0x00010000
#define _base_PML4    0x00021000
#define _base_PDPT    0x00022000
#define _base_PDT4GB  0x00024000
#define _base_idt64   0x00028000

struct __memstat {
	uint32  acpiRecMemBase, acpiRecMemSize, acpiNVSMemBase, acpiNVSMemSize;
	uint32  szMidMem, szHiMem;
	uint64  szExtMem;
	uint16   szLowMem;
	uint16  _text_vram;
	uint16  _apm_ver;
	uint8   _col, _row;
	uint16  gdt64_Base;
};

#endif /* _KERNEL_H_INCLUDED_ */
