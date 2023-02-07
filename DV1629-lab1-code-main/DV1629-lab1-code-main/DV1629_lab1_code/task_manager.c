#include <stdio.h>
#include <errno.h>   // for errno
#include <stdlib.h>  // for strtol
#include <string.h>

int main(int argc, char** argv) {
    int running = 1;
    char cmd[512];
    int task, nrThreads;

    printf("\n-------------- TASK MANAGER --------------\n");
    printf("\nEnter task you want to run (-1 for exit): ");
    scanf ("%d",&task);
    printf("Enter nr of threads to run: ");
    scanf ("%d",&nrThreads);

    while(running) {
        if(task == -1) {
            break;
        } else {
            char str[512];
            snprintf(str, sizeof(str), "gcc DV1629_Lab1_Code/task_%d.c -lstdc++ -lpthread -o main.o && ./main.o %d", task, nrThreads);
            system(str);
        }
        printf("\nEnter task you want to run: ");
        scanf ("%d",&task);
        printf("Enter nr of threads to run: ");
        scanf ("%d",&nrThreads);
    }

    return 0;
}