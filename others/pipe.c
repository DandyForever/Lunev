#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define BUFF_SIZE  256

int main(int argc, char *argv[])
{
	int pipefd[2] = {0};
	pid_t cpid = 0;
	char bufin[BUFF_SIZE] = {0};
	char bufout[BUFF_SIZE] = {0};

	if (argc != 2) 
	{
		fprintf(stderr, "Usage: %s <string>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (pipe(pipefd) == -1) 
	{
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	cpid = fork();
	if (cpid == -1) 
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (cpid == 0) 
	{    
		close (pipefd[0]);

		int fd = open (argv[1], O_RDONLY);
		if (fd == -1)
		{
			printf ("file %s can't be opened", argv[1]);
			exit(-1);
		}

		int count_read = BUFF_SIZE;
		while (count_read != 0)
		{
			count_read = read (fd, bufin, BUFF_SIZE);

			if (count_read == -1)
			{
				printf ("read ERROR\n");
				exit (EXIT_FAILURE);
			}

			if (write (pipefd[1], bufin, count_read) == -1)
			{
				printf ("write ERROR\n");
				exit (EXIT_FAILURE);
			}
		}
		
		close (fd);
		close (pipefd[1]);
		exit (EXIT_SUCCESS);
	} 
	else 
	{       
		close (pipefd[1]);
		
		int count_write = BUFF_SIZE;
		while (count_write != 0)
		{
			count_write = read (pipefd[0], bufout, BUFF_SIZE);

			if (count_write == -1)
			{
				printf ("read ERROR\n");
				exit (EXIT_FAILURE);
			}

			if (write (STDOUT_FILENO, bufout, count_write) == -1)
			{
				printf ("write ERROR\n");
				exit (EXIT_FAILURE);
			}
		}

		close (pipefd[0]);
		wait (NULL);
		exit(EXIT_SUCCESS);
	}
}

