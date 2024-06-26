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

//used to get sleeptime for child and parent
void msecSleepParent();
void msecSleepChild();


int main(int argc, char **argv)
{
	struct shm_struct {
		int buffer[N];
		int front, rear;
		int amount;
	};
	volatile struct shm_struct *shmp = NULL;
	char *addr = NULL;
	pid_t pid = -1;
	int var1 = 0, var2 = 0, shmid = -1;
	struct shmid_ds *shm_buf;

	//used to get random number for millisecond-sleep.
	srand(time(NULL));

	/* allocate a chunk of shared memory */
	shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
	shmp = (struct shm_struct *) shmat(shmid, addr, 0);
	shmp->front = 0;
	shmp->rear = 0;
	shmp->amount = 0;
	pid = fork();
	if (pid != 0) {
		/* here's the parent, acting as producer */
		while (var1 < 100) {
				/* write to shmem */
				if (shmp->amount < N) { //if buffer isn't full
				var1++;
				printf("Sending %d\n", var1); fflush(stdout);
				shmp->buffer[shmp->front] = var1;
				shmp->front = (shmp->front+1) % N; //making it circular, bounded by N(=10)
				shmp->amount++; // the amount change can cause race condition
				msecSleepParent();
			} // else busy wait if buffer is full
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
	} else {
		/* here's the child, acting as consumer */
		while (var2 < 100) {
			/* read from shmem */
			if (shmp->amount > 0) { //if buffer isn't empty
				var2 = shmp->buffer[shmp->rear];
				shmp->rear = (shmp->rear+1) % N; //making it circular, bounded by N(=10)
				shmp->amount--; // the amount change can cause race condition
				printf("Received %d\n", var2); fflush(stdout);
				msecSleepChild();
			} // else busy wait if buffer is empty
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
	usleep(random*1000);
}
