#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> /* For O_* constants */

const char *semName1 = "my_sema1";
const char *semName2 = "my_sema2";

int main(int argc, char **argv)
{
	pid_t pid;
	sem_t *sem_id1 = sem_open(semName1, O_CREAT, O_RDWR, 1);
	sem_t *sem_id2 = sem_open(semName2, O_CREAT, O_RDWR, 0);
	int i, status;

	pid = fork();
	if (pid) { //parent
		for (i = 0; i < 100; i++) {
			sem_wait(sem_id1); //wait until region is unlocked -> added to semaphore queue
			putchar('A'); fflush(stdout); //ok -> dowork
			sem_post(sem_id2); //unlocks region -> waiting process 'B' can now enter.
		}
		sem_close(sem_id1);
		sem_close(sem_id2);
		wait(&status);
		sem_unlink(semName1);
		sem_unlink(semName2);
	} else { //child
		for (i = 0; i < 100; i++) {
			sem_wait(sem_id2); //waiting for process 'A' to be done
			putchar('B'); fflush(stdout); //ok -> dowork
			sem_post(sem_id1); // unlocks region -> waiting process 'A' can now enter.
		}
		sem_close(sem_id1);
		sem_close(sem_id2);
	}
}
