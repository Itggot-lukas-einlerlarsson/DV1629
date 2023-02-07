#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <sys/time.h>
#include <time.h>

#define PERMS 0644
struct my_msgbuf {
   long mtype;
   int mint;
};

int main(void) {
   struct my_msgbuf buf;
   int msqid;
   int len;
   key_t key;
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

   srand(time(NULL));

   char empty;
   scanf("%c",&empty); //then go

   for(int i = 0; i < 50; i++) {
       // Random value to send
       short unsigned res = rand();
       buf.mint = res;

       printf("\tsent: %d\n", buf.mint);

       // Send value to the message queue
       if (msgsnd(msqid, &buf, sizeof(buf.mint), 0) == -1)
           perror("msgsnd");
   }

   buf.mint = -1;
   len = sizeof(buf.mint);
   if (msgsnd(msqid, &buf, len, 0) == -1) /* +1 for '\0' */
      perror("msgsnd");


   // while(fgets(buf.mtext, sizeof buf.mtext, stdin) != NULL) {
   //    len = strlen(buf.mtext);
   //    /* remove newline at end, if it exists */
   //    if (buf.mtext[len-1] == '\n') buf.mtext[len-1] = '\0';
   //    if (msgsnd(msqid, &buf, len+1, 0) == -1) /* +1 for '\0' */
   //       perror("msgsnd");
   // }
   // strcpy(buf.mtext, "end");
   // len = strlen(buf.mtext);
   // if (msgsnd(msqid, &buf, len+1, 0) == -1) /* +1 for '\0' */
   //    perror("msgsnd");
   //
   // if (msgctl(msqid, IPC_RMID, NULL) == -1) {
   //    perror("msgctl");
   //    exit(1);
   // }
   printf("message queue: done sending messages.\n");
   return 0;
}
