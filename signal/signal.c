#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFSIZE 128

#define KILL(pid, SIG)				\
	do {					\
		if (kill (pid, SIG) == -1)	\
		{				\
			perror ("kill");	\
			exit (-1);		\
		}				\
	} while (0)

#define ALARM					\
	do {					\
		alarm (1);			\
		sigsuspend (&Set);		\
		alarm (0);			\
	} while (0)

#define WRITE					\
	do {					\
		int bytes = 0;			\
		int shift = 0;			\
		do {				\
			bytes = write (STDOUT_FILENO, Buffer + shift, index);\
			shift += bytes;		\
			index -= bytes;		\
		} while (index);		\
	} while (0)

char Buffer[BUFFSIZE];
char Bit = 0;
sigset_t Set;
int index = 0;

int child (pid_t pid, const char* file);
int parent (pid_t pid);
void u1_hdlr ();
void u2_hdlr ();
void ch_hdlr ();

int main (int argc, char* argv[])
{
	struct sigaction u1_act = {0};
	u1_act.sa_handler = u1_hdlr;
	sigemptyset (&u1_act.sa_mask);
	sigaction (SIGUSR1, &u1_act, NULL);

	struct sigaction u2_act = {0};
	u2_act.sa_handler = u2_hdlr;
	sigemptyset (&u2_act.sa_mask);
	sigaction (SIGUSR2, &u2_act, NULL);

	struct sigaction ch_act = {0};
	ch_act.sa_handler = ch_hdlr;
	sigemptyset (&ch_act.sa_mask);
	sigaction (SIGCHLD, &ch_act, NULL);

	struct sigaction al_act = {0};
	al_act.sa_handler = exit;
	sigemptyset (&al_act.sa_mask);
	sigaction (SIGALRM, &al_act, NULL);

	sigfillset (&Set);
	sigdelset (&Set, SIGINT);
	sigdelset (&Set, SIGCHLD);
	sigdelset (&Set, SIGALRM);

	sigprocmask (SIG_SETMASK, &Set, NULL);
	
	sigdelset (&Set, SIGUSR1);
	sigdelset (&Set, SIGUSR2);
	
	pid_t ppid = getpid();	
	pid_t pid = fork ();

	if (pid == -1)
	{
		perror ("fork");
		exit(-1);
	}

	if (pid == 0)
	{
		child (ppid, argv[1]);
	} else 
	{
		parent (pid);
	}

	return 0;
}

int child (pid_t pid, const char* file)
{
	int fd = open (file, O_RDONLY);
	if (fd == -1)
	{
		perror ("read");
		exit (-1);
	}

	int ret_read = 0;
	do {
		ret_read = read (fd, Buffer, BUFFSIZE);

		char byte = 0, mask = 128;

		for (int i = 0; i < ret_read; i++)
		{
			KILL(pid, SIGUSR1);
			ALARM;

			byte = Buffer[i];

			while (mask)
			{
				if (mask & byte)
				{
					KILL(pid, SIGUSR1);
				} else
				{
					KILL(pid, SIGUSR2);
				}
				
				mask = (mask >> 1) & 127;

				ALARM;
			}

			mask = 128;
		}
	} while (ret_read);

	close (fd);

	KILL(pid, SIGUSR2);
	exit (EXIT_SUCCESS);
}

int parent (pid_t pid)
{
	int bitnum = 0;
	char symbol = 0;

	do {	
		ALARM;

		if (bitnum == 0)
		{
			KILL(pid, SIGUSR1);
			ALARM;
		}

		bitnum++;
		symbol = (symbol << 1) + Bit;
		if (bitnum == 8)
		{
			Buffer[index++] = symbol;
			bitnum = 0;
			if (index == BUFFSIZE)
			WRITE;
		}
		
		KILL(pid, SIGUSR1);
				
	} while (1);
}

void u1_hdlr ()
{
	Bit = 1;
}

void u2_hdlr ()
{
	Bit = 0;
}

void ch_hdlr ()
{
	if (!Bit)
	{
		WRITE;
		exit (EXIT_SUCCESS);
	} else
	{
		exit (-1);
	}
}
