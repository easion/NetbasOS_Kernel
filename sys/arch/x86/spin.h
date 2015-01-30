#ifndef _ARCH_SPINLOCK_H_
#define _ARCH_SPINLOCK_H_

static inline void spin_lock( spinlock_t *lock )
{
	__asm__ __volatile__(
		"1: lock ; decb %0\n"
		"jge 3f\n"
		"2:\n"
		"cmpb $0, %0\n"
		"rep; nop\n"
		"jle 2b\n"
		"sti\n"
		"jmp 1b\n"
		"3:\n"
		: "=m"(lock->lock) : : "memory");
}


static inline void spin_unlock( spinlock_t *lock )
{
	__asm__ __volatile__(
		"movb $1, %0"
		: "=m"(lock->lock) : : "memory");
}

#endif
