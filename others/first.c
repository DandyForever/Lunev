#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	char *endptr = 0, *str = 0;
        long long  val = 0;

        if (argc < 2) 
	{
        	printf("too few arguments\n");
        	return 2;
        }

	if (argc > 2)
	{
		printf ("too many arguments\n");
		return 2;
	}

	str = argv[1];

        errno = 0;
        val = strtoll(str, &endptr, 10);

	if (*endptr != '\0' || endptr == str)
	{
		printf ("Not an integer decimal number: %s\n", str);
		return 1;
	}

        if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))  || (errno != 0 && val == 0)) {
        	printf ("the number is too big\n");
                return 2;
        }
        	
	long long i = 0;
	
	if (val > 0)
		for (i = val; i >= 0; i--)
		{
			printf ("%lld\n", i);
		}
	
	else
		for (i = val; i <= 0; i++)
		{
			printf ("%lld\n", i);
		}

	return 0;
       }
