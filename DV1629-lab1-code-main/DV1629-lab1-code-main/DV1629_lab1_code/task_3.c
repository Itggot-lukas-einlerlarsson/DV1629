#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> /* For O_* constants */
#include <stdlib.h>

#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200

const char *semName1 = "my_sema_7";
const char *semName2 = "my_sema_8";

struct shm_struct {
    int buffer[10];
    int start, end;
};

int main() {
	volatile struct shm_struct *shmp = NULL;
	char *addr = NULL;
	pid_t pid = -1;
	int var1 = 0, var2 = 0, shmid = -1, status;
	struct shmid_ds *shm_buf;

    /* initalize semaphores, first one to the size of the array */
	sem_t *sem_id1 = sem_open(semName1, O_CREAT, O_RDWR, 10);
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
			shmp->end++;
			shmp->end %= 10;

			// Wait between 0.1 - 0.5s
			sleep(((rand() %(5-1+1)+1)/10));
			
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
            shmp->start++;
            shmp->start %= 10;
            printf("\tReceived %d\n", var2); fflush(stdout);

            // Wait between 0.2 - 2.0s
            sleep(((rand() %(20-2+1)+2)/10));

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
