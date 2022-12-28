#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    pid_t pid, pid2;
    unsigned i;
    unsigned niterations = 10;
    pid = fork();
    printf("\n");
    if (pid == 0)
     { //if pid is child.
        for (i = 0; i < niterations; ++i)
            printf("A = %d, ", i);
        // printf("Child A pid: %d", getpid());
     }
     else
     {
       pid2 = fork();
       if (pid2 == 0)
       { //if pid is child.
         for (i = 0; i < niterations; ++i)
             printf("C = %d, ", i);
          // printf("Child C pid: %d", getpid());
       }
       else
       { //if fail or parent
          for (i = 0; i < niterations; ++i)
              printf("B = %d, ", i);
          printf("child pids = %d \n", pid);
          printf("child pids = %d \n", pid2);
       }
    }
    printf("\n");
}
