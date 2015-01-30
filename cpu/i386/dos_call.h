
#ifndef __dos_call__
#define __dos_call__


struct handler {
  u16_t cs;
  u32_t eip;
};
extern struct handler exc_table[32];

/* Parameter block for the dos_exec call, see INT 21h AH=4Bh */
typedef struct
{
  u16_t  Env;
  u32_t CmdTail;
  u32_t Fcb1;
  u32_t Fcb2;
  u32_t Res1;
  u32_t Res2;
}
tExecParams;

/* Data structure for INT 21 AX=7303h                   */
/* Windows95 - FAT32 - Get extended free space on drive */
typedef struct
{
  u16_t  Size;            /* Size of the returned structure (this one)  */
  u16_t  Version;         /* Version of this structure (desired/actual) */
  /* The following are *with* adjustment for compression */
  u32_t SecPerClus;      /* Sectors per cluster               */
  u32_t BytesPerSec;     /* Bytes per sector                  */
  u32_t AvailClus;       /* Number of available clusters      */
  u32_t TotalClus;       /* Total number of clusters on drive */
  /* The following are *without* adjustment for compression */
  u32_t RealSecPerClus;  /* Number of sectors per cluster     */
  u32_t RealBytesPerSec; /* Bytes per sector                  */
  u32_t RealAvailClus;   /* Number of available clusters      */
  u32_t RealTotalClus;   /* Total number of clusters on drive */
  u8_t  Reserved[8];
}
__attr_packet tExtDiskFree;

/* Registers structure to be used in DPMI service */
/* "Simulate real mode interrupt".                */
typedef union rmregs
{
  /*struct
  {
    unsigned edi;
    unsigned esi;
    unsigned ebp;
    unsigned Res;
    unsigned ebx;
    unsigned edx;
    unsigned ecx;
    unsigned eax;
  } d;*/
  regs_t d;
  struct
  {
    unsigned short di, di_hi;
    unsigned short si, si_hi;
    unsigned short bp, bp_hi;
    unsigned short Res, Res_hi;
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
  struct
  {
    unsigned char edi[4];
    unsigned char esi[4];
    unsigned char ebp[4];
    unsigned char res[4];
    unsigned char bl, bh, ebx_b2, ebx_b3;
    unsigned char dl, dh, edx_b2, edx_b3;
    unsigned char cl, ch, ecx_b2, ecx_b3;
    unsigned char al, ah, eax_b2, eax_b3;
  } h;
}
tRMRegs;


/* Error codes */
#define FD32_E(x)     (0x80000000 | (x))
#define FD32_EINVAL   FD32_E(0x01) /* Invalid argument        */
#define FD32_ENOENT   FD32_E(0x02) /* (FIXME: file not found) No such file or directory */
#define FD32_ENOTDIR  FD32_E(0x03) /* (FIXME: path not found) Not a directory */
#define FD32_EMFILE   FD32_E(0x04) /* Too many open files     */
#define FD32_EACCES   FD32_E(0x05) /* Access denied           */
#define FD32_EBADF    FD32_E(0x06) /* Invalid file handle     */
#define FD32_ENOMEM   FD32_E(0x08) /* Insufficient memory     */
#define FD32_EFORMAT  FD32_E(0x0B) /* Format invalid          */
#define FD32_EACODE   FD32_E(0x0C) /* Access code invalid     */
#define FD32_EIDATA   FD32_E(0x0D) /* Data invalid            */
#define FD32_ENODRV   FD32_E(0x0F) /* Invalid drive           */
#define FD32_EBUSY    FD32_E(0x10) /* Attempt to remove the current directory */
#define FD32_EXDEV    FD32_E(0x11) /* Not same device         */
#define FD32_ENMFILE  FD32_E(0x12) /* No more files           */
#define FD32_EROFS    FD32_E(0x13) /* Read-only file system   */
#define FD32_ENODEV   FD32_E(0x14) /* No such device          */
#define FD32_ENOTRDY  FD32_E(0x15) /* Drive not ready         */
#define FD32_ECRC     FD32_E(0x17) /* CRC error               */
#define FD32_EISEEK   FD32_E(0x19) /* Invalid seek            */
#define FD32_EMEDIA   FD32_E(0x1A) /* Unknown media (not DOS) */
#define FD32_ENOSEC   FD32_E(0x1B) /* Sector not found        */
#define FD32_EWRITE   FD32_E(0x1D) /* Write fault             */
#define FD32_EREAD    FD32_E(0x1E) /* Read fault              */
#define FD32_EGENERAL FD32_E(0x1F) /* General failure         */
#define FD32_EVSHAR   FD32_E(0x20) /* Sharing violation       */
#define FD32_EVLOCK   FD32_E(0x21) /* Lock violation          */
#define FD32_ECHANGE  FD32_E(0x22) /* Invalid media change (ES:DI -> media ID structure)(see #1546) */
#define FD32_EOINPUT  FD32_E(0x26) /* Out of input            */
#define FD32_ENOSPC   FD32_E(0x27) /* No space left on drive  */
#define FD32_EEXIST   FD32_E(0x50) /* File exists             */
#define FD32_EMKDIR   FD32_E(0x52) /* Cannot make directory   */
#define FD32_EINT24   FD32_E(0x53) /* Fail on INT 24          */
#define FD32_ENOTLCK  FD32_E(0xB0) /* Not locked              */
#define FD32_ELOCKED  FD32_E(0xB1) /* Locked in drive         */
#define FD32_ENOTREM  FD32_E(0xB2) /* Media not removable     */
#define FD32_ENOLCK   FD32_E(0xB4) /* No more locks available */
#define FD32_EEJECT   FD32_E(0xB5) /* Eject request failed    */
/* FD32 defined error codes */
#define FD32_ENMOUNT  FD32_E(0x100) /* File system not mounted */
#define FD32_EUTF8    FD32_E(0x101) /* Invalid UTF-8 sequence  */
#define FD32_EUTF16   FD32_E(0x102) /* Invalid UTF-32 sequence */
#define FD32_EUTF32   FD32_E(0x103) /* Invalid Unicode char    */
#define FD32_ENMDEV   FD32_E(0x104) /* No more devices         */

#define FD32_LFNPMAX 260 /* Max length for a long file name path  */
#define FD32_LFNMAX  255 /* Max length for a long file name       */
#define FD32_SFNPMAX 64  /* Max length for a short file name path */
#define FD32_SFNMAX  14  /* Max length for a short file name      */

#if 0
  #define RM_SET_CARRY   r->flags |= 0x0001
  #define RM_CLEAR_CARRY r->flags &= 0xFFFFFFFE
#else
  #define RMREGS_SET_CARRY   r->x.flags |= 0x0001
  #define RMREGS_CLEAR_CARRY r->x.flags &= 0xFFFE
#endif


/* Carry Flag set/clear macros */
#define SET_CARRY   r->d.eflags |= 0x0001
#define CLEAR_CARRY r->d.eflags &= 0xFFFFFFFE

/* DPMI error codes */
#define DPMI_DESCRIPTOR_UNAVAILABLE   0x8011
#define DPMI_INVALID_SELECTOR         0x8022

extern inline void dpmi_return(int res, union rmregs *r)
{
  r->x.ax = res;
  if (res < 0) {
    SET_CARRY;
  } else {
    CLEAR_CARRY;
  }
}

#endif
