#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
/* test2.c */

#define SIZE (16*1024)
#define ITERATIONS 5

// programmet allokerar dynamiskt minne genom calloc (heap)
// sedan skickar den den informationen till en temporär fil

int main(int argc, char **argv)
{
    struct link {
        int x[SIZE][SIZE];
    };
    struct link *start;
    int iteration;
    FILE *f;
    start = (struct link *) calloc(1, sizeof(struct link));
    if (!start) {
        printf("Fatal error: Can't allocate memory of %d x %d = %lu\n", SIZE, SIZE, (unsigned long)SIZE*SIZE);
        exit(-1);
    }
    unsigned long size = 0;
    for (iteration = 0; iteration < ITERATIONS; iteration++) {
        printf("test2, iteration: %d\n", iteration);
        f = fopen("/tmp/file1.txt", "w");
        size = SIZE*SIZE;
        fwrite(start->x, sizeof(start->x[0][0]), SIZE*SIZE, f);
        fclose(f);
        printf("%lu\n", size);
    }
}
