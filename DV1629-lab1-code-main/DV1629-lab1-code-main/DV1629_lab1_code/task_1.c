#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid, pid_2;
    unsigned i, counter = 0;
    unsigned niterations = 100;
    
    pid = fork();
    counter++;
    if(pid != 0) {
        counter++;
        pid_2 = fork();
    }

    if (pid == 0) {
        for (i = 0; i < niterations; ++i)
            printf("A = %d, ", i);
    } else if (pid_2 == 0) {
        for (i = 0; i < niterations; ++i)
            printf("C = %d, ", i);
    }
    else {
        for (i = 0; i < niterations; ++i)
            printf("B = %d, ", i);
        for (i = 0; i < counter; i++) {
            printf("\nProcceses = %d", pid+i);
        }
    }
    
    printf("\n");

    return 0;
}
