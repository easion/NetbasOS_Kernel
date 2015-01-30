

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

//#include  "KDEBUG.h"


#define BLOCK_SIZE      4000
#define NUM_BLOCKS      5

int errct = 0;
char inbuf[BLOCK_SIZE + 2]="hello\n";
char outbuf[BLOCK_SIZE + 2];

//-----------------------------
int  main(int argc, char **argv)
{
  int   i,  pid,  adrs;
  int   ch = 32;
  int   stat_loc;
  int   pipefd[2];
  int count;

  printf("ARGC= %d,  ARGV= %x  \n", argc, argv);

  for(i = 0; i < argc; i++)
    printf("arg[%d] %s  ", i, argv[i]);
  printf("\n");

  //  MBREAK("How are you ?");

  for (i=0; i<BLOCK_SIZE; i++)   {
    if ((ch++) > 126) ch = 32 ;
    //inbuf[i] = ch;
  }

  pipe(pipefd);

  if ((pid = fork()) != 0)
    {
      printf("parent  Parent  PArent  PARent\n");

      for (i = 0; i < NUM_BLOCKS; i++){
		 // inbuf[5]=
		 sprintf(inbuf,"Hello %d",i);
        write(pipefd[1], inbuf, BLOCK_SIZE);
	  }

	  printf("father send done\n");
      wait(&stat_loc);
	  printf("father  done\n");
    }
  else
    {
      printf("child   Child   CHild   CHIld \n");

      for (i = 0; i < NUM_BLOCKS; i++)
        {
		  count = read(pipefd[0], outbuf, BLOCK_SIZE);
          if (count != BLOCK_SIZE){
			  printf("error read %d bytes  \n", count);
            break;
		  }
          outbuf[128] = 0;
          printf("%s  \n", outbuf);
        }

      printf("Child exits \n");
      _exit(0);
    }

  printf("PID=%d    \n", pid);

   wait(0);
  return  0;
}
