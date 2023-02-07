#include<stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int no_phys_pages = atoi(argv[1]), page_faults=0, page_size = atoi(argv[2]);
    int frame[no_phys_pages],fcount[no_phys_pages];
    char *filename = argv[3];
    FILE *fp;

    printf("No. physical pages = %d, page size = %d\t\n", no_phys_pages, page_size);
    printf("Reading memory trace from %s...", filename);

    for(int i = 0;i< no_phys_pages;i++) {
        frame[i]=-1;
        fcount[i]=0; // couter of page's index occurance
    }

    fp = fopen(filename, "r");

    int i = 0, page;
    while(fscanf(fp, "%d", &page) != EOF) {
        int j=0,flag=0;

        for(int j = 0; j < no_phys_pages; j++) {
            if(page >= frame[j] && page < frame[j] + page_size){
                flag = 1;
                // i+1 to keep the counter the largest
                fcount[j] = i+1;
            }
        }

        if(flag != 1) {
            int min=0;
            for(int j = 0; j < no_phys_pages; j++) {
                if(fcount[min]>fcount[j]) {
                    min=j;
                }
            }
            frame[min]=(page/page_size)*page_size;
            // i+1 to keep the counter the largest
            fcount[min] = i+1;
            page_faults++;
        }
        i++;
    }
    fclose(fp);

    printf("\tRead %d memory references\n", i);
    printf("Result: %d page faults\n", page_faults);

    return 0;
} 
