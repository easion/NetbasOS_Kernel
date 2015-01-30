#include <stdio.h>

int x = 10;
int y;

int foo (int fooarg)
	{
	fooarg = fooarg + 1;
	return fooarg;
	}

int main(void)
	{
	volatile int z;

	while(1)
		{
		printf("Hello, world!\n");

		foo(0x1234);
		
		for(z = 0; z < 10; z++)
			{
			y = x * z;
			foo(y);
			}
		}

	return 0;
	}


int atexit(void* v) { return 0; }
