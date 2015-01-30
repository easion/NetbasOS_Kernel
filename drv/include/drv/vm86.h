

#ifndef vm86_dll_H
#define vm86_dll_H

#ifdef __cplusplus
extern "C" {
#endif

#define __SYSTEM__

 
typedef union vm86_regs{
  struct {
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long res;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
  } d;
  struct {
    unsigned short di, di_hi;
    unsigned short si, si_hi;
    unsigned short bp, bp_hi;
    unsigned short res, res_hi;
    unsigned short bx, bx_hi;
    unsigned short dx, dx_hi;
    unsigned short cx, cx_hi;
    unsigned short ax, ax_hi;
    unsigned short flags;
    unsigned short es;
    unsigned short ds;
    unsigned short fs;
    unsigned short gs;
    unsigned short ip;
    unsigned short cs;
    unsigned short sp;
    unsigned short ss;
  } x;
  struct {
    unsigned char edi[4];
    unsigned char esi[4];
    unsigned char ebp[4];
    unsigned char res[4];
    unsigned char bl, bh, ebx_b2, ebx_b3;
    unsigned char dl, dh, edx_b2, edx_b3;
    unsigned char cl, ch, ecx_b2, ecx_b3;
    unsigned char al, ah, eax_b2, eax_b3;
  } h;
} vm86regs_t;

int do_int86(int service,vm86regs_t *,vm86regs_t *);
void read_real_memory(char *buf, int sz);
void fill_real_memory(char *buf, int sz);
void real_memory_addr(u32_t *addr, int *sz);

typedef struct VESA_INFO {
	 unsigned char VESASignature[4] __attribute__ ((packed));
	 unsigned short VESAVersion __attribute__ ((packed));
	 unsigned long OEMStringPtr __attribute__ ((packed));
	 unsigned char Capabilities[4] __attribute__ ((packed));
	 unsigned long VideoModePtr __attribute__ ((packed));
	 unsigned short TotalMemory __attribute__ ((packed));
	 unsigned short OemSoftwareRev __attribute__ ((packed));
	 unsigned long OemVendorNamePtr __attribute__ ((packed));
	 unsigned long OemProductNamePtr __attribute__ ((packed));
	 unsigned long OemProductRevPtr __attribute__ ((packed));
	 unsigned char Reserved[222] __attribute__ ((packed));
	 unsigned char OemData[256] __attribute__ ((packed));
} VESA_INFO;

typedef struct MODE_INFO {
	 unsigned short ModeAttributes __attribute__ ((packed));
	 unsigned char WinAAttributes __attribute__ ((packed));
	 unsigned char WinBAttributes __attribute__ ((packed));
	 unsigned short WinGranularity __attribute__ ((packed));
	 unsigned short WinSize __attribute__ ((packed));
	 unsigned short WinASegment __attribute__ ((packed));
	 unsigned short WinBSegment __attribute__ ((packed));
	 unsigned long WinFuncPtr __attribute__ ((packed));
	 unsigned short BytesPerScanLine __attribute__ ((packed));

	 unsigned short XResolution __attribute__ ((packed));
	 unsigned short YResolution __attribute__ ((packed));
	 unsigned char XCharSize __attribute__ ((packed));
	 unsigned char YCharSize __attribute__ ((packed));
	 unsigned char NumberOfPlanes __attribute__ ((packed));
	 unsigned char BitsPerPixel __attribute__ ((packed));

  /* >=1.2 */
	 unsigned char NumberOfBanks __attribute__ ((packed));
	 unsigned char MemoryModel __attribute__ ((packed));
	 unsigned char BankSize __attribute__ ((packed));
	 unsigned char NumberOfImagePages __attribute__ ((packed));
	 unsigned char Reserved_page __attribute__ ((packed));
	 unsigned char RedMaskSize __attribute__ ((packed));
	 unsigned char RedMaskPos __attribute__ ((packed));
	 unsigned char GreenMaskSize __attribute__ ((packed));
	 unsigned char GreenMaskPos __attribute__ ((packed));
	 unsigned char BlueMaskSize __attribute__ ((packed));
	 unsigned char BlueMaskPos __attribute__ ((packed));
	 unsigned char ReservedMaskSize __attribute__ ((packed));
	 unsigned char ReservedMaskPos __attribute__ ((packed));
	 unsigned char DirectColorModeInfo __attribute__ ((packed));
	 unsigned long PhysBasePtr __attribute__ ((packed));
	 unsigned long OffScreenMemOffset __attribute__ ((packed));
	 unsigned short OffScreenMemSize __attribute__ ((packed));
	 unsigned char Reserved[206] __attribute__ ((packed));
} MODE_INFO;

#define FP_OFF(x)      ((unsigned long)(x) & 0x000F)
#define FP_SEG(x)      (((unsigned long)(x) & 0xFFFF0) >> 4)


#ifdef __cplusplus
}
#endif

#endif


