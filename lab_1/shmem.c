#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <time.h>
#include <stdlib.h>

#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200

#define N 10

//useed to get a sleeptime of 0.1-2 milliseconds
void msecSleepParent();
void msecSleepChild(){


int main(int argc, char **argv)
{
	/*	cirucular buffer , inspiration from s 142 in coursebook.*/
	struct shm_struct {
		int buffer[N];
		int index;
		int full;
		//int* front = buffer, rear = front; //starts as empty
		unsigned empty;
	};
	volatile struct shm_struct *shmp = NULL;
	char *addr = NULL;
	pid_t pid = -1;
	int var1 = 0, var2 = 0, shmid = -1;
	struct shmid_ds *shm_buf;

	//used to get random nubmer for millisecond-sleep.
	srand(time(NULL));

	/* allocate a chunk of shared memory */
	shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
	shmp = (struct shm_struct *) shmat(shmid, addr, 0);
	shmp->empty = 0;
	shmp->index = 0;
	shmp->full = 0;
	pid = fork();
	if (pid != 0) {
		/* here's the parent, acting as producer */
		while (var1 < 100) {
				/* write to shmem */
				if (shmp->full < 10) { //if buffer isn't full, full = count in many ways.
				var1++;
				printf("Sending %d\n", var1); fflush(stdout);
				shmp->buffer[shmp->index] = var1;
				shmp->full++;
				msecSleepParent();
			} else { //if buffer is full
				shmp->empty = 1;
			}
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
	} else {
		/* here's the child, acting as consumer */
		while (var2 < 100) {
			/* read from shmem */
			if (shmp->full > 0) { //if buffer isn't empty
				var2 = shmp->buffer[shmp->index];
				shmp->index = (shmp->index+1) % N; //making it circular, bounded by N(=10)
				shmp->full--;
				printf("Received %d\n", var2); fflush(stdout);
				msecSleepChild();
			} else { //if buffer is empty
				shmp->empty = 0;
			}
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
	}
}

void msecSleepParent(){
	int random = rand() % 500;
	while (random < 100) {
		random = rand() % 500;
	}
	//sleep in microseconds(10^(-6)) -> i multiply by 1000 to get milliseconds(10^(-3))
	usleep(random*1000);
}

void msecSleepChild(){
	int random = rand() % 2000;
	while (random < 200) {
		random = rand() % 2000;
	}
	//sleep in microseconds(10^(-6)) -> i multiply by 1000 to get milliseconds(10^(-3))
	usleep(random*1000);
}
