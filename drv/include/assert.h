
#undef assert
#ifdef __cplusplus
extern "C" {
#endif

#define __ASSERT_DEBUG__

static inline void __assertion(const char* msg, const char* file, int line, char *err)
{
   kprintf("The Kernel Assertion failed at %s line %d: %s\n", file, line, msg);
   if (err){
	   kprintf("fllow error message: %s\n", err);
   }
   disable();
   while(1);
}

#ifndef __ASSERT_DEBUG__
#define assert(test)
#else
void __assertion(const char* msg, const char* file, int line, char *);
//#define assert(test)((test)?0: __assertion(#test,__FILE__, __LINE__, (char*)0))
#define assert(test)  do{\
	if(!(test))__assertion(#test,__FILE__, __LINE__, (char*)0);\
}\
while (0)

#define MSGASSERT(test, s)((test)?0: __assertion(#test,__FILE__, __LINE__, s))
#endif 



#ifdef __cplusplus
}
#endif

