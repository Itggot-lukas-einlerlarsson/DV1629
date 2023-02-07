#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 1024
#define NR_THREADS 8

static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];

void* thread_init_matrix(void* buf) {
    unsigned long childID = (unsigned long)buf;

    for (int j = 0; j < SIZE; j++) {
        a[childID][j] = 1.0;
        b[childID][j] = 1.0;
    }
    return NULL;
}

static void init_matrix_par(void) {
    pthread_t *children;
    unsigned int id = 0;
    unsigned long nThreads = SIZE;
    children = malloc(nThreads * sizeof(pthread_t) );

    for (int i = 0; i < SIZE; i++) {
        pthread_create(&(children[i]), NULL, thread_init_matrix, (void*)(__intptr_t)i);
        pthread_join(children[i], NULL);
    }

}

static void init_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++) {
	        a[i][j] = 1.0;
	        b[i][j] = 1.0;
        }
}

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

void* thread_matmul_matrix_v2(void* buf) {
    unsigned long childID = (unsigned long)buf;
    for(int i = childID*(SIZE/NR_THREADS); i < (childID + 1)*(SIZE/NR_THREADS); i++)  {
        for (int j = 0; j < SIZE; j++) {
            c[i][j] = 0.0;
            for (int k = 0; k < SIZE; k++)
                c[i][j] = c[i][j] + a[i][k] * b[k][j];
        }
    }
    return NULL;
}

static void matmul_par_v2() {
    pthread_t *children;
    unsigned int id = 0;
    unsigned long nThreads = SIZE;
    children = malloc(nThreads * sizeof(pthread_t));

    for (int i = 0; i < NR_THREADS; i++) {
        pthread_create(&(children[i]), NULL, thread_matmul_matrix_v2, (void*)(__intptr_t)i);
    }

    for (int i = 0; i < NR_THREADS; i++) {
        pthread_join(children[i], NULL);
    }
}

void* thread_matmul_matrix(void* buf) {
    unsigned long childID = (unsigned long)buf;

    for (int j = 0; j < SIZE; j++) {
        c[childID][j] = 0.0;
        for (int k = 0; k < SIZE; k++)
            c[childID][j] = c[childID][j] + a[childID][k] * b[k][j];
    }
    return NULL;
}

static void matmul_par()
{
    pthread_t *children;
    unsigned int id = 0;
    unsigned long nThreads = SIZE;
    children = malloc(nThreads * sizeof(pthread_t) );

    for (int i = 0; i < SIZE; i++) {
        pthread_create(&(children[i]), NULL, thread_matmul_matrix, (void*)(__intptr_t)i);
    }

    for (int i = 0; i < SIZE; i++) {
        pthread_join(children[i], NULL);
    }
}

static void print_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++)
	        printf(" %7.2f", c[i][j]);
	    printf("\n");
    }
}

int main() {
    // init_matrix();
    init_matrix_par();

    // matmul_seq();
    // matmul_par();
    matmul_par_v2();

    // print_matrix();
    return 0;
}