#include <stdio.h>
#include <unistd.h>

void
DoStart(int writefd) {
    char buf[10] = "ho";
    if(write(writefd, buf, 10) == -1) {
        perror("DoStart write");
        exit(1);
    }
}

void
DoFilter(int readfd, int writefd) {
    char buf[10];
    if(read(readfd, buf, 10) == -1) {
        perror("DoFilter read");
        exit(1);
    }
    buf[2] = 'h'; buf[3] = 'o'; buf[4] = '\0';
    if(write(writefd, buf, 10) == -1 ) {
        perror("DoFilter write");
    }
}

void
DoEnd(int readfd){
    char buf[10];
    if(read(readfd, buf, 10) == -1) {
        perror("DoFilter read");
        exit(1);
    }
    printf("%s\n", buf);
}

int
main()
{
    int pd1[2], pd2[2];
    int pid1, pid2;

    if(pipe(pd1) == -1) {
        perror("Error creating pipe 1");
        exit(1);
    }
    if(pipe(pd2) == -1) {
        perror("Error creating pipe 2");
        exit(1);
    }

    pid1 = fork();
    if(pid1 == 0) {
        close(pd1[0]);
        DoStart(pd1[1]);
        close(pd1[1]);
        exit(0);
    } else {
        pid2 = fork();
        if(pid2 == 0) {
            close(pd1[1]); close(pd2[0]);
            DoFilter(pd1[0], pd2[1]);
            close(pd2[1]); close(pd1[0]);
        } else {
            close(pd2[1]);
            DoEnd(pd2[0]);
            close(pd2[0]);
            printf("done\n");
        }
    }
}


