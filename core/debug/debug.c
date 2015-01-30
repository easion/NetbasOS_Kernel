
#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <jicama/grub.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <jicama/spin.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "gdb.h"






int dbg_register_file[GDB_REGISTER_FILE_COUNT]; /* XXXmpetit -- must be made generic */


static bool serial_debug_on = false;
CREATE_SPINLOCK (dbg_spinlock );

struct debugger_command
{
	struct debugger_command *next;
	void (*func)(int, char **);
	const char *cmd;
	const char *description;
};

static struct debugger_command *commands;

#define LINE_BUF_SIZE 1024
#define MAX_ARGS 16
#define HISTORY_SIZE 16

static char line_buf[HISTORY_SIZE][LINE_BUF_SIZE] = { "", };
static char parse_line[LINE_BUF_SIZE] = "";
static int cur_line = 0;
static char *args[MAX_ARGS] = { NULL, };

#define distance(a, b) ((a) < (b) ? (b) - (a) : (a) - (b))

static int debug_read_line(char *buf, int max_len)
{
	char c;
	int ptr = 0;
	bool done = false;
	int cur_history_spot = cur_line;

	while(!done) {
		c = arch_dbg_con_read();
		switch(c) {
			case '\n':
			case '\r':
				buf[ptr++] = '\0';
				dbg_puts("\n");
				done = true;
				break;
			case 8: // backspace
				if(ptr > 0) {
					dbg_puts("\x1b[1D"); // move to the left one
					dbg_putch(' ');
					dbg_puts("\x1b[1D"); // move to the left one
					ptr--;
				}
				break;
			case 27: // escape sequence
				c = arch_dbg_con_read(); // should be '['
				c = arch_dbg_con_read();
				switch(c) {
					case 67: // right arrow acts like space
						buf[ptr++] = ' ';
						dbg_putch(' ');
						break;
					case 68: // left arrow acts like backspace
						if(ptr > 0) {
							dbg_puts("\x1b[1D"); // move to the left one
							dbg_putch(' ');
							dbg_puts("\x1b[1D"); // move to the left one
							ptr--;
						}
						break;
					case 65: // up arrow
					case 66: // down arrow
					{
						int history_line = 0;

//						dprintf("1c %d h %d ch %d\n", cur_line, history_line, cur_history_spot);

						if(c == 65) {
							// up arrow
							history_line = cur_history_spot - 1;
							if(history_line < 0)
								history_line = HISTORY_SIZE - 1;
						} else {
							// down arrow
							if(cur_history_spot != cur_line) {
								history_line = cur_history_spot + 1;
								if(history_line >= HISTORY_SIZE)
									history_line = 0;
							} else {
								break; // nothing to do here
							}
						}

//						dprintf("2c %d h %d ch %d\n", cur_line, history_line, cur_history_spot);

						// swap the current line with something from the history
						if(ptr > 0)
							dprintf("\x1b[%dD", ptr); // move to beginning of line

						strcpy(buf, line_buf[history_line]);
						ptr = strlen(buf);
						dprintf("%s\x1b[K", buf); // print the line and clear the rest
						cur_history_spot = history_line;
						break;
					}
					default:
						break;
				}
				break;
			case '$':
			case '+':
				/* HACK ALERT!!!
				 *
				 * If we get a $ at the beginning of the line
				 * we assume we are talking with GDB
				 */
				if(ptr == 0) {
					strcpy(buf, "gdb");
					ptr= 4;
					done= true;
					break;
				} else {
					/* fall thru */
				}
			default:
				buf[ptr++] = c;
				dbg_putch(c);
		}
		if(ptr >= max_len - 2) {
			buf[ptr++] = '\0';
			dbg_puts("\n");
			done = true;
			break;
		}
	}
	return ptr;
}


static int debug_parse_line(char *buf, char **argv, int *argc, int max_args)
{
	int pos = 0;

	strcpy(parse_line, buf);

	if(!isspace(parse_line[0])) {
		argv[0] = parse_line;
		*argc = 1;
	} else {
		*argc = 0;
	}

	while(parse_line[pos] != '\0') {
		if(isspace(parse_line[pos])) {
			parse_line[pos] = '\0';
			// scan all of the whitespace out of this
			while(isspace(parse_line[++pos]))
				;
			if(parse_line[pos] == '\0')
				break;
			argv[*argc] = &parse_line[pos];
			(*argc)++;

			if(*argc >= max_args - 1)
				break;
		}
		pos++;
	}

	return *argc;
}


static void kernel_debugger_loop()
{
	int argc;
	struct debugger_command *cmd;

	disable();

	dprintf("kernel debugger on cpu %d\n", 0);

	for(;;) {
		dprintf("> ");
		debug_read_line(line_buf[cur_line], LINE_BUF_SIZE);
		debug_parse_line(line_buf[cur_line], args, &argc, MAX_ARGS);
		if(argc <= 0)
			continue;


		cmd = commands;
		while(cmd != NULL) {
			if(strcmp(args[0], cmd->cmd) == 0) {
				cmd->func(argc, args);
			}
			cmd = cmd->next;
		}
		cur_line++;
		if(cur_line >= HISTORY_SIZE)
			cur_line = 0;
	}
}


#if __GNUC__>=4
void dbg_save_registers(int *data)
{
}

#else

__asm__ (".global dbg_save_registers;.type dbg_save_registers,@function; "\
	"dbg_save_registers:\n\r"
	"pushl	%esi\n\r"
	"pushl	%eax\n\r"
	"movl	12(%esp), %esi\n\r"
	"movl	%eax, 0(%esi)\n\r"
	"movl	%ebx, 4(%esi)\n\r"
	"movl	%ecx, 8(%esi)\n\r"
	"movl	%edx, 12(%esi)\n\r"

	"lea	16(%esp), %eax\n\r"
	"movl	%eax, 16(%esi)\n\r"
	"movl	%ebp, 20(%esi)\n\r"

	"movl	4(%esp), %eax\n\r"
	"movl	%eax, 24(%esi)\n\r"
	"movl	%edi, 28(%esi)\n\r"

	"movl	8(%esp), %eax\n\r"
	"movl	%eax, 32(%esi)\n\r"

	"pushfl\n\r"
	"popl	%eax\n\r"
	"mov	%eax, 36(%esi)\n\r"

	"movl	%cs, 40(%esi)\n\r"
	"movl	%ss, 44(%esi)\n\r"
	"movl	%ds, 48(%esi)\n\r"
	"movl	%es, 52(%esi)\n\r"

	"popl	%eax\n\r"
	"popl	%esi\n\r"
	"ret"
);
#endif

void kernel_debugger(void)
{
	/* we're toast, so let all the sem calls whatnot through */
	//kernel_startup = true;

	dbg_save_registers(dbg_register_file);

	kernel_debugger_loop();
}
/*
int panic(const char *fmt, ...)
{
	int ret = 0;
	va_list args;
	char temp[128];

	dbg_set_serial_debug(true);

	disable();

	va_start(args, fmt);
	ret = vsprintf(temp, fmt, args);
	va_end(args);

	dprintf("PANIC%d: %s", smp_get_current_cpu(), temp);

	if(debugger_on_cpu != smp_get_current_cpu()) {
		// halt all of the other cpus

		// XXX need to flush current smp mailbox to make sure this goes
		// through. Otherwise it'll hang
		smp_send_broadcast_ici(SMP_MSG_CPU_HALT, 0, 0, 0, NULL, SMP_MSG_FLAG_SYNC);
	}

	kernel_debugger();

	enable();
	return ret;
}*/

int dprintf(const char *fmt, ...)
{
	va_list args;
	char temp[512];
	int ret = 0;

	if(serial_debug_on) {
		va_start(args, fmt);
		ret = vsprintf(temp,512, fmt, args);
		va_end(args);

		dbg_puts(temp);
	}
	return ret;
}

char dbg_putch(char c)
{
	char ret;

	if(serial_debug_on) {
		disable();
		spin_lock(&dbg_spinlock);

		ret = arch_dbg_con_putch(c);

		spin_unlock(&dbg_spinlock);
		enable();
	} else {
		ret = c;
	}

	return ret;
}

void dbg_puts(const char *s)
{
	if(serial_debug_on) {
		disable();
		spin_lock(&dbg_spinlock);

		arch_dbg_con_puts(s);

		spin_unlock(&dbg_spinlock);
		enable();
	}
}

size_t dbg_write(const void *buf, size_t len)
{
	size_t ret;

	if(serial_debug_on) {
		disable();
		spin_lock(&dbg_spinlock);

		ret = arch_dbg_con_write(buf, len);

		spin_unlock(&dbg_spinlock);
		enable();
	} else {
		ret = len;
	}
	return ret;
}

int dbg_add_command(void (*func)(int, char **), const char *name, const char *desc)
{
	struct debugger_command *cmd;

	cmd = (struct debugger_command *)kmalloc(sizeof(struct debugger_command),0);
	if(cmd == NULL)
		return -1;

	cmd->func = func;
	cmd->cmd = name;
	cmd->description = desc;

	disable();
	spin_lock(&dbg_spinlock);

	cmd->next = commands;
	commands = cmd;

	spin_unlock(&dbg_spinlock);
	enable();

	return 0;
}

static void cmd_reboot(int argc, char **argv)
{
	do_reboot();
}

static void cmd_help(int argc, char **argv)
{
	struct debugger_command *cmd;

	dprintf("debugger commands:\n");
	cmd = commands;
	while(cmd != NULL) {
		dprintf("%-32s\t\t%s\n", cmd->cmd, cmd->description);
		cmd = cmd->next;
	}
}


int dbg_init2(void)
{
	int ret;

	dbg_add_command(&cmd_help, "help", "List all debugger commands");
	dbg_add_command(&cmd_reboot, "reboot", "Reboot");
	dbg_add_command(&cmd_gdb, "gdb", "Connect to remote gdb. May supply an optional iframe.");

	ret = arch_dbg_con_init2();
	if(ret < 0)
		return ret;

	return 0;
}


int dbg_init(void)
{
	int ret;

	commands = NULL;

	ret = arch_dbg_con_init();
	if(ret < 0)
		return ret;
	return 0;
}



bool dbg_set_serial_debug(bool new_val)
{
	int temp = serial_debug_on;
	serial_debug_on = new_val;
	return temp;
}

bool dbg_get_serial_debug()
{
	return serial_debug_on;
}





#if 1 //ia32

static int hexval(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
		
	return 0;
}

unsigned long atoul(const char *num)
{
	int value = 0;
	if (num[0] == '0' && num[1] == 'x') {
		// hex
		num += 2;
		while (*num && isxdigit(*num))
			value = value * 16 + hexval(*num++);
	} else {
		// decimal
		while (*num && isdigit(*num))
			value = value * 10 + *num++  - '0';
	}

	return value;
}

void dbg_make_register_file(unsigned int *file, const regs_t * frame)
{
	file[0] = frame->eax;
	file[1] = frame->ebx;
	file[2] = frame->ecx;
	file[3] = frame->edx;
	file[4] = frame->user_esp;
	file[5] = frame->ebp;
	file[6] = frame->esi;
	file[7] = frame->edi;
	file[8] = frame->eip;
	file[9] = frame->eflags;
	file[10] = frame->cs;
	file[11] = frame->user_ss;
	file[12] = frame->ds;
	file[13] = frame->es;
}
#define _SERIAL_DBG_PORT 1

#if _SERIAL_DBG_PORT == 1
static const int serial_ioport_base = 0x3f8;
#elif _SERIAL_DBG_PORT == 2
static const int serial_ioport_base = 0x2f8;
#elif _SERIAL_DBG_PORT == 0xe9
#define BOCHS_E9_HACK 1
#else
#error _SERIAL_DBG_PORT set to unsupported com port
#endif

static const int dbg_baud_rate = 115200;


int arch_dbg_con_init()
{
#if !BOCHS_E9_HACK
	short divisor = 115200 / dbg_baud_rate;

	out8(0x80, serial_ioport_base+3);	/* set up to load divisor latch	*/
	out8(divisor & 0xf, serial_ioport_base);		/* LSB */
	out8(divisor >> 8, serial_ioport_base+1);		/* MSB */
	out8(3, serial_ioport_base+3);		/* 8N1 */
#endif
	return 0;
}

int arch_dbg_con_init2()
{
	return 0;
}

char arch_dbg_con_read(void)
{
#if BOCHS_E9_HACK
	return in8(0xe9);
#else
	while ((in8(serial_ioport_base+5) & 1) == 0)
		;
	return in8(serial_ioport_base);
#endif
}

static void _arch_dbg_con_putch(const char c)
{
#if BOCHS_E9_HACK
	out8(c, 0xe9);
#else

 	while ((in8(serial_ioport_base+5) & 0x20) == 0)
		;
	out8(c, serial_ioport_base);
 
#endif
}


char arch_dbg_con_putch(const char c)
{
	if (c == '\n') {
		_arch_dbg_con_putch('\r');
		_arch_dbg_con_putch('\n');
	} else if (c != '\r')
		_arch_dbg_con_putch(c);

	return c;
}

void arch_dbg_con_puts(const char *s)
{
	while(*s != '\0') {
		arch_dbg_con_putch(*s);
		s++;
	}
}

size_t arch_dbg_con_write(const void *buf, size_t len)
{
	const char *cbuf = (const char *)buf;
	size_t ret = len;

	while(len > 0) {
		arch_dbg_con_putch(*cbuf);
		cbuf++;
		len--;
	}
	return ret;
}


#endif
