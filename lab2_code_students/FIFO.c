#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    int no_phys_pages = atoi(argv[1]);
    int page_size = atoi(argv[2]);
    const char* filename = argv[3];
    printf("No physical pages = %i, page size = %i\n", no_phys_pages, page_size);
    printf("Reading memory trace from %s... \t", filename);

    FILE* file_ptr;
    file_ptr = fopen(filename, "r"); //read file
    int memory_reference; //holds memory references found in mem-file
    int no_page_faults = 0;
    int no_memory_references = 0;
    int page_queue[no_phys_pages]; // physical page frame
    int page_index = 0;
    int in_frame; // bool
    int div_helper = 0;

    for (size_t i = 0; i < no_phys_pages; i++) {
        page_queue[i] = -42;
    }

    while (fscanf(file_ptr, "%d", &memory_reference) != EOF){ // && no_memory_references < 100) { //read until whitespace found -> each line
        no_memory_references++;
        in_frame = 0;

        for (size_t i = 0; i < no_phys_pages; i++) { // check if in physical mem already
            if (memory_reference >= page_queue[i] && memory_reference < page_queue[i] + page_size) {
                in_frame = 1; // if in physical frame page
            }
        }

        if (!in_frame) { // if not in physical page frame -> page fault -> get rid of page put first in queue
            div_helper = memory_reference/page_size; // round memory to the page size it exists in AKA get rid of rest via integer division
            memory_reference = div_helper * page_size;
            page_queue[page_index] = memory_reference; // a new memory reference is added along with its page size.
            page_index = (page_index+1) % no_phys_pages; // go to next in queue
            no_page_faults++;
        }

    }
    printf("Read %i memory references\n", no_memory_references);
    printf("Result: %i page faults\n", no_page_faults);
    fclose (file_ptr);
    return 0;
}
