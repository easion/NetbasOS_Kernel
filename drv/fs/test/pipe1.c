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
			printf("close fd[1]\n");
			close(fd[1]);
			printf("end fd[1]\n");
			read(fd[0], buffer, 6);
			printf("%s\n", buffer);
		}else{
			printf("close fd[0]\n");
			close(fd[0]);
			printf("end fd[0]\n");
			wait(&status);
		}
	}

	_exit(0);

	return 0;
}
