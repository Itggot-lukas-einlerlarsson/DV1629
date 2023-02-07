#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 1024

static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];

// Thread creation
void* thread_init_matrix(void* buf) {
    unsigned long childID = (unsigned long)buf;
    // Init as sequential but based on row of ID
    for (int j = 0; j < SIZE; j++) {
        a[childID][j] = 1.0;
        b[childID][j] = 1.0;
    }
    return NULL;
}
// Init matrixes a and b setting them to 1:s parallely
static void init_matrix_par(void) {
    pthread_t *children;
    unsigned int id = 0;
    unsigned long nThreads = SIZE;
    children = malloc(nThreads * sizeof(pthread_t) );

    for (int i = 0; i < SIZE; i++) {
        // Init threads and join after to await it
        pthread_create(&(children[i]), NULL, thread_init_matrix, (void*)(__intptr_t)i);
        pthread_join(children[i], NULL);
    }

}
// Init matrixes a and b setting them to 1:s
static void init_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++) {
	        a[i][j] = 1.0;
	        b[i][j] = 1.0;
        }
}
// Ordinary given sequencial matrix multiplication
static void matmul_seq()
{
    int i, j, k;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            c[i][j] = 0.0;
            for (k = 0; k < SIZE; k++)
                c[i][j] = c[i][j] + a[i][k] * b[k][j];
        }
    }
}
// Thread creation
void* thread_matmul_matrix(void* buf) {
    unsigned long childID = (unsigned long)buf;
    // Do the matrix multiplications on the row based on the given ID for the thread
    for (int j = 0; j < SIZE; j++) {
        c[childID][j] = 0.0;
        for (int k = 0; k < SIZE; k++)
            c[childID][j] = c[childID][j] + a[childID][k] * b[k][j];
    }
    return NULL;
}
// Create all threads for the parallel matrix multiplications
static void matmul_par()
{
    pthread_t *children;
    unsigned int id = 0;
    unsigned long nThreads = SIZE;
    children = malloc(nThreads * sizeof(pthread_t) );

    // Do a loop for each child to create
    for (int i = 0; i < SIZE; i++) {
        pthread_create(&(children[i]), NULL, thread_matmul_matrix, (void*)(__intptr_t)i);
    }

    // Await and resynchronize with all threads
    for (int i = 0; i < SIZE; i++) {
        pthread_join(children[i], NULL);
    }
}
// Print out finished matrix
static void print_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++)
            // Print out each cell in the 2D array (matrix)
	        printf(" %7.2f", c[i][j]);
	    printf("\n");
    }
}

int main() {
    // INIT MATRIX SEQ + PAR
    // init_matrix();
    init_matrix_par();

    // MATRIX MULTIPLICATION SEQ + PAR
    // matmul_seq();
    matmul_par();

    // print_matrix();
    return 0;
}
