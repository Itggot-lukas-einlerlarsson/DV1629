#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    pid_t pid, pid2;
    unsigned i;
    unsigned niterations = 100;
    pid = fork();
    printf("\n");
    if (pid != 0) { //if not in child process A, create child process C
        pid2 = fork();
    }
    if (pid == 0) { //if pid is child.
        for (i = 0; i < niterations; ++i)
            printf("A = %d, ", i);
    }
    if (pid2 == 0) { //if pid2 is child.
        for (i = 0; i < niterations; ++i)
             printf("C = %d, ", i);
    }
    if (pid != 0 && pid2 != 0) { // parent
        for (i = 0; i < niterations; ++i)
            printf("B = %d, ", i);
        printf("child pid A = %d  |  ", pid);
        printf("child pid2 B = %d", pid2);
    }
    printf("\n");
}
