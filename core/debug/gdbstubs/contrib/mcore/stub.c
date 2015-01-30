#include "mcore.h"
#include "mmc2107.h"

void
_start( void )
{
  main();
}

int 
main ( void )
{
  while (1) 
  { 
    gdb_breakpoint();
  }

  return 0;
}
