#include "io.h"

#define com1 0x3f8 
#define com2 0x2f8

#define combase com1

void init_serial(void)
{
    outb(inb(combase + 3) | 0x80, combase + 3);
    outb(12, combase);                           /* 9600 bps, 8-N-1 */
    outb(0, combase+1);
    outb(inb(combase + 3) & 0x7f, combase + 3);
}

int getDebugChar(void)
{
    while (!(inb(combase + 5) & 0x01));
    return inb(combase);
}

void putDebugChar(int ch)
{
    while (!(inb(combase + 5) & 0x20));
    outb((char) ch, combase);
}

void exceptionHandler(int exc, void *addr)
{
    setInterruptGate(exc, addr);
}

void flush_i_cache(void)
{
   __asm__ __volatile__ ("jmp 1f\n1:");
}

/* 
void *memset(void *ptr, int val, unsigned int len); 

   needs to be defined if it hasn't been already
*/
