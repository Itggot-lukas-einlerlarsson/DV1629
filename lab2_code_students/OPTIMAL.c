#include <stdio.h>
#include <stdlib.h>
#define FILESIZE 100000

// Check which page frame is gonna get referenced furthest forward in the future
// The futher away the next reference to page is, the more unoptimal it is, least optimal page gets replaced when page fault occurs.
int get_least_optimal(int* all_memory_references, int* page_frame, int no_phys_pages, int no_memory_references, int page_size);

int main(int argc, char const* argv[]) {
    int no_phys_pages = atoi(argv[1]);
    int page_size = atoi(argv[2]);
    const char* filename = argv[3];
    printf("No physical pages = %i, page size = %i\n", no_phys_pages, page_size);
    printf("Reading memory trace from %s... \t", filename);

    FILE* file_ptr;
    file_ptr = fopen(filename, "r"); // read file
    int all_memory_references[FILESIZE];
    int count = 0;
    int memory_reference; // holds memory references found in mem-file
    // first read all memory_reference into program from file
    while (fscanf(file_ptr, "%d", &memory_reference) != EOF){
        all_memory_references[count] = memory_reference;
        count++;
    }

    int no_page_faults = 0;
    int no_memory_references = 0;
    int page_index = 0;
    int in_frame; // bool
    int div_helper = 0;
    int page_frame[no_phys_pages]; // physical page frame
    for (size_t i = 0; i < no_phys_pages; i++) {
        page_frame[i] = -42;
    }

    // then go through each memory reference and apply optimal page replacement algorithm
    for (size_t i = 0; i < FILESIZE; i++) {
        memory_reference = all_memory_references[i];
        no_memory_references++;
        in_frame = 0;

        for (size_t i = 0; i < no_phys_pages; i++) { // check if in physical mem already
            if (memory_reference >= page_frame[i] &&
                memory_reference < page_frame[i] + page_size) {
                in_frame = 1; // if in physical frame page
            }
        }

         // if not in physical page frame -> page fault -> find least used future physical page
        if (!in_frame) {
            page_index = get_least_optimal(all_memory_references, page_frame, no_phys_pages, no_memory_references, page_size);
            div_helper = memory_reference/page_size; //  integer division
            memory_reference = div_helper * page_size;
            page_frame[page_index] = memory_reference; // a new memory reference is added along with its page size.
            no_page_faults++;
        }
    }
    printf("Read %i memory references\n", no_memory_references);
    printf("Result: %i page faults\n", no_page_faults);
    fclose (file_ptr);
    return 0;
}

// get the page frame in physical memory that is least optimal
int get_least_optimal(int* all_memory_references, int* page_frame, int no_phys_pages, int no_memory_references, int page_size){
    // question: when does the page frame reference happen again next time?
    int optimality[no_phys_pages]; // holds how far the next reference is for each page frame, i.e. how unoptimal each page is
    int in_frame;
    for (size_t i = 0; i < no_phys_pages; i++) {
        optimality[i] = 0;
        for (size_t j = no_memory_references; j < FILESIZE; j++) {
            in_frame = 0;
            if (all_memory_references[j] >= page_frame[i] &&
                all_memory_references[j] < page_frame[i] + page_size) {
                in_frame = 1; // if in physical frame page
            }
            if (!in_frame) {
                optimality[i]++; // if memory reference doesnt happen soon, increment
            } else {
                break;
            }
        }
    }
    int least_optimal_index = 0;
    int max = 0;
    for (size_t i = 0; i < no_phys_pages; i++) {
        if (max < optimality[i]) {
            max =  optimality[i];
            least_optimal_index = i;
        }
    }
    return least_optimal_index;
}
