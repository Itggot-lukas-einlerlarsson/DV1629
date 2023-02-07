#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct threadSettings_2 {
	unsigned int id;
    char name[200];
};
// Shared variables
pthread_mutex_t locks[5];

void* child(void* params) {
    // Thread represented as a professor
    struct threadSettings_2 *args = (struct threadSettings_2*) params;
    unsigned long id = args->id;
    while(1) {
        srand(time(NULL));
        printf("\n%s: thinking...\n", args->name);
        // Random number between 1 and 5s
        int num = rand() % (5-1+1) + 1;
        // Get left and right chopstick to see priority, helps us prevent circular wait and thereby deadlock
        int left = id; int right = (id + 1) % 5;

        // Step 1 - Think
        sleep(num);
        printf("\n%s: thinking -> get first chopstick\n", args->name);

        // Step 2 - Get first chopstick based on priority
        if(left > right) {
            pthread_mutex_lock(&locks[left]);
        } else {
            pthread_mutex_lock(&locks[right]);
        }
        printf("\n%s: got first chopstick -> thinking\n", args->name);

        // Step 3 - Think 2 - 8s
        num = rand() % (8-2+1) + 2;
        sleep(num);
        printf("\n%s: thinking -> get second chopstick\n", args->name);

        // Step 4 - Pick up second chopstick based on priority
        if(left > right) {
            pthread_mutex_lock(&locks[right]);
        } else {
            pthread_mutex_lock(&locks[left]);
        }
        printf("\n%s: got second chopstick -> eating\n", args->name);

        // Step 5 - Eat between 5 and 10s
        printf("\n%s: eating...\n", args->name);
        num = rand() % (10-5+1) + 5;
        sleep(num);
        printf("\n%s: done eating -> put away chopsticks\n", args->name);

        // Put chopstick away
        pthread_mutex_unlock(&locks[id]);
        pthread_mutex_unlock(&locks[(id + 1) % 5]);
        printf("\n%s: done putting chopsticks away\n", args->name);
    }

    free(args);
    return NULL;
}

void main(int argc, char** argv) {
    srand(time(NULL));
    pthread_t *children;
    struct threadSettings_2* args;
    unsigned int id = 0;
    unsigned long nThreads = 5;

    children = malloc(nThreads * sizeof(pthread_t) );
    // Names of the professors (threads)
    char children_names[5][200] = {"Archimedes", "Tanenbaum", "Lamport", "Stallings", "Silberschatz"};
    for (id = 0; id < nThreads; id++) {
    	args = malloc(sizeof(struct threadSettings_2));
        // Give the professor a seat at the table (id)
		args->id = id;
        // Give the professor (thread) a name
		strcpy(args->name, children_names[id]);
        pthread_create(&(children[id]), NULL, child, (void*)args);
    }
    for (id = 0; id < nThreads; id++)
        pthread_join(children[id], NULL);
    
    printf("\n{%lu} thread(s) has completed.\n\n", nThreads);
    free(children);
    return 0;
}
