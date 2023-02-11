#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

// Shared Variables
#define N 5
pthread_mutex_t lock[N]; //forks

struct professorSettings{
    unsigned int professorID;
    char* name;
};

void philosopher(struct professorSettings* args);
void* professor(void* params);


void philosopher(struct professorSettings* args) {
    int random;
    while (1) {
        // printf("%s: thinking id:%u\n", args->name, args->professorID);
        // random = rand() % 3 + 1;
        // sleep(random);
        // if (args->professorID % 2 != 0) {
        //     printf("%s: try to get left chopstick\n", args->name);
        //     pthread_mutex_lock(&lock[args->professorID]);
        //     printf("%s: thinking -> got left chopstick\n", args->name);
        //     printf("%s: got left chopstick -> thinking\n", args->name);
        //     random = rand() % 6 + 2;
        //     sleep(random);
        //
        //     printf("%s: try to get right chopstick\n", args->name);
        //     pthread_mutex_lock(&lock[(args->professorID + 1) % N]); //circular set
        //
        //     printf("%s: thinking -> get right chopstick\n", args->name);
        //     printf("%s: get right chopstick -> eating\n", args->name);
        //     random = rand() % 5 + 5;
        //     sleep(random);
        // } else {
        //     printf("%s: try to get right chopstick\n", args->name);
        //     pthread_mutex_lock(&lock[(args->professorID + 1) % N]);
        //     printf("%s: thinking -> got right chopstick\n", args->name);
        //     printf("%s: got right chopstick -> thinking\n", args->name);
        //     random = rand() % 6 + 2;
        //     sleep(random);
        //
        //     printf("%s: try to get left chopstick\n", args->name);
        //     pthread_mutex_lock(&lock[args->professorID]); //circular set
        //
        //     printf("%s: thinking -> get left chopstick\n", args->name);
        //     printf("%s: get left chopstick -> eating\n", args->name);
        //     random = rand() % 5 + 5;
        //     sleep(random);
        // }

        printf("%s: thinking\n", args->name);
        random = rand() % 3 + 1;
        sleep(random);

        printf("%s: try to get left chopstick\n", args->name);
        pthread_mutex_lock(&lock[args->professorID]);
        printf("%s: thinking -> got left chopstick\n", args->name);
        printf("%s: got left chopstick -> thinking\n", args->name);
        random = rand() % 6 + 2;
        sleep(random);

        // clock_t start, end;
        // double timeElapsed;
        // start = clock();
        // printf("%s: try to get right chopstick\n", args->name);
        // while (timeElapsed < 5) {
        //     if (pthread_mutex_lock(&lock[(args->professorID + 1) % N]) == 0) {
        //         break;
        //     }
        //     end = clock();
        //     timeElapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
        //     printf("%d\t", timeElapsed);
        //     // pthread_mutex_lock(&lock[(args->professorID + 1) % N]); //circular set
        // } else {
        //     pthread_mutex_unlock(&locks[id]);
        //     printf("%s:chopsticks not used but freed\n", args->name);
        //     continue;
        // }
        printf("%s: try to get right chopstick\n", args->name);
        if (pthread_mutex_trylock(&lock[(args->professorID + 1) % N]) != 0) {
            printf("%s: thinking -> get right chopstick\n", args->name);
            printf("%s: get right chopstick -> eating\n", args->name);
            random = rand() % 5 + 5;
            sleep(random);
        } else { //both couldnt be taken -> give back
            pthread_mutex_unlock(&lock[args->professorID]);
            printf("%s:chopsticks not used and freed\n", args->name);
            continue;
        }
        printf("%s: done eating -> give back chopsticks\n", args->name);
        pthread_mutex_unlock(&lock[args->professorID]);
        pthread_mutex_unlock(&lock[(args->professorID + 1) % N]);
        printf("%s:chopsticks used and freed\n", args->name);
    }
}

void* professor(void* params) {
    struct professorSettings* args = (struct professorSettings*) params;
    philosopher(args);
    free(args);
    return NULL;
}

int main(int argc, char const *argv[]) {
    //thread info
    pthread_t *professors;
    struct professorSettings* args;
    unsigned long id = 0;
    unsigned long nThreads = N;
    char* names[N] = {"Tannenbaum", "Bos", "Lamport", "Stallings", "Silberschatz"};

    //for random:
    srand(time(NULL));

    //creating threads
    professors = malloc( nThreads * sizeof(pthread_t));
    for (id = 0; id < nThreads; id++){
        args = malloc(sizeof(struct professorSettings));
        args->professorID = id;
        args->name = names[id];
        pthread_create(&(professors[id]), NULL, professor, (void*)args);
    }

    //termination of threads
    for (id = 0; id < nThreads; id++)
      pthread_join(professors[id], NULL);
    free(professors);
    return 0;
}