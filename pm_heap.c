/*
 * pm_heap.c / Add Thread Safety
 *
 * Omkar Ubale / CS5600 / Northeastern University
 * Spring 2023 / Feb 27, 2023
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
int pageUsage[HEAP_SIZE_IN_MEGA_BYTES * 256];
int pageNext[HEAP_SIZE_IN_MEGA_BYTES * 256];

pthread_mutex_t allocation_mutex;

/// @brief Initializes the heap.
void pm_init()
{
    printf("init started\n");

    // initialize mutex for output file
    if (pthread_mutex_init(&allocation_mutex, NULL) != 0)
    {
        printf("\nERROR: output file mutex init failed\n");
        exit(1);
    }

    memset(pm_heap, '0', sizeof(pm_heap));
    memset(pageUsage, 0, sizeof(pageUsage));
    memset(pageNext, -1, sizeof(pageNext));

    printf("init complete\n\n");
}

/// @brief Allocates memory and gives the start address of the memory.
/// @param size the size of memory requested
/// @return the start address of the memory location allocated.
void *pm_malloc(int size)
{
    printf("malloc started for size %d\n", size);

    // Mutex acquired for allocating memory in the heap
    pthread_mutex_lock(&allocation_mutex);

    int sizeRequired = size;
    int *allocatedMemoryStart = 0;
    // find available memory location
    for (int i = 0; i < HEAP_SIZE_IN_MEGA_BYTES * 256; i++)
    {
        // we found the first page available in this possible block of contiguous memory
        if (pageUsage[i] == 0)
        {
            // set the start of the memory address of page as the location to return
            allocatedMemoryStart = pm_heap + (i * PAGE_SIZE);

            // space is needed
            while (sizeRequired > 0 && i < HEAP_SIZE_IN_MEGA_BYTES * 256)
            {
                if (pageUsage[i] == 0)
                {
                    // set the start of the block if this is the start of the allocated memory
                    if (allocatedMemoryStart == 0)
                    {
                        allocatedMemoryStart = pm_heap + (i * PAGE_SIZE);
                    }
                    // this page is needed
                    pageUsage[i] = 1;
                }
                else
                {
                    // reset allocated blocks to de-allocated
                    int startPageNumber = ((void *)allocatedMemoryStart - (void *)pm_heap) / (PAGE_SIZE);

                    while (startPageNumber < i)
                    {
                        pageUsage[startPageNumber] = 0;
                        pageNext[startPageNumber] = -1;
                        startPageNumber++;
                    }

                    // Reset the allocation so it can be done when next contiguous block is found
                    allocatedMemoryStart = 0;
                    sizeRequired = size;
                    break;
                }

                i++;

                // more than this page is needed.
                if (sizeRequired > PAGE_SIZE)
                {
                    sizeRequired -= PAGE_SIZE;
                    pageNext[i - 1] = i;
                }
                // this page is enough.
                else
                {
                    pthread_mutex_unlock(&allocation_mutex);

                    printf("malloc complete inside loop\n\n");
                    return allocatedMemoryStart;
                }
            }
        }
    }

    pthread_mutex_unlock(&allocation_mutex);

    printf("malloc complete outside loop\n\n");

    return sizeRequired > 0 ? 0 : allocatedMemoryStart;
}

/// @brief Frees up the memory used by this pointer
/// @param ptr The pointer for which the memory is to be freed up
void pm_free(void *ptr)
{
    printf("free started\n");

    // Mutex acquired for allocating memory in the heap
    pthread_mutex_lock(&allocation_mutex);

    int pageNumber = pm_getPageNumber(ptr);

    while (true)
    {
        // This is the end of this contiguous block of allocated memory
        if (pageNext[pageNumber] == -1)
        {
            pageUsage[pageNumber] = 0;
            break;
        }
        // There are more pages left in this contiguous block of memory
        else if (pageNext[pageNumber] != pageNumber + 1)
        {
            break;
        }
        // marking this page as free
        pageNext[pageNumber] = -1;
        pageUsage[pageNumber] = 0;

        pageNumber++;
    }

    pthread_mutex_unlock(&allocation_mutex);

    printf("free complete\n\n");
}

/// @brief Gets the page number used in the heap for the given pointer
/// @param ptr the given pointer
/// @return the page number in the heap
int pm_getPageNumber(void *ptr)
{
    int *p = ptr;
    return (((void *)p - (void *)pm_heap) / (PAGE_SIZE));
}
