
#ifndef __CPP_DRV_LIB_H__
#define __CPP_DRV_LIB_H__


#ifndef __cplusplus
#error only c++
#endif

#include <drv/spin.h>

#define __api_link extern "C" __attribute__((regparm(0)))

__api_link void __static_initialization_and_destruction_0(int, int);


static inline void* operator new(size_t size)
{
	unsigned flags;
	spinlock_t s_lock;

	spinlock_init(&s_lock, "new_lock");

	flags = spinlock_disable(&s_lock);
    void* p = kmalloc(size,0);
    spinunlock_enable(&s_lock, flags);
    return p;
}

static inline void operator delete(void* addr)
{
	unsigned flags;
	spinlock_t s_lock;

	spinlock_init(&s_lock, "delete_lock");

	flags = spinlock_disable(&s_lock);
	kfree(addr);
    spinunlock_enable(&s_lock, flags);
    return;
}

static inline void* operator new[](size_t size)
{
	unsigned flags;
	spinlock_t s_lock;

	spinlock_init(&s_lock, "new_lock[]");

	flags = spinlock_disable(&s_lock);
	void* p = kmalloc(size,0);
    spinunlock_enable(&s_lock, flags);
    return p;
}

static inline void operator delete[](void* addr)
{
	unsigned flags;
	spinlock_t s_lock;

	spinlock_init(&s_lock, "delete_lock[]");

	flags = spinlock_disable(&s_lock);
	kfree(addr);
    spinunlock_enable(&s_lock, flags);
    return;
}

#endif //__CPP_DRV_LIB_H__


