#include <jicama/system.h>
#include <jicama/utsname.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <jicama/module.h>
#include <assert.h>

void arch_init2()
{
	kprintf("arch_init2 called..\n");
	sym_init();
}



static void idle()
{
	static int first = 0;
	char *str = "hello"; //(char*)0x01400000;

	if (first == 0)
	{
		first++;
		kprintf("idle run %s ...\n",str);
		uexit(1);
		kprintf("idle run 2 ...\n");
	}

	return;

	kprintf("idle run  ...\n");
	//int err = uwrite("news");
	//kprintf("idle():uwrite  %d ok..\n", err);
}

void load_dll()
{
	char *so_mem = mm_malloc(8000);
	//char so_mem[4096];
	char* addr= (char*)0x02000000;
	char *darg[] = {
		"armfs",
		"-rramdisk",
		"-mrw",
		NULL
	};

	if (!so_mem)
	{
		return ;
	}
	memcpy(so_mem, addr, 95*1024);

	dev_prvi_t dev_m;

	int fd = dev_open("/dev/ramdisk", 0,&dev_m);

	if (fd<0)
	{
		return ;
	}

	unsigned long disk_addr = 0x01400000;
	unsigned long disk_len = 0x00400000;

	dev_ioctl(&dev_m, 1, disk_addr, 4, true); //ramdisk 开始地址
	dev_ioctl(&dev_m, 2, disk_len, 4,true); //ramdisk 尺寸
	dev_ioctl(&dev_m, 3, 512, 4, true); //ramdisk 块大小

	kprintf("so_mem start at %x\n", (u32_t)so_mem);

	load_dll_memory("armfs",so_mem, 4096, darg);
	kprintf("load_dll_memory ok\n");
}

void arch_init3()
{
#define INTGLOBAL		21
	kprintf("arch_init3 called..\n");
	en_irq(INTGLOBAL);
	
	set_idle_hook(NULL);
}


void system_init()
{
	kprintf("system_init called..\n");
}


int traps_init()
{
	kprintf("traps_init called..\n");

	irq_table_init();

	/* init board */
	//arm_board_init();
	kprintf("arm_board_init ok ...\n");
	return 0;
}

void do_gettime(struct tm*  toget)
{
}

void get_time( struct tm *time)
{
}


int grub_mem_useable(unsigned long* lower, unsigned long* upper)
{
	return 0;
}

void kb_init(void)
{
}

void kb_deinit()
{
}

int proc_mem_count(proc_t *rp)
{
}

main()
{
}


char _irq_stack_start[128];
char _fiq_stack_start[128];
char _undefined_stack_start[64];
char _abort_stack_start[64];

__local void arm_clear_bss(void)
{
	unsigned char *dst;

	extern unsigned  __bss_start;
	extern unsigned  __bss_end;

	dst = (unsigned char*)&__bss_start; 
	arm_board_init();
	while (dst < (unsigned char*)&__bss_end) *dst++ = 0;
}



int arch_startup(void)
{
	/* clear .bss */
	arm_clear_bss();

	/* enable cpu cache */
	arm_icache_enable();
	arm_dcache_enable();

	return core_start(NULL);
}

console_t vtty[3];
extern struct tty_ops ser_ops ;

void console_init(void)
{
	int i;
	unsigned the_eflag;


	for(i=0;i<1;i++){
		vtty[i].attrib = WHITE | BG_BLACK;
		vtty[i].base = 0 ;
		vtty[i].vga_ops =  &ser_ops; 
		regster_console_device(&vtty[i]);
	}

	select_console(&vtty[0],true);	

	current_tty->cur_x = 0;
	current_tty->cur_y = 0;

	kprintf("  \x1B[33mN\x1B[31me\x1B[32mt\x1B[31mb\x1B[36ma\x1B[32ms  "
	"\x1B[33;1mO\x1B[34;0mS"
	"  %s %s \n  %s %s\n", UTS_VERSION,UTS_RELEASE,  COPYRIGHT, EMAIL);
#if defined(__GNUC__)
#if __GNUC__>=3
	syslog(LOG_INFO, "Kernel built with GCC(3) version %u.%u.%u\n",
		__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
	syslog(LOG_INFO, "Kernel built with GCC version %u.%u\n",
		__GNUC__, __GNUC_MINOR__);
#endif
#endif


}


