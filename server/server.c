#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <fcntl.h>

#define RD 0
#define WR 1
#define CH_BS 131072
#define PT_BS 1024
#define CHECK(cond, msg, err)				\
	do {						\
		if (cond)				\
		{					\
			if (err) errno = err;		\
			printf ("%d:", __LINE__);	\
			perror (#msg);			\
			exit (-1);			\
		}					\
	} while (0)	

typedef struct rustem
{
	int from[2];
	int to[2];
	char* buffer;
	char* next;
	int buf_size;
	int num_bytes;
} child_;

child_* proxy =  NULL;

int parse_n (char* str);
void create (const int val, const int fd);
void parent (const int n);
void child  (const int n, const int current);

int main (int argc, char* argv[])
{
	CHECK (argc != 3, main, EINVAL);

	int val = parse_n (argv[1]);
 	
	val += 2;

	int fd = open (argv[2], O_RDONLY);
	CHECK (fd == -1, open, 0);

	create (val, fd);
	
	int ret = -1;

	for (int i = 1; i < val - 1; i++)
	{
		ret = fork ();
		CHECK (ret == -1, fork, 0);
		
		if (!ret)
			child (val, i);
	}

	parent (val);

	return 0;
}

int parse_n (char* str)
{
	char* endptr = 0;	
	int val = 0;
	errno = 0;

        val = strtoll(str, &endptr, 10);

	CHECK(*endptr != '\0' || endptr == str, parse_n, EINVAL);

	CHECK((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))  ||
		 (errno != 0 && val == 0), parse_n, 0);

	return (int) val;
}

void create (const int val, const int fd)
{
	proxy = calloc (val, sizeof (*proxy));

	for (int i = 1; i < val - 1; i++)
	{
		CHECK (pipe (proxy[i].from) == -1, pipe, 0);
		CHECK (pipe (proxy[i].to  ) == -1, pipe, 0);
		
		CHECK (fcntl (proxy[i].from[RD], F_SETFL, O_RDONLY | O_NONBLOCK) == -1, fcntl, 0);
		CHECK (fcntl (proxy[i].to[WR], F_SETFL, O_WRONLY | O_NONBLOCK) == -1, fcntl, 0);
	}

	proxy[0].from[RD] = fd;
	proxy[0].from[WR] = -1;
	proxy[0].to[RD]   = -1;
	proxy[0].to[WR]   = -1;

	proxy[val - 1].from[RD] = -1;
	proxy[val - 1].from[WR] = -1;
	proxy[val - 1].to[RD]   = -1;
	proxy[val - 1].to[WR]   = STDOUT_FILENO;
}

void child (const int n, const int current)
{	
	for (int i = 1; i < n - 1; i++)
	{
		close (proxy[i].from[RD]);
		close (proxy[i].to[WR]);

		if (i != current)
		{
			close (proxy[i].from[WR]);
			close (proxy[i].to[RD]);
		}
	}

	char buffer[CH_BS] = {};
	int ret = 0;

	do {
		ret = read (proxy[current].to[RD], buffer, CH_BS);
		CHECK(ret == -1, read, 0);
		CHECK(write (proxy[current].from[WR], buffer, ret) == -1, write, 0);
	} while (ret);

	close (proxy[current].to[RD]);
	close (proxy[current].from[WR]);

	free (proxy);
	exit (EXIT_SUCCESS);
}

void reading (const int current);
void writing (const int current);

void parent (const int n)
{
	int pow_3 = 3;

	for (int i = n - 2; i > 0; i--)
	{
		close (proxy[i].from[WR]);
		close (proxy[i].to[RD]);

		if (PT_BS * pow_3 < CH_BS)
			proxy[i].buf_size = PT_BS * pow_3;
		else
			proxy[i].buf_size = CH_BS;
		pow_3 *= 3;
		proxy[i].buffer = calloc (proxy[i].buf_size, sizeof (*proxy[i].buffer));
		proxy[i].next = proxy[i].buffer;
	}

	proxy[n - 1].buf_size = PT_BS;
	proxy[n - 1].buffer = calloc (proxy[n - 1].buf_size, 
			sizeof (*proxy[n - 1].buffer));
	proxy[n - 1].next = proxy[n - 1].buffer;

	int nfds = 0;
	while (1)
	{
		fd_set readfds, writefds;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);

		int counter = 0;
		for (int i = 0; i < n; i++)
		{
			if (proxy[i].from[RD] > nfds) nfds = proxy[i].from[RD];
			if (proxy[i].to[WR] > nfds)   nfds = proxy[i].to[WR];

			if (proxy[i].from[RD] != -1 && proxy[i + 1].num_bytes == 0)
			{
				FD_SET(proxy[i].from[RD], &readfds);
				counter++;	
			}

			if (proxy[i].to[WR] != -1 && proxy[i].num_bytes != 0)
			{
				FD_SET(proxy[i].to[WR], &writefds);
				counter++;
			}
		}
		
		if (!counter) break;

		CHECK(select (nfds + 1, &readfds, &writefds, NULL, NULL) == -1, select, 0);
		
		for (int i = 0; i < n; i++)
		{
			if (FD_ISSET(proxy[i].from[RD], &readfds)) reading (i);
			if (FD_ISSET(proxy[i].to[WR]  , &writefds)) writing (i);	
		}	
	}

	for (int i = 1; i < n; i++)
		free (proxy[i].buffer);

	free (proxy);
	exit (EXIT_SUCCESS);
}

void reading (const int current)
{
	int ret = read (proxy[current].from[RD], 
			proxy[current + 1].buffer, proxy[current + 1].buf_size);
	CHECK(ret == -1, read, 0);
	proxy[current + 1].num_bytes += ret;

	if (!ret)
	{
		CHECK(close (proxy[current].from[RD]) == -1, close, 0);
		if (proxy[current + 1].to[WR] != 1)
			CHECK(close (proxy[current + 1].to[WR]) == -1, close, 0);
		proxy[current].from[RD] = -1;
		proxy[current + 1].to[WR] = -1;
	}	
}

void writing (const int current)
{
	int ret = -1;
	ret = write (proxy[current].to[WR], proxy[current].next, 
			proxy[current].num_bytes);
	CHECK(ret == -1, write, 0);
		
	proxy[current].num_bytes -= ret;
	proxy[current].next += ret;

	if (!proxy[current].num_bytes)
		proxy[current].next = proxy[current].buffer;
}
