#ifndef GDBCPU32_H
#define GDBCPU32_H

/* cause software breakpoint; we use trap #0 here because gdb uses trap #1
   and we need to be able to tell the difference (we expect gdb to replace
   its trap #1 instructions with valid code after a placed breakpoint is
   hit; compiled-in breakpoints are not replaced with anything, however */
#define COMPILED_IN_BREAKPOINT asm(" trap #0")

void gdb_interrupt_handler(void);
void gdb_cout ( const char *buf );

#endif /* GDBCPU32_H */
