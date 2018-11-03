#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

int Sum = 0;

void* increament (void* args)
{
	int k = * (int*) args;
	for (int i = 0; i < k; i++)
	{
		Sum++;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	char *endptr1 = 0, *str1 = 0, *str2 = 0, *endptr2 = 0;
        int  val = 0, k = 0;

        if (argc < 3) 
	{
        	printf("too few arguments\n");
        	return 2;
        }

	if (argc > 3)
	{
		printf ("too many arguments\n");
		return 2;
	}

	str1 = argv[1];
	str2 = argv[2];

        errno = 0;
        val = strtoll(str1, &endptr1, 10);
	k = strtoll(str2, &endptr2, 10);

	if (*endptr1 != '\0' || endptr1 == str1 || *endptr2 != '\0' || endptr2 == str2)
	{
		printf ("Not an integer decimal number: \n");
		return 1;
	}

        if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))  || (errno != 0 && val == 0)) 
	{
        	printf ("the number is too big\n");
                return 2;
        }
	if (val <= 0 || k <= 0)
	{
		printf ("the number(s) are < 0\n");
		return 3;
	}
        	
	pthread_t threadID[100000];

	for (int i = 0; i < val; i++)
	{
		pthread_create (threadID + i, NULL, increament, (void*) &k);
	}

	for (int i = 0; i < val; i++)
	{
		pthread_join (threadID[i], NULL);
	}

	printf ("%d\n", Sum);

	return 0;
}
