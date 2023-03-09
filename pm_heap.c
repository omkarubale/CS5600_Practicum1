/*
 * pm_heap.c / Practicum 1
 *
 * Omkar Ubale, Ujwal Gupta / CS5600 / Northeastern University
 * Spring 2023 / March 9, 2023
 *
 * For documentation, Doxygen commenting has been used for better readability in
 * IDEs like VS Code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "pm_heap.h"

char pm_heap[HEAP_SIZE_IN_MEGA_BYTES * PAGE_SIZE * 256];
void *pageMapping[HEAP_SIZE_IN_MEGA_BYTES * 256];

pthread_mutex_t heap_access_mutex;

/// @brief Initializes the heap.
void pm_init()
{
    printf("init started\n");

    // initialize mutex for output file
    if (pthread_mutex_init(&heap_access_mutex, NULL) != 0)
    {
        printf("\nERROR: output file mutex init failed\n");
        exit(1);
    }

    memset(pm_heap, '0', sizeof(pm_heap));
    memset(pageMapping, NULL, sizeof(pageMapping));

    printf("init complete\n\n");
}

/// @brief Allocates memory and gives the page number in the virtual page table.
/// @param size the size of memory requested (cannot be more than 4KB)
/// @return the page number of the allocated memory in the virtual page table.
int pm_malloc(int size)
{
    printf("malloc started for size %d\n", size);

    // Mutex acquired for allocating memory in the heap
    pthread_mutex_lock(&heap_access_mutex);

    // TODO: check if the pageMapping array has available space (can be done during allocation as well)
    // TODO: if yes:
    //      - allocate first page available
    //      - create Page struct
    //      - set values in Page struct
    //      - set value in pageMapping array
    // TODO: if no:
    //      - Return not possible (-1)

    pthread_mutex_unlock(&heap_access_mutex);

    return 0;
}

/// @brief Access the variable represented by the page number.
/// @param pageNumber the page number in the virtual table for the variable.
/// @return the pointer address for the memory page being accessed.
void *access(int pageNumber)
{
    // TODO: if page is in disk, use page replacement algorithm (LRU):
    //      - remove old page from heap and put it in disk (fwrite)
    //      - Get needed page from disk and put it back in heap (fopen)
    //      - return new memory address of the page requested
    // TODO: if page is in heap, return page address

    return NULL;
}

/// @brief Frees up the memory used by this pointer
/// @param ptr The pointer for which the memory is to be freed up
void pm_free(int pageNumber)
{
    printf("free started\n");

    // Mutex acquired for allocating memory in the heap
    pthread_mutex_lock(&heap_access_mutex);

    // TODO: destroy Page object in pageMapping array
    // TODO: set pageMapping value for this pageNumber to NULL

    pthread_mutex_unlock(&heap_access_mutex);

    printf("free complete\n\n");
}
