#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>

#define N 10
#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200

const char *semName1 = "my_sema_7";
const char *semName2 = "my_sema_8";

struct shm_struct {
    int buffer[N];
    int start;
	int end;
};

int main() {
	volatile struct shm_struct *shmp = NULL;
	char *addr = NULL;
	pid_t pid = -1;
	int var1 = 0, var2 = 0, shmid = -1, status;
	struct shmid_ds *shm_buf;

    /* initalize semaphores, first one to the size of the array */
	sem_t *sem_id1 = sem_open(semName1, O_CREAT, O_RDWR, N);
	sem_t *sem_id2 = sem_open(semName2, O_CREAT, O_RDWR, 0);

	/* allocate a chunk of shared memory */
	shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
	shmp = (struct shm_struct *) shmat(shmid, addr, 0);
	pid = fork();

    srand(time(NULL));

    if (pid != 0) {
		/* here's the parent, acting as producer */
		while (var1 < 100) {
			/* write to shmem */
			// Decrease value (write), if 0 (we've written up to the consumer, we wait)
			sem_wait(sem_id1);
			var1++;

			printf("Sending %d\n", var1); fflush(stdout);
			shmp->buffer[shmp->end] = var1;
			shmp->end = (shmp->end+1) % N;

			msecSleepParent()

			// Post sem_id2 to increase semaphore value (allow it to start reading)
			sem_post(sem_id2);
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
    	sem_close(sem_id1);
		sem_close(sem_id2);
		wait(&status);
		sem_unlink(semName1);
		sem_unlink(semName2);
	} else {
		/* here's the child, acting as consumer */
		while (var2 < 100) {
			sem_wait(sem_id2);

            var2 = shmp->buffer[shmp->start];
            shmp->start = (shmp->start+1) % N;
            printf("\tReceived %d\n", var2); fflush(stdout);

            msecSleepChild()

            // Post sem_id1 to increase semaphore value (allow it to write more, can be up to 10, so if we read more, it is allowed to write more)
            sem_post(sem_id1);
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
    	sem_close(sem_id1);
		sem_close(sem_id2);
	}

    return 0;
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
