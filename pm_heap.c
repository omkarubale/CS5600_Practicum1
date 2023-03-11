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
#include <time.h>
#include "pm_heap.h"

char pm_heap[HEAP_SIZE_IN_MEGA_BYTES * PAGE_SIZE * 256];
void *pageMapping[2 * HEAP_SIZE_IN_MEGA_BYTES * 256];

#define MAX_TIME 67767976233521999;

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

void createPage(int pageNumber, char *name, char *type)
{
    t_Page *page;

    page->inHeap = true;
    page->name = name;
    page->type = type;
    page->pageNumberInHeap = pageNumber;

    time_t t;
    page->lastAccessed = time(t);

    pageMapping[pageNumber] = (void *)page;
}

/// @brief Moves page to disk.
/// @param pageNumber the virtual page number of the page that has to be moved to disk.
void movePageToDisk(int pageNumber)
{
    t_Page *page = ((t_Page *)pageMapping[pageNumber]);

    FILE *fptr = NULL;

    // write to file with name as pageNumber
    char output_filname[50];
    sprintf(output_filname, "%d", pageNumber);

    // build page contents to be moved to disk
    char pageContents[PAGE_SIZE];
    int pageStartInHeap = page->pageNumberInHeap * (PAGE_SIZE);

    for (int i = 0; i < (PAGE_SIZE); i++)
    {
        pageContents[i] = pm_heap[pageStartInHeap + i];
    }

    // move page contents to disk
    fptr = fopen(output_filname, "a+");
    fputs(pageContents, fptr);
    fclose(fptr);

    // set page to in disk
    page->inHeap = false;
    page->pageNumberInDisk = pageNumber;
}

void movePageToHeap()
{
}

/// @brief Allocates memory and gives the page number in the virtual page table.
/// @param size the size of memory requested (cannot be more than 4KB)
/// @return the page number of the allocated memory in the virtual page table.
int pm_malloc(int size, char *name, char *type)
{
    printf("malloc started for size %d\n", size);

    // Mutex acquired for allocating memory in the heap
    pthread_mutex_lock(&heap_access_mutex);

    int pagesInHeap = 0;
    time_t oldestPageTime = MAX_TIME;
    int oldestPageNumber = 0;
    int pageAvailable = -1;

    for (int i = 0; i < (2 * HEAP_SIZE_IN_MEGA_BYTES * 256); i++)
    {
        // current page is not available in pageMapping
        if (pageMapping[i] != NULL)
        {
            // increment pages count in heap
            if (((t_Page *)pageMapping[i])->inHeap == true)
            {
                pagesInHeap++;
            }

            // set oldest page to current page if this page is older than oldest page so far
            if (difftime(((t_Page *)pageMapping[i])->lastAccessed, oldestPageTime) > 0)
            {
                oldestPageNumber = i;
                oldestPageTime = ((t_Page *)pageMapping[i])->lastAccessed;
            }

            continue;
        }

        // current page is available in pageMapping
        if (pageAvailable == -1)
        {
            pageAvailable = i;
        }
    }

    // heap is not full
    if (pagesInHeap < (HEAP_SIZE_IN_MEGA_BYTES * 256))
    {
        createPage(pageAvailable, name, type);

        pthread_mutex_unlock(&heap_access_mutex);
        return pageAvailable;
    }

    // pageMapping is full
    if (pageAvailable == -1)
    {
        return -1;
    }

    // heap is full and pageMapping is not full

    // write oldest page to disk
    movePageToDisk(oldestPageNumber);

    // add the new page to heap and add to pageMapping
    createPage(pageAvailable, name, type);

    pthread_mutex_unlock(&heap_access_mutex);

    return pageAvailable;
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

    pthread_mutex_lock(&heap_access_mutex);

    if (!pageMapping[page_num]->inHeap)
    {

        // page not in physical memory. Reteriving from disk.
        void *page_ptr = load_page_disk(pageNumber); // to be implemented

        // find replacement using LRU
        time_t oldest_last_used = time(NULL);
        int num_of_pages = sizeof(pageMapping); // Right??
        for (int i = 0; i < no_of_pages; i++)
        {
            if (pageMappping[i]->inHeap && pageMapping[i]->lastAccessed < victim_time)
            {
                victim_page = i;
                oldest_last_used = pageMapping[i]->lastAccessed;
            }
        }

        // write the victim page to disk ??
        write_page_disk(victim_page); // to be implemented

        // replacing the victim page with new page

        memcpy(pageMapping[victim_page]->data, page_ptr, PAGE_SiZE);
        free(page_ptr);
    }
    pageMapping[pageNumber]->lastAccessed = time(NULL);
    pthread_mutex_lock(&heap_access_mutex);
    return pageMapping[pageNumber]->data;
}

/// @brief Frees up the memory used by this pointer
/// @param ptr The pointer for which the memory is to be freed up
void pm_free(int pageNumber)
{
    printf("free started\n");

    // Mutex acquired for allocating memory in the heap
    pthread_mutex_lock(&heap_access_mutex);

    // destroy Page object in pageMapping array
    free(((t_Page *)pageMapping[pageNumber]));
    pageMapping[pageNumber] = NULL;

    pthread_mutex_unlock(&heap_access_mutex);

    printf("free complete\n\n");
}
