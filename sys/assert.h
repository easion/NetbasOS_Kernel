
//#undef ASSERT

//#define __ENABLE_ASSERT__

#ifndef __ENABLE_ASSERT__
#define ASSERT(test)
#else
void __assertion(const char* msg, const char* file, int line, char *);
#define ASSERT(test)((test)?0: __assertion(#test,__FILE__, __LINE__, NULL))
#define MSGASSERT(test, s)((test)?0: __assertion(#test,__FILE__, __LINE__, s))
#endif 

#ifndef _ASSERT_H
#define _ASSERT_H


#include <jicama/system.h>



#ifdef __cplusplus
extern "C" {
void __assertion(const char* msg, const char* file, int line);
#define BUG(info) __assertion(info, __FILE__, __LINE__)

#endif


#ifdef __cplusplus
}
#endif

#endif /* _ASSERT_H */
