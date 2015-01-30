
#ifndef vm86h
#define vm86h

#define	PFX_ES		0x001
#define	PFX_CS		0x002
#define	PFX_SS		0x004
#define	PFX_DS		0x008
#define	PFX_FS		0x010
#define	PFX_GS		0x020

#define	PFX_OP32	0x040
#define	PFX_ADR32	0x080
#define	PFX_LOCK	0x100
#define	PFX_REPNE	0x200
#define	PFX_REP		0x400

#define FPU_CONTEXT_SIZE	108

//! (real-mode) Extract the segment part of a linear address.
#define SEGMENT(linear)		( (unsigned short)(((u32_t)(linear) & 0xFFFF0000) >> 4) )
//! (real-mode) Extract the offset part of a linear address.
#define OFFSET(linear)		( (unsigned short)((u32_t)(linear) & 0xFFFF) )
//! (real-mode) Make a linear address from a segment:offset address.
#define LINEAR(seg, off)	( (u32_t)(((unsigned short)(seg) << 4) + (unsigned short)(off)) )
//! BIOS Interrupt Vector Table (IVT) start address.
#define BIOS_IVT_START		0x00000000
//! BIOS Interrupt Vector Table (IVT) end address.
#define BIOS_IVT_END		0x00001000

//! Video Buffer area start address.
#define VIDEO_BUF_START		0x000A0000
//! Video Buffer area end address.
#define VIDEO_BUF_END		0x000C0000

//! ROM BIOS memory start address.
#define BIOS_ROM_START		0x000C0000
//! ROM BIOS memory end address.
#define BIOS_ROM_END		0x00100000

inline static u16_t get_TR(void)
{u16_t r; __asm__ __volatile__ ("strw %0" : "=q" (r)); return(r); }

#define FP_OFF(x)      ((u32_t)(x) & 0x000F)
#define FP_SEG(x)      (((u32_t)(x) & 0xFFFF0) >> 4)


#define VM86_STACK_SIZE 1024 
#define VM86_USEABLE_BUFSZ 4096
#define VM_MASK 0x00020000


 /*
 typedef struct  {

         long ebx;
         long ecx;
         long edx;
         long esi;
         long edi;
         long ebp;
         long eax;
         long __null_ds;
         long __null_es;
         long __null_fs;
         long __null_gs;
         long orig_eax;
         long eip;
         long cs;
         long eflags;
         long esp;
         long ss;


         long es;
         long ds;
         long fs;
         long gs;

         unsigned long flags;
         unsigned long screen_bitmap;
 }vm86_struct_t;
*/
#define VM86_SCREEN_BITMAP 1


#endif
