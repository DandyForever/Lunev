#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

int consumer ();
int producer (const char* file_name);

int main (int argc, char* argv[])
{

	int ret = 0;
	if (argc == 1)
	{
		ret = consumer ();
	}

	else if (argc == 2)
	{
		ret = producer (argv[1]);
	}

	else
	{
		printf ("argument invalid: too many arguments\n");
		return 1;
	}

	return 0;
}


int consumer ()
{
	
}

int consumer (const char* file_name)
{

}

