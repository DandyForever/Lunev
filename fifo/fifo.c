#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#define PID_FIFO  "pid_fifo"
#define CHMOD 0644
#define FIFO_NAME_SIZE 100
#define BUFFSIZE 256
#define TIME_LIMIT 1000
#define SMALL_TIME 100

int reciever ();
int streamer (const char* file_name);

int main (int argc, char* argv[])
{
	if (argc == 1)
	{
		reciever ();
	}

	else if (argc == 2)
	{
		streamer (argv[1]);
	}

	else
	{
		printf ("argument invalid: too many arguments\n");
		return 1;
	}

	return 0;
}

int waiting_streamer (const int fd);

int reciever ()
{
	int pid_fifo = mkfifo (PID_FIFO, CHMOD);

	//
	
	pid_t pid = getpid ();

	//
	
	int pid_fd = open (PID_FIFO, O_RDWR);

	//
	
	write (pid_fd, &pid, sizeof (pid_t));
	//
	close (pid_fd);

	char fifo_name[FIFO_NAME_SIZE] = {0};

	sprintf (fifo_name, "process%d", pid);

	int recieve_fifo = mkfifo (fifo_name, CHMOD);

	//
	
	int fifo_fd = open (fifo_name, O_RDONLY);

	//
	
	if (waiting_streamer (fifo_fd) == -1)
	{
		printf ("streamer is invalid\n");
		return -1;
	}

	char buffer[BUFFSIZE] = {0};

	int count_symbols = -1;

	while (count_symbols != 0)
	{
		count_symbols = read (fifo_fd, buffer, BUFFSIZE);
		write (STDOUT_FILENO, buffer, count_symbols);
	}

	close (fifo_fd);
}

int waiting_streamer (const int fd)
{
	int bytes = -1;
	int time = 0;

	while (time < TIME_LIMIT)
	{
		usleep (SMALL_TIME);
		time += SMALL_TIME;

		ioctl (fd, FIONREAD, &bytes);

		if (bytes > 0) break;
	}

	if (bytes == 0)
		return -1;

	return 0;
}

int streamer (const char* file_name)
{
	int pid_fifo = mkfifo (PID_FIFO, CHMOD);

	//
	
	int pid_fd = open (PID_FIFO, O_RDWR);

	//
	
	pid_t pid = -1;
	
	int check_read = read (pid_fd, &pid, sizeof (pid_t));

	//
	
	close (pid_fd);

	char fifo_name[FIFO_NAME_SIZE] = {0};

	sprintf (fifo_name, "process%d", pid);

	int stream_fifo = mkfifo (fifo_name, CHMOD);

	//
	
	int fifo_fd = open (fifo_name, O_WRONLY);

	//
	
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
}
