#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct threadArgs {
	unsigned int id;
	unsigned int numThreads;
	unsigned int squaredId;
};

void* child(void* params) {
	struct threadArgs *args = (struct threadArgs*) params;
	unsigned int childID = args->id;
	unsigned int numThreads = args->numThreads;
	args->squaredId = childID*childID;
	printf("Greetings from child #%u of %u\n", childID, numThreads);
	pthread_exit(&args->squaredId);
}

int main(int argc, char** argv) {
	pthread_t* children; // dynamic array of child threads
	struct threadArgs* args; // argument buffer
	unsigned int numThreads = 0;

	//to print later
	// unsigned int childIdSquares[numThreads];

	// get desired # of threads
	if (argc > 1){
		numThreads = atoi(argv[1]);
	}
	int* childIdSquares[numThreads];
	children = malloc(numThreads * sizeof(pthread_t)); // allocate array of handles
	args = malloc(numThreads * sizeof(struct threadArgs)); // args vector
	for (unsigned int id = 0; id < numThreads; id++) {
		// create threads
		args[id].id = id;
		args[id].squaredId = 0;
		args[id].numThreads = numThreads;
		pthread_create(&(children[id]), // our handle for the child
			NULL, // attributes of the child
			child, // the function it should run
			(void*)&args[id]); // args to that function
		// childIdSquares[id] = args[id].squaredId;
	}
	printf("I am the parent (main) thread.\n");
	for (unsigned int id = 0; id < numThreads; id++) {
		// pthread_join(children[id], childIdSquares[id]); //(void**)&(ptr[0])
		pthread_join(children[id], (void**)&childIdSquares[id]); //(void**)&(ptr[0])
	}
	free(args); // deallocate args vector
	free(children); // deallocate array
	for (unsigned int id = 0; id < numThreads; id++) {
		printf("Child square id: %d\n", *childIdSquares[id]);
	}
	return 0;
}
