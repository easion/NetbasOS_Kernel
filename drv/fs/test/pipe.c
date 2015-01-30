#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(void)
{
	int fd[2];
	char buffer[32];

	if(pipe(fd) == 0){
		int status;

		if(fork() == 0){
			close(fd[1]);
			for(;;){
				read(fd[0], buffer, 6);
				printf("%s\n", buffer);
			}
		}else{
			close(fd[0]);
			for(;;)
				write(fd[1], "hello", 6);
			wait(&status);
		}
	}

	return 0;
}
