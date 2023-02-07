#include <stdio.h>
#include <stdlib.h>

int main() {
    char cmd[200] = {0};
    for(int i = 4; i < 16; i*=2) {
        for(int _ = 0; _ < 4; _++) {
            sprintf(cmd, "gcc lab1_code_students_v1.3/bankaccount.c -lstdc++ -lpthread -o main.o && ./main.o %i",i*4);
            system(cmd);
        }
    }

    return 0;
}
