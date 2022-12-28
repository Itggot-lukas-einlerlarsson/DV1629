#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//time
#include <time.h>

#define PERMS 0644
struct my_msgbuf { //mailbox
   long mtype;
   int mtext[200]; //char mtext[200];
};

int main(void) {
   struct my_msgbuf buf;
   int msqid;
   int len;
   key_t key;
   system("touch msgq.txt");  //skapa messagequeue file

   if ((key = ftok("msgq.txt", 'B')) == -1) { //message queue
      perror("ftok");
      exit(1);
   }

   if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1) { //obtain a System V message queue
      perror("msgget");
      exit(1);
   }
   printf("message queue: ready to send messages.\n");
   printf("Enter lines of text, ^D to quit:\n");
   buf.mtype = 1; /* we don't really care in this case */

   srand(time(NULL));
   int random;

   for (size_t i = 0; i < 50; i++) {
     random = rand();
     len = 4; //strlen(buf.mtext)
     buf.mtext[0] = random;
     if (msgsnd(msqid, &buf, len, 0) == -1) /* +1 for '\0' */
        perror("msgsnd");
   }

   /*
   while(fgets(buf.mtext, sizeof buf.mtext, stdin) != NULL) {
      len = strlen(buf.mtext);
      if (buf.mtext[len-1] == '\n') buf.mtext[len-1] = '\0';
      if (msgsnd(msqid, &buf, len+1, 0) == -1)
         perror("msgsnd");
   }
   */
   buf.mtex[0] = 0;//strcpy(buf.mtext, 0);
   len = 4;//strlen(buf.mtext);
   if (msgsnd(msqid, &buf, len, 0) == -1) /* +1 for '\0' */
      perror("msgsnd");

   if (msgctl(msqid, IPC_RMID, NULL) == -1) {
      perror("msgctl");
      exit(1);
   }
   printf("message queue: done sending messages.\n");
   return 0;
}
