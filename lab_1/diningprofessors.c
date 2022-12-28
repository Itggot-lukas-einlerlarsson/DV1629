#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

// Shared Variables
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
double bankAccountBalance = 0;

//when a professor has both chopsticks he can eat -> otherwise not.
//if the left chopstick is taken (1) -> the professor needs to wait and is next in queue.
//if the right chopstick is taken (1) -> professor needs to wait and is next in queue i guess
//
int chopstick0 = 0;
int chopstick1 = 0;
int chopstick2 = 0;
int chopstick3 = 0;
int chopstick4 = 0;


void deposit(double amount) {
    bankAccountBalance += amount;
}

void withdraw(double amount) {
    bankAccountBalance -= amount;
}

void think() {
  int random = rand() % 8 + 2;
  sleep(random);
}

// utility function to identify even-odd numbers
unsigned odd(unsigned long num) {
    return num % 2;
}

// simulate id performing 1000 transactions
void dine(unsigned long id) {
    for (int i = 0; i < 1000; i++) {
      if (id == 0) {
        printf("Tannenbaum\n", );
      }
      if (id == 1) {
        printf("Bos\n", );
      }
      if (id == 2) {
        printf("Lamport\n", );
      }
      if (id == 3) {
        printf("Stallings\n", );
      }
      if (id == 4) {
        printf("Silberschatz\n", );
      }
        if (odd(id)){
            pthread_mutex_lock(&lock);
            deposit(100.00); // odd threads deposit
            pthread_mutex_unlock(&lock);
        } else {
            pthread_mutex_lock(&lock);
            withdraw(100.00); // even threads withdraw
            pthread_mutex_unlock(&lock);
        }
    }
}

void* child(void* buf) {
    unsigned long childID = (unsigned long)buf;
    char* name = "childpild";
    dine(childID);
    return NULL;
}

int main(int argc, char** argv) {
    pthread_t *children;
    unsigned long id = 0;
    unsigned long nThreads = 4;

    //for random:
    srand(time(NULL));

    pthread_mutex_init(&lock, 0);
    children = malloc( nThreads * sizeof(pthread_t) );
    for (id = 1; id < nThreads; id++)
        pthread_create(&(children[id-1]), NULL, child, (void*)id);
    dine(0); // main thread work (id=0)
    for (id = 1; id < nThreads; id++)
        pthread_join(children[id-1], NULL);
    printf("\nThe final account balance with %lu threads is $%.2f.\n\n", nThreads, bankAccountBalance);
    free(children);
    pthread_mutex_destroy(&lock);
    return 0;
}
