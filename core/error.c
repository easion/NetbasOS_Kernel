/*
** system error conter handler
*/

#include <jicama/system.h>
#include <assert.h>




/*
**
*/
__public void __assertion(const char* msg, const char* file, int line, char *err)
{

  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace (array, 5);
#if 1

  backtrace_symbols (array, size);

  //kprintf ("Obtained %d stack frames.\n", size);

  //for (i = 0; i < size; i++)
   //  kprintf ("%s\n", strings[i]);
#endif

   kprintf("The Kernel Assertion failed at %s line %d: %s\n", file, line, msg);
   if (err){
	   kprintf("fllow error message: %s\n", err);
   }
   disable();
   while(1);
}

/*
**
*/
void compiler_error(char *msg)
{
   kprintf("The Kernel Assertion failed:\n at %s line %d", __FILE__, __LINE__); 
   kprintf("Message:%s\n",  msg);
   disable();
   while(1);
}

/*
**
*/
int error_hander()
{
	return 1;
}

