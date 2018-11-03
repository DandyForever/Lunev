#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>

#define MSGFLG 0666


struct msgbuf {
	long mtype;
};

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

	if (val <= 0)
	{
		printf ("the number is bad\n");
		return 3;
	}
        
	/*pid_t status = 0, id = 1, iwait = 0;
	for (int i = 0; i < val; i++)
	{
		if (id != 0)
		{
			id = fork ();
			if (id == 0)
				printf ("%d %d\n", i, getpid());
			iwait = wait (&status);
		}
		else
		{
			exit(0);
		}

	}*/

	int msqid = msgget (IPC_PRIVATE, MSGFLG);
	if (msqid == -1)
	{
		printf ("msgget failure: returned -1\n");
		return 1;
	}
	
	int i = -1;
	pid_t pid = -1;
	val++;

	struct msgbuf msg = {0};
	msg.mtype = 1;
	if (msgsnd (msqid, &msg, 0, 0) == -1)
	{
		printf ("msgsnd failure: returned -1\n");
		exit (1);
	}


	for (i = 1; i < val; i++)
	{
		pid = fork ();
		if (pid == -1)
		{
			printf ("fork failure: returned -1\n");
			exit (1);
		}

		if (pid == 0) break;
	}

	/*if (i == val)
	{
		struct msgbuf msg = {0};
		msg.mtype = 1;
		if (msgsnd (msqid, &msg, 0, 0) == -1)
		{
			printf ("msgsnd failure: returned -1\n");	
			exit (1);
		}

		for (int j = 1; j < val; j++) wait(NULL);
	}*/

	if (i < val)
	{
		struct msgbuf msg = {0};
		if (msgrcv (msqid, &msg, 0, i, 0) == -1)
		{
			printf ("msgrcv failure: returned -1\n");
			exit (1);
		}

		printf ("%d ", i);
		fflush (stdout);

		msg.mtype = i + 1;
		if (msgsnd (msqid, &msg, 0, 0) == -1)
		{
			printf ("msgsnd failure: returned -1\n");
			exit (1);
		}
		exit (0);
	}

	else
	{
		if (msgrcv (msqid, &msg, 0, val, 0) == -1)
		{
			printf ("msgrcv failure: returned -1\n");
			exit (1);
		}

		msgctl (msqid, IPC_RMID, NULL);
		printf ("\n");
	}

	return 0;
}
