
#define GDB_REGISTER_FILE_COUNT 14 //ia32
//#define GDB_REGISTER_FILE_COUNT 32 //arm

extern int dbg_register_file[GDB_REGISTER_FILE_COUNT];
//#define dprintf kprintf

#define out8(A,B) outb((B),(A))
#define in8 inb
extern void cmd_gdb(int, char **);


void arch_dbg_con_puts(const char *s);
char arch_dbg_con_putch(const char c);
int arch_dbg_con_init();
int arch_dbg_con_init2();
char arch_dbg_con_read(void);
void dbg_puts(const char *s);
char dbg_putch(char c);
size_t arch_dbg_con_write(const void *buf, size_t len);
void dbg_save_registers(int *);
void do_reboot(void);
int dprintf(const char *fmt, ...);
void dbg_make_register_file(unsigned int *file, const regs_t * frame);
unsigned long atoul(const char *num);


#include <ansi.h>

