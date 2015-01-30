#include <drv/drv.h>
#include <drv/spin.h>
//#include <drv/cpplib.h>
#include <drv/timer.h>
#include <drv/ia.h>
#include <drv/vm86.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "util.h"

typedef struct
{
    //by pusha
	int irq;
	unsigned edi, esi, ebp, esp, ebx, edx, ecx, eax;
	unsigned gs, fs, es, ds;
    unsigned zero, zero2;
	unsigned eip, cs, eflags, user_esp, user_ss;
} interrupt_regs_t;

void biosdisk_reflect(void *arg, unsigned intnum)
{
  u32_t *bos;
  u32_t isr_cs, isr_eip;
  u16_t *old_esp;
  unsigned rmint;
  const u32_t *rm_irq_table = (u32_t *)0;

   bos = (u32_t *)get_int86_sp0();

    if ((intnum >= 0) && (intnum < 0x08)) {
      rmint = intnum +  8;
    } 
	else {
     printk("[BIOSDISK] To be reflected in VM86 mode...\n");
     if ((intnum >= 0x10) && (intnum < 0x10 + 8)) {
        rmint = intnum+0x68;
      } 
	  else {
        if ((intnum == 0x40) || (intnum == 0x15)) {
          rmint = intnum;
        } else {
			  /* Error!!! Should we panic? */
		  panic("[BIOSDISK] Strange interrupt %02xh to be reflected!!!\n",
			  intnum);
          return;
        }
      }
    }

    old_esp = (u16_t *)(*(bos - 6) + (*(bos - 5) << 4));
    //printk("[BIOSDISK] Old stack (Linear): 0x%lx\n", (u32_t)old_esp);
    *(old_esp - 1) = (u16_t)(*(bos - 7)) /*CPU_FLAG_VM | CPU_FLAG_IOPL*/;
    *(old_esp - 2) = (u16_t)(*(bos - 8));
    *(old_esp - 3) = (u16_t)(*(bos - 9));
    *(bos - 6) -= 6;
    
    isr_cs= ((rm_irq_table[rmint]) & 0xFFFF0000) >> 16;
    isr_eip = ((rm_irq_table[rmint]) & 0x0000FFFF);
    //printk("[BIOSDISK] I have to call 0x%lx:0x%lx\n",   isr_cs, isr_eip);
    *(bos - 8) = isr_cs;
    *(bos - 9) = isr_eip;
  
}

my_syscall()
{
	printf("my called\n");
	biosdisk_reflect(NULL,0X40);
}

void int0x40_call();

void irq_init()
{
	printk("install irq ...\n");
	//return ;
	put_irq_handler(6,(u32_t)&biosdisk_reflect,NULL,"bios_floppy"); 
	put_irq_handler(14,(u32_t)&biosdisk_reflect,NULL,"bios_hd"); 
	//put_interrupt(0x40, (unsigned)&int0x40_call );
}


/* Mnemonics for standard BIOS disk operations */
typedef enum
{
    STD_READ   = 0x02,
    STD_WRITE  = 0x03,
    STD_VERIFY = 0x04
}
StdOp;

/* A Cylinder/Head/Sector address */
typedef struct
{
    unsigned c, h, s;
}
Chs;
static inline int lba_to_chs(u32_t lba, Chs *chs, const Chs *total)
{
    u32_t    temp;
    unsigned bytes_per_cyl = total->h * total->s;

    if (lba >= total->c * bytes_per_cyl) return -1;
    chs->c   = lba / bytes_per_cyl;
    temp     = lba % bytes_per_cyl;
    chs->h   = temp / total->s;
    chs->s   = temp % total->s + 1;
    return 0;
}
static inline int std_disk_op(StdOp operation, const biosdisk_drv_t *d, u32_t lba,
                              u8_t num_sectors, u16_t buf_seg, u16_t buf_off)
{
    vm86regs_t reg86;
    Chs chs;
    Chs total = { d->d_Cylinders, d->d_Heads, d->d_Sectors };

    if (lba + num_sectors > d->d_Size) return -1; /* Invalid LBA address */
    if (lba_to_chs(lba + d->d_Start, &chs, &total))
        return -1; /* Invalid LBA address */

    reg86.h.ah = operation;   /* Can be STD_READ, STD_WRITE or STD_VERIFY */
    reg86.h.al = num_sectors; /* Must be nonzero                          */
    reg86.h.ch = (u8_t) chs.c;
    reg86.h.cl = chs.s | ((chs.c & 0x300) >> 2);
    reg86.h.dh = chs.h;
    reg86.h.dl = d->d_DeviceHandle;
    reg86.x.es = buf_seg;
    reg86.x.bx = buf_off;

    do_int86( 0x13, &reg86,&reg86 );
    if (reg86.x.flags & 0x01 ) return -1; //INT 13h error code in AH
    return 0;
}


int biosdisk_stdread(const biosdisk_drv_t *d, u32_t start, u32_t count, void *buffer)
{
    int         res;
#define REAL_VM_MEM (unsigned)0X9000
    u16_t        buf_segment=0, buf_offset=0;
	buf_offset = FP_OFF(REAL_VM_MEM);
	buf_segment = FP_SEG(REAL_VM_MEM);

    //buf_sel = fd32_dosmem_get(count * d->block_size, &buf_segment, &buf_offset);
    res = std_disk_op(STD_READ, d, start, count, buf_segment, buf_offset);
    //fd32_memcpy_from_lowmem(buffer, buf_sel, 0, count * d->block_size);
    //fd32_dosmem_free(buf_sel, count * d->block_size);
	printf("copy it ...\n");
	memcpy(buffer, (void*)REAL_VM_MEM, count*512);
    return res;
}

int biosdisk_stdwrite(const biosdisk_drv_t *d, u32_t start, u32_t count, const void *buffer)
{
    int         res;
    //LOWMEM_ADDR buf_sel;
    u16_t        buf_segment, buf_offset;

    //buf_sel = fd32_dosmem_get(count * d->block_size, &buf_segment, &buf_offset);
    //fd32_memcpy_to_lowmem(buf_sel, 0, (void *) buffer, count * d->block_size);
    res = std_disk_op(STD_WRITE, d, start, count, buf_segment, buf_offset);
    //fd32_dosmem_free(buf_sel, count * d->block_size);
    return res;
}

int biosdisk_test()
{
    vm86regs_t reg86;
    Chs chs;
	char buf[512];
    char*dosptr=(char*)REAL_VM_MEM;
	biosdisk_drv_t *d = get_biosdisk_args(0x200);
    u16_t        buf_seg=0, buf_off=0;
	u32_t lba = 0;
	u8_t num_sectors=1;
    Chs total = { d->d_Cylinders, d->d_Heads, d->d_Sectors };

	buf_off = FP_OFF(dosptr);
	buf_seg = FP_SEG(dosptr);

    if (lba + num_sectors > d->d_Size) return -1; /* Invalid LBA address */
    if (lba_to_chs(lba + d->d_Start, &chs, &total))
        return -1; /* Invalid LBA address */

	printf("chs.c%d,h%d,s%d\n", chs.c,chs.h,chs.s);
    
    reg86.h.ah = STD_READ;   /* Can be STD_READ, STD_WRITE or STD_VERIFY */
    reg86.h.al = num_sectors; /* Must be nonzero                          */
    reg86.h.ch = (u8_t) chs.c;
    //reg86.h.cl = chs.s | ((chs.c & 0x300) >> 2);
    reg86.h.cl = 1;
    reg86.h.dh = 0;
    reg86.h.dl = 0x00;
    reg86.x.es = buf_seg;
    reg86.x.bx = buf_off;

    do_int86( 0x13, &reg86,&reg86 );
	printf("reg86.x.flags %x\n", reg86.x.flags);


    if (reg86.x.flags & 0x01 ){
		bd_reset(0x00);
		return -1; //INT 13h error code in AH
	}

	printf("biosdisk_test: %s \n", (char *)&dosptr[0x36]);
    return 0;
}

