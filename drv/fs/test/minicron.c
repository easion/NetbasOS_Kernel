
/*
	minicron.c
	part of m0n0wall (http://m0n0.ch/wall)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* usage: minicron interval pidfile cmd */

int main(int argc, char *argv[]) {
	
	int interval;
	FILE *pidfd;
	
	if (argc < 4)
		exit(1);
	
	interval = atoi(argv[1]);
	if (interval == 0)
		exit(1);
	
	/* unset loads of CGI environment variables */
	unsetenv("CONTENT_TYPE"); unsetenv("GATEWAY_INTERFACE");
	unsetenv("REMOTE_USER"); unsetenv("REMOTE_ADDR");
	unsetenv("AUTH_TYPE"); unsetenv("SCRIPT_FILENAME");
	unsetenv("CONTENT_LENGTH"); unsetenv("HTTP_USER_AGENT");
	unsetenv("HTTP_HOST"); unsetenv("SERVER_SOFTWARE");
	unsetenv("HTTP_REFERER"); unsetenv("SERVER_PROTOCOL");
	unsetenv("REQUEST_METHOD"); unsetenv("SERVER_PORT");
	unsetenv("SCRIPT_NAME"); unsetenv("SERVER_NAME");
	
	/* go into background */
#ifdef HAVE_DAEMON
	if ( daemon( 1, 1 ) < 0 )
	    {
	    exit( 1 );
	    }
#else /* HAVE_DAEMON */
	switch ( fork() )
	    {
	    case 0:
	    break;
	    case -1:
	    //syslog( LOG_CRIT, "fork - %m" );
	    exit( 1 );
	    default:
	    exit( 0 );
	    }
#ifdef HAVE_SETSID
        (void) setsid();
#endif /* HAVE_SETSID */
#endif /* HAVE_DAEMON */

	if (strcmp(argv[2],"none"))
	{	
		/* write PID to file */
		pidfd = fopen(argv[2], "w");
		if (pidfd) {
			fprintf(pidfd, "%d\n", getpid());
			fclose(pidfd);
		}
	}
		
	while (1) {
		usleep(interval*1000);
		
		system(argv[3]);
	}
}


