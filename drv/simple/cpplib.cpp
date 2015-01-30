
#include <drv/drv.h>
#include <drv/cpplib.h>

__api_link void __pure_virtual()
{
	panic("pure virtual function called\n"); 
}

__api_link void __cxa_pure_virtual()
{
	panic("pure virtual function called\n"); 
}

__api_link void __cxa_atexit()
{
	panic("__at_exit called\n"); 
}

__api_link void __dso_handle()
{
	panic("__dso_handle called\n");
}


__api_link void _Unwind_Resume()
{
	printk("%s call\n", __FUNCTION__);
}

__api_link void __gxx_personality_v0()
{
	printk("%s call\n", __FUNCTION__);
}


