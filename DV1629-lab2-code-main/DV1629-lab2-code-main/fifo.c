#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int no_phys_pages = atoi(argv[1]), page_size = atoi(argv[2]);
    char *filename = argv[3];
    FILE *fp;
    char pagec[page_size];
    int page_faults = 0, memory_references = 0, frame[no_phys_pages], index = -1;

    printf("No. physical pages = %d, page size = %d\t\n", no_phys_pages, page_size);
    printf("Reading memory trace from %s...", filename);

    for(int i = 0; i < no_phys_pages; i++) {
        frame[i] = -1;
    }

    fp = fopen(filename, "r");
    while(fscanf(fp, "%s", pagec) != EOF) {
        memory_references++;
        int page = atoi(pagec);
        int flag = 0;

        for(int i = 0; i < no_phys_pages; i++) {
            if (page >= frame[i] && page < frame[i] + page_size) {
                flag = 1;
                break;
            }
        }
        if(flag != 1) {
            index = (index + 1) % no_phys_pages;
            frame[index] = (page/page_size)*page_size;
            page_faults++;
        }
    };
    printf("\tRead %d memory references\n", memory_references);
    printf("Result: %d page faults\n", page_faults);
   
    return 0;
}
