#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 2
#include <stdio.h>
#undef _POSIX_C_SOURCE
#endif
#include <stdlib.h>

#define COMMAND "ls"

#define BUFFER_SIZE 1024

int main()
{
   char buffer[BUFFER_SIZE];
   FILE *fp = popen(COMMAND, "r");
   if (fp == NULL) {
      perror(COMMAND);
      return EXIT_FAILURE;
   }
   printf("gots line %d ..\n",__LINE__);

   do {
	   memset(buffer, 0, BUFFER_SIZE);
      size_t n = fread(buffer, 1, BUFFER_SIZE, fp);

   printf("gots line %d ,got%d bytes..\n",__LINE__,n);
      if (ferror(fp)) {
         perror("cannot read");
         break;
      }
      fwrite(buffer, 1, n, stdout);
   }
   while (!feof(fp));

   printf("gots line %d ..\n",__LINE__);
   pclose(fp);
   return EXIT_SUCCESS;
}

