/***************************************************************************
 *
 * parallell version of Matrix-Matrix multiplication
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define SIZE 1024

static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];
// pthread_mutex_t lock[SIZE]; //forks



struct threadArgs {
	unsigned int row;
};

void* child(void* params) {
	struct threadArgs *args = (struct threadArgs*) params;
    int j, k;
    // pthread_mutex_lock(&lock[c[i]]);
    for (j = 0; j < SIZE; j++) {
        c[args->row][j] = 0.0;
        for (k = 0; k < SIZE; k++)
            c[args->row][j] = c[args->row][j] + a[args->row][k] * b[k][j];
    }
    // pthread_mutex_unlock(&lock[c[i]]);
	// pthread_exit(&args->id);
}

static void
init_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++) {
	        /* Simple initialization, which enables us to easy check
	         * the correct answer. Each element in c will have the same
	         * value as SIZE after the matmul operation.
	         */
	        a[i][j] = 1.0;
	        b[i][j] = 1.0;
        }
}

static void
matmul_seq()
{
    int i, j, k;
    pthread_t* children; // dynamic array of child threads
	struct threadArgs* args; // argument buffer
	unsigned int numThreads = SIZE;
    children = malloc(numThreads * sizeof(pthread_t)); // allocate array of handles
	args = malloc(numThreads * sizeof(struct threadArgs)); // args vector

    for (i = 0; i < SIZE; i++) {
        // Här kommen en ny tråd
		args[i].row = i;
		pthread_create(&(children[i]), // our handle for the child
			NULL, // attributes of the child
			child, // the function it should run
			(void*)&args[i]); // args to that function
    }
    for (int i = 0; i < SIZE; i++) {
        pthread_join(children[i], NULL);
    }
    free(args); // deallocate args vector
    free(children); // deallocate array
}

static void
print_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++)
	        printf(" %7.2f", c[i][j]);
	    printf("\n");
    }
}

int
main(int argc, char **argv)
{
    init_matrix();
    matmul_seq();
    // print_matrix();
}
