#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct threadSettings {
	unsigned int id;
    char name[200];
};
// Shared variables
pthread_mutex_t locks[5];

void* child(void* params) {
    struct threadSettings *args = (struct threadSettings*) params;
    unsigned long id = args->id;
    while(1) {
        printf("\n%s: thinking...\n", args->name);
        int num = rand() % (5-1+1) + 1;

        sleep(num);
        printf("\n%s: thinking -> get left chopstick\n", args->name);

        pthread_mutex_lock(&locks[id]);
        printf("\n%s: got left chopstick -> thinking\n", args->name);

        num = rand() % (8-2+1) + 2;
        sleep(num);
        printf("\n%s: thinking -> get right chopstick\n", args->name);

        pthread_mutex_lock(&locks[(id + 1) % 5]);
        printf("\n%s: got right chopstick -> eating\n", args->name);

        printf("\n%s: eating...\n", args->name);
        num = rand() % (10-5+1) + 5;
        sleep(num);
        printf("\n%s: done eating -> put away chopsticks\n", args->name);

        pthread_mutex_unlock(&locks[id]);
        pthread_mutex_unlock(&locks[(id + 1) % 5]);
        printf("\n%s: done putting chopsticks away\n", args->name);
    }

    free(args);
    return NULL;
}

int main(int argc, char** argv) {
    srand(time(NULL));
    pthread_t *children;
    struct threadSettings* args;
    unsigned int id = 0;
    unsigned long nThreads = 5;

    children = malloc(nThreads * sizeof(pthread_t) );
    char children_names[5][200] = {"Archimedes", "Tanenbaum", "Lamport", "Stallings", "Silberschatz"};
    for (id = 0; id < nThreads; id++) {
    	args = malloc(sizeof(struct threadSettings));
		args->id = id;
        
		strcpy(args->name, children_names[id]);
        pthread_create(&(children[id]), NULL, child, (void*)args);
    }
    for (id = 0; id < nThreads; id++)
        pthread_join(children[id], NULL);
    
    printf("\n{%lu} thread(s) has completed.\n\n", nThreads);
    free(children);
    return 0;
}
