#include "gdb.h"

#if defined(H8300_H8S2148EDK)
#include "h8300.h"
#endif

int main (void)
{
  gdb_startup();

  while (1)
    {
      gdb_breakpoint();
      gdb_console_output(24, "gdbstubs: hello, world!\n");
    }
}

