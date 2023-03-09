/*
 * pm_heap.h / Add Thread Safety
 *
 * Omkar Ubale / CS5600 / Northeastern University
 * Spring 2023 / Feb 27, 2023
 *
 * For documentation, Doxygen commenting has been used for better readability in
 * IDEs like VS Code.
 */

#define HEAP_SIZE_IN_MEGA_BYTES 10
// 1 page - 4 KB : 4 * 1024
#define PAGE_SIZE 4 * 1024

struct PageTableEntry
{
    int pageNumber;
    int size;
    struct PageTableEntry *next;
};

void pm_init();

void *pm_malloc(int size);

void pm_free(void *ptr);

int pm_getPageNumber(void *ptr);