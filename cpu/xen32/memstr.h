
 /** @addtogroup ia32	
 * @{
 */
/** @file
 */

#ifndef __ia32_MEMSTR_H__
#define __ia32_MEMSTR_H__

/** Copy memory
 *
 * Copy a given number of bytes (3rd argument)
 * from the memory location defined by 2nd argument
 * to the memory location defined by 1st argument.
 * The memory areas cannot overlap.
 *
 * @param dst Destination
 * @param src Source
 * @param cnt Number of bytes
 * @return Destination
 */
static inline void * memcpy(void * dst, const void * src, size_t cnt)
{
        unative_t d0, d1, d2;

        __asm__ __volatile__(
                /* copy all full dwords */
                "rep movsl\n\t"
                /* load count again */
                "movl %4, %%ecx\n\t"
                /* ecx = ecx mod 4 */
                "andl $3, %%ecx\n\t"
                /* are there last <=3 bytes? */
                "jz 1f\n\t"
                /* copy last <=3 bytes */
                "rep movsb\n\t"
                /* exit from asm block */
                "1:\n"
                : "=&c" (d0), "=&D" (d1), "=&S" (d2)
                : "0" ((unative_t) (cnt / 4)), "g" ((unative_t) cnt), "1" ((unative_t) dst), "2" ((unative_t) src)
                : "memory");

        return dst;
}


/** Compare memory regions for equality
 *
 * Compare a given number of bytes (3rd argument)
 * at memory locations defined by 1st and 2nd argument
 * for equality. If bytes are equal function returns 0.
 *
 * @param src Region 1
 * @param dst Region 2
 * @param cnt Number of bytes
 * @return Zero if bytes are equal, non-zero otherwise
 */
static inline int memcmp(const void * src, const void * dst, size_t cnt)
{
	uint32_t d0, d1, d2;
	int ret;
	
	__asm__ (
		"repe cmpsb\n\t"
		"je 1f\n\t"
		"movl %3, %0\n\t"
		"addl $1, %0\n\t"
		"1:\n"
		: "=a" (ret), "=%S" (d0), "=&D" (d1), "=&c" (d2)
		: "0" (0), "1" ((unative_t) src), "2" ((unative_t) dst), "3" ((unative_t) cnt)
	);
	
	return ret;
}

/** Fill memory with words
 * Fill a given number of words (2nd argument)
 * at memory defined by 1st argument with the
 * word value defined by 3rd argument.
 *
 * @param dst Destination
 * @param cnt Number of words
 * @param x Value to fill
 */
static inline void memsetw(uintptr_t dst, size_t cnt, uint16_t x)
{
	uint32_t d0, d1;
	
	__asm__ __volatile__ (
		"rep stosw\n\t"
		: "=&D" (d0), "=&c" (d1), "=a" (x)
		: "0" (dst), "1" (cnt), "2" (x)
		: "memory"
	);

}

/** Fill memory with bytes
 * Fill a given number of bytes (2nd argument)
 * at memory defined by 1st argument with the
 * word value defined by 3rd argument.
 *
 * @param dst Destination
 * @param cnt Number of bytes
 * @param x Value to fill
 */
static inline void memsetb(uintptr_t dst, size_t cnt, uint8_t x)
{
	uint32_t d0, d1;
	
	__asm__ __volatile__ (
		"rep stosb\n\t"
		: "=&D" (d0), "=&c" (d1), "=a" (x)
		: "0" (dst), "1" (cnt), "2" (x)
		: "memory"
	);

}

#endif

 /** @}
 */

