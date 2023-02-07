#include<stdio.h>
#include <stdlib.h>

int predict_forward(int page[], int frames[], int adresses_c, int no_phys_pages, int index, int page_size) {
    int rep = -1, farthest = index, j;
    for(int i = 0; i < no_phys_pages; i++) {
        for(j = index; j < adresses_c; j++) {
            if(frames[i] == (page[j]/page_size)*page_size) {
                if(j > farthest) {
                    farthest = j;
                    rep = i;
                }
                break;
            }
        }
        if(j == adresses_c) {
            return i;
        }
    }
    if(rep != -1) {
        return rep;
    } else {
        return 0;
    }
}

int search(int page, int frames[], int frame_size, int page_size) {
    for(int i = 0; i < frame_size; i++) {
        if(page >= frames[i] && page < frames[i] + page_size) {
            return 1;
        }
    }
    return 0;
}


int main(int argc, char **argv)
{

    char *filename = argv[3];
    FILE *fp;
    int adresses[1000000], adresses_c = 0;

    fp = fopen(filename, "r");
    while(fscanf(fp, "%d", &adresses[adresses_c]) != EOF) {
        adresses_c++;
    }
    fclose(fp);

    int no_phys_pages = atoi(argv[1]), page_size = atoi(argv[2]);
    int frame[no_phys_pages];

    printf("No. physical pages = %d, page size = %d\t\n", no_phys_pages, page_size);
    printf("Reading memory trace from %s...\t", filename);

    for(int i = 0;i< no_phys_pages;i++) {
        frame[i]=-1;
    }

    int frame_counter = 0, page_faults = 0;
    for(int i = 0; i < adresses_c; i++) {
        if(!search(adresses[i], frame, no_phys_pages, page_size)) {
            page_faults++;
            if(frame_counter < no_phys_pages) {
                frame[frame_counter++] = (adresses[i]/page_size)*page_size;
            } else {
                int pred_index = predict_forward(adresses, frame, adresses_c, no_phys_pages, i+1, page_size);
                frame[pred_index] = (adresses[i]/page_size)*page_size;
            }
        }
    }
    printf("Read %d memory references\n", adresses_c);
    printf("Result: %d page faults\n", page_faults);
    return 0;
}

// Main function
// int main(int argc, char **argv)
// {

//     char *filename = argv[3];
//     FILE *fp;
//     int adresses[1000000], adresses_c = 0;

//     fp = fopen(filename, "r");
//     while(fscanf(fp, "%d", &adresses[adresses_c]) != EOF) {
//         adresses_c++;
//     }
//     fclose(fp);

//     printf("PAGE\t--------- PAGE SIZE ---------\n");
//     printf("\t128\t256\t512\t1024\n\n");
//     for(int i = 1; i <= 128; i*=2) {
//         printf("%d\t",i);
//         for(int j = 128; j <= 1024; j*=2) {
//             int no_phys_pages = i, page_size = j;
//             int frame[no_phys_pages];

//             for(int i = 0;i< no_phys_pages;i++) {
//                 frame[i]=-1;
//             }

//             int frame_counter = 0, page_faults = 0;
//             for(int i = 0; i < adresses_c; i++) {
//                 if(!search(adresses[i], frame, no_phys_pages, page_size)) {
//                     page_faults++;
//                     if(frame_counter < no_phys_pages) {
//                         frame[frame_counter++] = (adresses[i]/page_size)*page_size;
//                     } else {
//                         int pred_index = predict_forward(adresses, frame, adresses_c, no_phys_pages, i+1, page_size);
//                         frame[pred_index] = (adresses[i]/page_size)*page_size;
//                     }
//                 }
//             }
//             printf("%d\t",page_faults);
//         }
//         printf("\n");
//     }

//     return 0;
// }
