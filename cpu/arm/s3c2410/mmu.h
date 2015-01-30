#include 	"s3c2410.h"

/*����Ϊ�����ַ*/ 
#define	MMU_TABLE_BASE	SDRAM_BASE
#define	PROCESS0_BASE	SDRAM_BASE+0x4000
#define 	VECTORS_BASE	0xffff0000
#define	VECTORS_PHY_BASE	SDRAM_BASE+SDRAM_SIZE-0x100000


/*�����ַSDRAM_RAW_RW_VA_BASE + i*0x100000ָ��PID=i�Ľ��̵�1M�ڴ棬���Ҵ������ַ>32M��������PID�޹�*/
/*SDRAM raw read/write vitual address base*/
#define	SDRAM_RAW_RW_VA_BASE	((VECTORS_BASE & 0xfff00000)-SDRAM_SIZE)	

/*�Ƿ�ʹ��Cache*/
#define	 CONFIG_CPU_D_CACHE_ON	1
#define	 CONFIG_CPU_I_CACHE_ON	1
  

void mmu_tlb_init(void);
void mmu_init(void);
