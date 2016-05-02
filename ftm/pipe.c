#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define  BUFF_SIZE   1024

int main(int argc, char *argv[])
{
	int   pipes[2];
	int   counter  = 0;
	pid_t pid;
	int	  sender = 1;

	if (argv[1][0] == 'r')
	{
		pipes[0] = atoi(argv[2]);
	}
	else 
	{
		if ( -1 == pipe(pipes))
		{
			perror( "parent pipe creation afiled.");
			exit( 1);
		}

		printf("%d %d\n", pipes[0], pipes[1]);
	}

	while( 1 )
	{
		if (sender)
		{
			char  buff[BUFF_SIZE];
			sprintf( buff, "message : %d\n", counter++);
			write( pipes[1], buff, strlen( buff));
		}
		else
		{
			char  buff[BUFF_SIZE];
			memset( buff, 0, BUFF_SIZE);
			read( pipes[0], buff, BUFF_SIZE);
			printf( "Receved : %s\n", buff);
		}
		sleep(1);
	}
}
