#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>


#define RD 0
#define RW 1
#define CH_BS 131072
#define CHECK(cond, msg, err)			\
	do {					\
		if (cond)			\
		{				\
			if (err) errno = err;	\
			perror (#msg);		\
			exit (-1);		\
		}				\
	} while (0)	

typedef struct rustem
{
	int from[2];
	int to[2];
	char* buffer;
	int buf_size;
} child_;

child_* proxy =  NULL;

int parse_n (char* str);
void create (const int val, const int fd);

int main (int argc, char* argv[])
{
	CHECK (argc != 3, main, EINVAL);

	int val = parse_n (argv[1]);
 	
	val += 2;

	int fd = open (argv[2], O_RDONLY);
	CHECK (fd == -1);

	create (val, fd);

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
	proxy = calloc (val, sizeof (*child));

	for (int i = 1; i < val - 1; i++)
	{
		CHECK (pipe (proxy[i].from) == -1);
		CHECK (pipe (proxy[i].to  ) == -1);
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
		CHECK(write (proxy[current].from[WR], buffer, ret) == -1);
	} while (ret);

	close (proxy[current].to[RD]);
	close (proxy[current].from[WR]);

	free (proxy);
	exit (EXIT_SUCCESS);
}
