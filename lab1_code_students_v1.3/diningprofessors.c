#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

// Shared Variables
#define N 5
pthread_mutex_t lock[N];

struct professorSettings{
    unsigned int professorID;
    char* name;
};

void philosopher(struct professorSettings* args);
void* professor(void* params);


void philosopher(struct professorSettings* args) {
    int random;
    while (1) {
        // think(name);
        // printf("hello from %s with pid: %d\n", args->name ,args->id);
        printf("\n%s: thinking\n", args->name);
        random = rand() % 3 + 1;
        sleep(random);

        printf("\n%s: try to get left chopstick\n", args->name);
        pthread_mutex_lock(&lock[args->professorID]);
        printf("\n%s: thinking -> got left chopstick\n", args->name);
        printf("\n%s: got left chopstick -> thinking\n", args->name);
        random = rand() % 6 + 2;
        sleep(random);

        printf("\n%s: try to get left chopstick\n", args->name);
        pthread_mutex_lock(&lock[(args->professorID + 1) % N]);

        printf("\n%s: thinking -> get right chopstick\n", args->name);
        printf("\n%s: get right chopstick -> eating\n", args->name);
        random = rand() % 5 + 5;
        sleep(random);
    }
}

void* professor(void* params) {
    struct professorSettings* args = (struct professorSettings*) params;
    // unsigned int id = args->id;
    philosopher(args);
    // free(args);
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
        // strcpy(args->name, names[id]);
        args->name = names[id];
        pthread_create(&(professors[id]), NULL, professor, (void*)args);
    }
    // philosopher(0, names[0]); // main thread work (id=0)

    //termination of threads
    for (id = 0; id < nThreads; id++)
      pthread_join(professors[id], NULL);
    free(professors);
    return 0;
}
