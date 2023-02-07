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
   int len;
   key_t key;
   srand(time(NULL));
   system("touch msgq.txt");

   if ((key = ftok("msgq.txt", 'B')) == -1) {
      perror("ftok");
      exit(1);
   }

   if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1) {

      perror("msgget");
      exit(1);
   }
   printf("message queue: ready to send messages.\n");
   printf("Enter lines of text, ^D to quit:\n");
   buf.mtype = 1; /* we don't really care in this case */

   int counter = 0;

   char _;
   scanf("%c",&_);
   for(int i = 0; i < 50; i++) {
        int res = rand() % 10;
        buf.mvalue = res;

        printf("%d\n", buf.mvalue);

        len = sizeof(res);

        if (msgsnd(msqid, &buf, sizeof(buf.mvalue), 0) == -1)
            perror("msgsnd");
      counter++;
   }

   buf.mvalue = -1;
   len = sizeof(buf.mvalue);
   if (msgsnd(msqid, &buf, len, 0) == -1) /* +1 for '\0' */
      perror("msgsnd");
   printf("\nItems sent: %d\nInput anything to continue: ", counter);

   // Can fix with semaphores. recv post semaphore when all is done and send can continue
  //  scanf("%c",&_);

   printf("message queue: done sending messages.\n");
   return 0;
}
