#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <string.h>
#include <sys/msg.h>

#define PERMS 0644

struct my_msgbuf {
   long mtype;
   int mvalue;
};

int main() {
  struct my_msgbuf buf;
  int msqid;
  int toend;
  key_t key;

  if ((key = ftok("msgq.txt", 'B')) == -1) {
     perror("ftok");
     exit(1);
  }

  if ((msqid = msgget(key, PERMS)) == -1) { /* connect to the queue */
     perror("msgget");
     exit(1);
  }
  printf("message queue: ready to receive messages.\n");

  int counter = 0;
  for(;;) { /* normally receiving never ends but just to make conclusion */
            /* this program ends with string of end */
     if (msgrcv(msqid, &buf, sizeof(buf.mvalue), 0, 0) == -1) {
        printf("\nItems recieved: %d\n", counter);
        perror("msgrcv");
        exit(1);
     }
     toend = buf.mvalue;
     if (toend == -1)
       break;
     counter++;

     printf("recvd: \"%d\"\n", buf.mvalue);
  }


  printf("\nItems recieved: %d\n", counter);
  printf("message queue: done receiving messages.\n");

  if (msgctl(msqid, IPC_RMID, NULL) == -1) {
     perror("msgctl");
     exit(1);
  }
  system("rm msgq.txt");
   return 0;
}
