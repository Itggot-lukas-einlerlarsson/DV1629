#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <stdlib.h>

#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200

struct shm_struct {
    int buffer[10];
    unsigned size;
    int start, end;
};

int main() {
	volatile struct shm_struct *shmp = NULL;
	char *addr = NULL;
	pid_t pid = -1;
	int var1 = 0, var2 = 0, shmid = -1;
	struct shmid_ds *shm_buf;

	/* allocate a chunk of shared memory */
	shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
	shmp = (struct shm_struct *) shmat(shmid, addr, 0);
	shmp->size = 0;
	pid = fork();
    
    srand(time(NULL));
	
    if (pid != 0) {
		/* here's the parent, acting as producer */
		while (var1 < 100) {
			/* write to shmem */
			var1++;
			while (shmp->size >= 10); /* busy wait until the buffer is empty */
			printf("Sending %d\n", var1); fflush(stdout);
            shmp->buffer[shmp->end] = var1;
            shmp->end++;
            shmp->end %= 10;
            /* This can cause data race condition with the child */
			shmp->size++;
            sleep(((rand() % 4)+1)/10);
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
	} else {
		/* here's the child, acting as consumer */
		while (var2 < 100) {
			/* read from shmem */
			while (shmp->size == 0); /* busy wait until there is something */

            var2 = shmp->buffer[shmp->start];
            shmp->start++;
            shmp->start %= 10;
            printf("Received %d\n", var2); fflush(stdout);
            /* This can cause data race condition with the parent */
            shmp->size--;
            sleep(((rand() % 18)+2)/10);
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
	}

    return 0;
}
