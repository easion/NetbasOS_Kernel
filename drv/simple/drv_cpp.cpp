
// Jicama OS cpp Loadable Modules Test
// 2006-10-21

#include <drv/drv.h>
#include <drv/cpplib.h>
#include <drv/timer.h>

#define __ctor(pri,sub,desc) static void __ctor2006_##pri##sub##_##desc ()
__ctor(100, 200, init1)
{
	printk("%s called(100)\n", __FUNCTION__);
}

__ctor(200, 200, init2)
{
	printk("%s called(200)\n", __FUNCTION__);
}

static char *banner="DLL INIT:";


class cpp_drv
{
public:
	cpp_drv()
	:name("name1")
	{
	 printf("%s(No Argement)\n",banner);
	}

	cpp_drv(char *name)
	:name("name2")
	{
	 printf("%s(Argement %s)\n",banner, name);
	}

	~cpp_drv()
	{
	 puts("DLL Exiting!\n");
	}
	void hello(){
		printf("%s: %s\n",name,"hello world");
	}
private:
		char *name;
};

cpp_drv drv;
krnl_timer_t kt_test;

void test_timer(void *arg)
{
	printf("time restart after 6 sec(%s)...\n", (char*)arg);
	restart_timer(&kt_test, 6000);
}


/*dll entry*/
__api_link int dll_main(char **argv)
{
	int i=0;
	//初始化全局的静态变量。内核已经做了这个工作
	//__static_initialization_and_destruction_0(1,0xffff);

	while (argv&&argv[i]){
		printf("%s\n", argv[i]);
		i++;
	}

	cpp_drv *cpp = new cpp_drv(__FILE__);
	cpp->hello();
	delete cpp;
	init_timer(&kt_test,test_timer,(void*)"Test" );
	install_timer(&kt_test,2000);
	return 0;
}


__api_link int dll_destroy()
{
	printf("dll_destroy called!\n");
	remove_timer(&kt_test);
	//释放全局的静态变量,内核已经做了这个工作
	//__static_initialization_and_destruction_0(0,0xffff);
	return 0;
}

__api_link int dll_version()
{
	printf("CPP DRIVER VERSION 0.01!\n");
	return 0;
}
