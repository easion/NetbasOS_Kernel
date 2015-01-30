#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER 255
int main(int argc,char **argv) {
        char buffer[BUFFER + 1];
        int fd[2];

        printf("Program start.\n");

        if(argc != 2) {
                fprintf(stderr,"Usage:%s string\n\a",argv[0]);
                exit(1);
        }

        if(pipe(fd) != 0) {
                fprintf(stderr,"Pipe Error:%s\n\a",strerror(errno));
                exit(1);
        }

        if(fork() == 0) {//当前进程为子进程
                close(fd[0]);
                printf("Child[%d] Write to pipe\n\a",getpid());
                snprintf(buffer,BUFFER,"%s",argv[1]);
                write(fd[1],buffer,strlen(buffer));
                printf("Chile[%d] Quit\n\a",getpid());
                exit(0);
        } else {//当前进程为父进程
                close(fd[1]);
                printf("Parent[%d] Read from pipe\n\a",getpid());
                memset(buffer,'\0',BUFFER + 1);
                read(fd[0],buffer,BUFFER);
                printf("Parent[%d] Read:%s\n",getpid(),buffer);
                exit(1);
        }
}

