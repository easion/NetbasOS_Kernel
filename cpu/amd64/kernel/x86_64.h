#if !defined(_PRINTF_H_INCLUDED_)
#define _PRINTF_H_INCLUDED_


inline static unsigned char
inb(int port)
{
   register unsigned char r;
   
   __asm__ __volatile__( "inb %%dx,%%al\n\t" : "=a" (r) : "d" (port));
   return(r);
}

inline static void
outb(int port, unsigned char data)
{
   __asm__ __volatile__("outb %%al,%%dx\n\t" :: "a" (data), "d" (port));
}

#endif /* _PRINTF_H_INCLUDED_ */
