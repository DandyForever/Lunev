#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define dbg printf ("meow\n"); fflush (stdout);

#define MEM_MEM 256

enum semaphores {
	CSM_CUR = 0,
	CSM_PRE = 1,
	PDC_CUR = 2,
	PDC_PRE = 3,
	MUTEX   = 4,
	FULL	= 5,
	CTL	= 6,
	SEM_NUM = 7
};

size_t MEM_SIZE = sizeof(ssize_t) + 256;

int Shm_id = 0;
int Sem_id = 0;

char* Buffer = 0;

struct sembuf Sems[SEM_NUM] = {};
short Sem_ind = 0;


int consumer ();
int producer (const char* file_name);

int main (int argc, char* argv[])
{
	Shm_id = shmget (25, MEM_MEM, IPC_CREAT | 0666);	
	if (Shm_id == -1)
	{
		perror ("shmget");
		exit (-1);
	}

	Buffer = (char*) shmat (Shm_id, NULL, 0);
	if (Buffer == (char*) (void*) -1)
	{
		perror ("shmat");
		exit (-1);
	}

	Sem_id = semget (25, SEM_NUM, IPC_CREAT | 0666);
	if (Sem_id == -1)
	{
		perror ("semget");
		exit (-1);
	}
	
	if (argc == 1)
	{
		consumer ();
		shmdt (Buffer);
		shmctl (Shm_id, IPC_RMID, NULL);
		semctl (Sem_id, 0, IPC_RMID);
	}

	else if (argc == 2)
	{
		producer (argv[1]);
	}

	else
	{
		printf ("argument invalid: too many arguments\n");
		return 1;
	}

	return 0;
}

#define DEBUG if (0)

#define BREAKER(...)					\
	do {						\
		printf ("Line: %d\n", __LINE__);	\
		getchar ();				\
	} while (0)

#define SET_VAL(SEM, VAL)				\
	do {						\
		sc = semctl (Sem_id, SEM, SETVAL, VAL);	\
		if (sc == -1)				\
		{					\
			perror ("semctl");		\
			exit (-1);			\
		}					\
	} while (0)

#define SEMTOBUF(SEM, OP, FLG)		 		\
	do {						\
		Sems[Sem_ind].sem_num = SEM;		\
		Sems[Sem_ind].sem_op  = OP;		\
		Sems[Sem_ind].sem_flg = FLG;		\
		Sem_ind++;				\
	} while(0) 

#define SEMOP(...)					\
	do {						\
		if (semop (Sem_id, Sems, Sem_ind) == -1)\
		{					\
			perror ("semop");		\
			return -1;			\
		}					\
		Sem_ind = 0;				\
	} while (0)

int consumer ()
{	
	SEMTOBUF(CSM_CUR,  0, IPC_NOWAIT); // check for another consumer
	SEMTOBUF(CSM_PRE,  0, IPC_NOWAIT); //
	SEMTOBUF(CSM_CUR, +1, SEM_UNDO);   // take the consumer position
	SEMOP();

	SEMTOBUF(PDC_CUR, -1, 0);         // wait for producer
	SEMTOBUF(PDC_CUR, +1, 0);	  //
	SEMTOBUF(PDC_PRE, +1, SEM_UNDO);  // pick up the producer
	SEMOP();

	SEMTOBUF(PDC_CUR, -1, IPC_NOWAIT);
	SEMTOBUF(PDC_CUR, +1, 0);
	SEMTOBUF(CTL    , -1, SEM_UNDO);
	SEMOP();

	ssize_t bytes = 0;
	do {
		SEMTOBUF(PDC_CUR, -1, IPC_NOWAIT); // check producer
		SEMTOBUF(PDC_CUR, +1, 0);	   //
		
		SEMTOBUF(MUTEX  ,  0, 0);
		SEMTOBUF(MUTEX  , +1, SEM_UNDO);
		SEMTOBUF(FULL	, -1, SEM_UNDO);
		SEMOP();
		
		memcpy (&bytes, Buffer, sizeof(ssize_t));
	
		if (write (STDOUT_FILENO, Buffer + sizeof(ssize_t), bytes) == -1)
		{
			perror ("write");
			return -1;
		}
		
		SEMTOBUF(MUTEX, -1, SEM_UNDO);
		SEMOP();
	} while (bytes);

	return 0;
}

int producer (const char* file_name)
{
	int fd = open (file_name, O_RDONLY);
	if (fd == -1)
	{
		perror ("open");
		return -1;
	}

	SEMTOBUF(PDC_CUR,  0, IPC_NOWAIT); // check for another producer
	SEMTOBUF(PDC_PRE,  0, IPC_NOWAIT); //
	SEMTOBUF(PDC_CUR, +1, SEM_UNDO);   // take the producer position
	SEMOP();

	SEMTOBUF(CSM_CUR, -1, 0);	  // wait for consumer
	SEMTOBUF(CSM_CUR, +1, 0);	  //
	SEMTOBUF(CSM_PRE, +1, SEM_UNDO);  // pick up the consumer
	SEMOP();
	
	SEMTOBUF(CSM_CUR, -1, IPC_NOWAIT);
	SEMTOBUF(CSM_CUR, +1, 0);
	SEMTOBUF(FULL   ,  0, IPC_NOWAIT);
	SEMTOBUF(CTL    , +1, SEM_UNDO);
	SEMOP();
	
	ssize_t bytes = 0;
	do {
		SEMTOBUF(CSM_CUR, -1, IPC_NOWAIT); //check consumer
		SEMTOBUF(CSM_CUR, +1, 0);	   //
		
		SEMTOBUF(FULL,     0, 0);	
		SEMTOBUF(MUTEX  ,  0, 0);
		SEMTOBUF(MUTEX  , +1, SEM_UNDO);
		SEMOP();

		bytes = read (fd, Buffer + sizeof(ssize_t), MEM_MEM - sizeof(ssize_t));
		if (bytes == -1)
		{
			perror ("read");
			return -1;
		}

		memcpy (Buffer, &bytes, sizeof(ssize_t));
		
		SEMTOBUF(MUTEX , -1, SEM_UNDO);
		SEMTOBUF(FULL  , +1, SEM_UNDO);
		SEMOP();
	} while (bytes);

	close (fd);

	return 0;
}

