#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>

#define _str(x) #x
#define str(x) _str(x)
#define CHECK(cond, func)					\
	do{							\
		if (cond)					\
		{						\
			perror(#func " at " str(__LINE__));	\
			exit(-1);				\
		}						\
	}while(0)			

#define PID_FIFO  "pid_fifo"
#define CHMOD 0666
#define FIFO_NAME_SIZE 100
#define BUFFSIZE 256
#define TIME_LIMIT 1000
#define SMALL_TIME 100

int consumer ();
int producer (const char* file_name);

int main (int argc, char* argv[])
{
	if (argc == 1)
	{
		consumer ();
	}

	else if (argc == 2)
	{
		producer (argv[1]);
	}

	else
	{
		errno = EINVAL;
		perror("main");
		return 1;
	}

	return 0;
}

int waiting_producer (const int fd);

int consumer ()
{
	int pid_fifo = mkfifo (PID_FIFO, CHMOD);
	CHECK(pid_fifo == -1 && errno != EEXIST, mkfifo);
	
	pid_t pid = getpid ();
	
	int pid_fd = open (PID_FIFO, O_WRONLY);
	CHECK(pid_fd == -1, open);
		
	char fifo_name[FIFO_NAME_SIZE] = {0};

	CHECK(sprintf (fifo_name, "process%d", pid) < 0, sprintf);

	int recieve_fifo = mkfifo (fifo_name, CHMOD);
	CHECK(recieve_fifo == -1 && errno != EEXIST, mkfifo);
	
	int fifo_fd = open (fifo_name, O_RDONLY | O_NONBLOCK);
	CHECK(fifo_fd == -1, open);

	int wrtpid = write (pid_fd, &pid, sizeof (pid_t));
	CHECK(wrtpid == -1, write);
	CHECK(close (pid_fd) == -1, close);

	CHECK(waiting_producer (fifo_fd) == -1, consumer);
	
	fcntl(fifo_fd, F_SETFL, O_RDONLY);

	char buffer[BUFFSIZE] = {0};

	int count_symbols = -1;

	while (count_symbols != 0)
	{
		count_symbols = read (fifo_fd, buffer, BUFFSIZE);
		write (STDOUT_FILENO, buffer, count_symbols);
	}

	CHECK(close (fifo_fd) == -1, close);
       	CHECK(unlink (fifo_name), unlilnk);	
	
	return 0;
}

int waiting_producer (const int fd)
{
	int bytes = -1;
	int time = 0;

	struct pollfd pfd = { 
		.fd = fd,
		.events = POLLIN
	};

	if(poll(&pfd, 1, TIME_LIMIT) == 0) {
		errno = ETIME;
		return -1;
	}
	
	return 0;
}

int producer (const char* file_name)
{
	int pid_fifo = mkfifo (PID_FIFO, CHMOD);
	CHECK(pid_fifo == -1 && errno != EEXIST, mkfifo);
	
	int pid_fd = open (PID_FIFO, O_RDWR);
	CHECK(pid_fd == -1, open);
	
	pid_t pid = -1;
	
	int check_read = read (pid_fd, &pid, sizeof (pid_t));
	CHECK(check_read == -1, read);
	
	CHECK(close (pid_fd) == -1, close);

	char fifo_name[FIFO_NAME_SIZE] = {0};

	CHECK(sprintf (fifo_name, "process%d", pid) == -1, sprintf);
	
	int fifo_fd = open (fifo_name, O_WRONLY | O_NONBLOCK);
	CHECK(fifo_fd == -1 && errno == ENXIO, open);
	
	fcntl (fifo_fd, F_SETFL, O_WRONLY);

	char buffer[BUFFSIZE] = {0};
	
	int streaming_file = open (file_name, O_RDONLY);

	int count_symbols = -1;

	while (count_symbols != 0)
	{
		count_symbols = read (streaming_file, buffer, BUFFSIZE);
		write (fifo_fd, buffer, count_symbols);
	}

	close (fifo_fd);
	close (streaming_file);

	return 0;
}

