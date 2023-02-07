#include <stdio.h>
#include <stdlib.h>

int main() {
    system("gcc lab1_code_students_v1.3/matmulseq.c -lstdc++ -lpthread -o main.o && time ./main.o");
    return 0;
}
