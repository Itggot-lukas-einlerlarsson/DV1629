#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

// Shared Variables
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*
 fixa alla 4 kraven:
 mutual exclusion mellan trådarna, två processer får inte ändra samtidigt.
 . kör semaphores i guess.
 --- alltså om de två chopsticksen är lediga -> locka dem och ät.
 -- om de inte är lediga, tänk istället.
 - även alla states måste visa att den går från en state till en annan och vilken state.
*/

/*
  struct shm_struct {
    int chopsticks[N]; //tidigare buffer från producer/consumer
    int index;
    int full;
    //int* front = buffer, rear = front; //starts as empty
    unsigned empty;
  };
  volatile struct shm_struct *shmp = NULL;
  char *addr = NULL;
  pid_t pid = -1;
  int var1 = 0, var2 = 0, shmid = -1;
  struct shmid_ds *shm_buf;
  //for sync of processess via semaphores:
  const char *semName1 = "sema1";
  const char *semName2 = "sema2";
  const char *semName2 = "sema3";
  const char *semName2 = "sema4";
  const char *semName2 = "sema5";
  sem_t *sem_id1 = sem_open(semName1, O_CREAT, O_RDWR, 1); // 1 -> locked region, next process that waits() can enter and lock.
  sem_t *sem_id2 = sem_open(semName2, O_CREAT, O_RDWR, 1);
  sem_t *sem_id3 = sem_open(semName1, O_CREAT, O_RDWR, 1); // -> locked region, next process that waits() can enter and lock.
  sem_t *sem_id4 = sem_open(semName2, O_CREAT, O_RDWR, 1);
  sem_t *sem_id5 = sem_open(semName1, O_CREAT, O_RDWR, 1); // -> locked region, next process that waits() can enter and lock.

 printf("%s tries to take %s chopstick\n", name, turn);
 if (chopstick[i] == 0 and chopstick[(i+1)%N] == 0) {
   pthread_mutex_lock(&lock)
   take_chopstick(i, name, left); //left
   take_chopstick((i+1)%N, name, right); //right
   eat(name)
 }

*/

#define N 5
// #define LEFT (i+N-1)%N
// #define RIGHT (i+N)%N
#define THINKING 0
#define HUNGRY 1
#define EATING 2

int chopsticks[N]; //everyone get 1 chopstick and the one to the right (i+1)%N
char* names[N] = {"Tannenbaum", "Bos", "Lamport", "Stallings", "Silberschatz"};
// int states[N];


int take_chopstick(int i, char* name, char* turn) {
  printf("%s tries to take %s chopstick\n", name, turn);
  if (chopsticks[i] == 0) { //if not being used -> lock
    chopsticks[i] = 1;
    printf("%s got %s chopstick\n", name, turn);
    return 1;
  }
  return 0;
}

void give_chopstick(int i, char* name, char* turn) {
  if (chopsticks[i] == 1) { //if being used -> free
    chopsticks[i] = 0;
    printf("%s gave %s chopstick\n", name, turn);
  }
}

void think(char* name) {
  printf("%s is thinking: hmmmm\n", name);
  int random = rand() % 8 + 2;
  sleep(random);
}

void small_think(char* name) {
  // printf("%s is thinking: hmmmm\n", name);
  int random = rand() % 3 + 1;
  sleep(random);
}

void eat(char* name) {
  printf("%s is eating: nom nom\n", name);
  int random = rand() % 8 + 2;
  sleep(random);
}

void philosopher(int i, char* name) {
  // int leftChopstick = chipsticks[(i+N-1)%N];
  // int rightChopstick = chopsticks[(i+N)%N];
  char* left = "left";
  char* right = "right";
  int checkChopsticks[2];
  // bool twoChopsticks = false;
  int count = 0
  while (count < 10) {
    // printf("hello from %s with pid: %d\n", name ,i);
    think(name);
    checkChopsticks[0] = take_chopstick(i, name, left); //left
    small_think(name);
    checkChopsticks[1] = take_chopstick((i+1)%N, name, right); //right
    if (checkChopsticks[0] == 1 && checkChopsticks[1] == 1) {
      //professor has both chopsticks <-> professor can eat
      eat(name);
    }
    give_chopstick(i, name, left); //left
    give_chopstick((i+1)%N, name, right); //right
    count++;
  }
}

void* professor(void* buf) {
    unsigned long professorID = (unsigned long)buf;
    char* name = names[professorID];
    philosopher(professorID, name);
    return NULL;
}

int main(int argc, char const *argv[]) {
  //thread info
  pthread_t *professors;
  unsigned long id = 0;
  unsigned long nThreads = 5;

  //for random:
  srand(time(NULL));

  //creating threads
  professors = malloc( nThreads * sizeof(pthread_t) );
  for (id = 1; id < nThreads; id++)
      pthread_create(&(professors[id-1]), NULL, professor, (void*)id);
  philosopher(0, names[0]); // main thread work (id=0)

  //termination of threads
  for (id = 1; id < nThreads; id++)
      pthread_join(professors[id-1], NULL);
  free(professors);
  return 0;
}
