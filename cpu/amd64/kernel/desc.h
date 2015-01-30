#if !defined(_IDT_H_INCLUDED_)
#define _IDT_H_INCLUDED_

#define AMD64_KCS    0x28   /* 1, RPL = 0 */
#define AMD64_KDS    0x30  /* 2, RPL = 0 */

#if 0
#define AMD64_INVS   0x0   /* 0 */
#define AMD64_KCS    0x8   /* 1, RPL = 0 */
#define AMD64_KDS    0x10  /* 2, RPL = 0 */
#define AMD64_UCS32  0x1b  /* 3, RPL = 3 */
#define AMD64_UDS    0x23  /* 4, RPL = 3 */
#define AMD64_UCS    0x2b  /* 5, RPL = 3 */
#define AMD64_UTCBS  0x33  /* 6, RPL = 3 */
#define AMD64_KDBS   0x38  /* 7, RPL = 0 */
#define AMD64_TBS    0x43  /* 8, RPL = 0 */
#define AMD64_TSS    0x48  /* 9, RPL = 0 */
#endif

struct __idt {
	u16_t size;
	u64_t address;
};

struct __gdt_table {
	u64_t limitL : 16;
	u64_t baseL  : 16;
	u64_t baseM  :  8;
	u64_t type   :  8;
	u64_t limitH :  8;
	u64_t baseH  :  8;
};

struct __gdt {
	u64_t limit : 16;
	u64_t base  : 32 __attr_packet;
	struct __gdt_table table[11];
};

struct __descriptor {
	u64_t offset_low  : 16;
	u64_t selector    : 16;
	u64_t ist         :  3;
	u64_t res0        :  5;
	u64_t type        :  4;
	u64_t s           :  1;
	u64_t dpl         :  2;
	u64_t p           :  1;
	u64_t offset_high : 48 __attr_packet;
	u64_t res1        : 32;
};

void loadIDT(u16_t size, u64_t address);
void loadIDTR(u16_t selector);
void set_handler(int index, u16_t selector, void (*address)(), int type, int dpl, int ist);
void add_int_gate(u16_t index, void (*address)());
void add_syscall_gate(u16_t index, void (*address)());
void add_trap_gate(u16_t index, void (*address)());
void error_handler(void);
void syscall_handler(void);
void init_handler();

#endif /* _IDT_H_INCLUDED_ */
