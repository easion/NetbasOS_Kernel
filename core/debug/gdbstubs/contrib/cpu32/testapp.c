#include "testapp.h"

static void fn2(void);
static void fn3(void);
static int fn4(int x);
static void infloop(void);

static int zzz;

void testapp(void)
{
  while (1)
  {
    infloop();
  }
}

static void infloop(void)
{
  int i, j;
  char s[10];

  s[3] = 0;

  for (i = 0; i < 10; i++)
  {
    for (j = 0; j < 5; j++)
    {
/*
      s[0] = i + '0';
      s[1] = j + '0';
      s[2] = '\n';
      gdb_cout(s);
*/

      if (j % 2)
      {
        fn2();
      }
      else
      {
        fn4(j);
      }
    }
  }
}

static void fn2(void)
{
  zzz += 3;

  fn3();
}

static void fn3(void)
{
  zzz -= 2;
}

static int fn4(int x)
{
  int a;

  a = x + 2;

  return a;
}

