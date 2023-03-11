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

    memcpy(pageContents, pm_heap + pageStartInHeap, PAGE_SIZE);

    // move page contents to disk
    fptr = fopen(output_filname, "w");

    if (fptr == NULL)
    {
        printf("\nERROR: file creation failed!\n");
        exit(1);
    }

    fputs(pageContents, fptr);
    fclose(fptr);

    // set page to in disk
    page->inHeap = false;
    page->pageNumberInDisk = pageNumber;
}

/// @brief Moves the virtual page from disk to heap in the specified page in heap.
/// @param pageNumber the virtual page being moved from the disk to heap.
/// @param heapPageAvailable the empty page in the heap where the page can be moved.
void movePageToHeap(int pageNumber, int heapPageAvailable)
{
    t_Page *page = ((t_Page *)pageMapping[pageNumber]);

    FILE *fptr = NULL;

    // write to file with name as pageNumber
    char input_filname[50];
    sprintf(input_filname, "%d", pageNumber);

    fptr = fopen(input_filname, "r");

    if (fptr == NULL)
    {
        printf("\nERROR: file on disk not found!\n");
        exit(1);
    }

    // get contents from disk character by character and place it in heap
    char ch = fgetc(fptr);
    int i = heapPageAvailable * (PAGE_SIZE);

    while (ch != EOF)
    {
        pm_heap[i] = ch;
        ch = fgetc(fptr);
    }

    // set page to in heap
    page->inHeap = true;
    page->pageNumberInHeap = heapPageAvailable;
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
    // NOTE: page replacement algorithm: Least Recently Used

    pthread_mutex_lock(&heap_access_mutex);

    // get page into heap if it is in disk
    if (((t_Page *)pageMapping[pageNumber])->inHeap == false)
    {
        // find oldest page in heap to be moved to disk
        time_t oldestPageTime = MAX_TIME;
        int oldestPageNumber = 0;
        int pageAvailable = -1;

        for (int i = 0; i < (2 * HEAP_SIZE_IN_MEGA_BYTES * 256); i++)
        {
            // current page is not available in pageMapping
            if (pageMapping[i] != NULL)
            {
                // set oldest page to current page if this page is older than oldest page so far
                if (difftime(((t_Page *)pageMapping[i])->lastAccessed, oldestPageTime) > 0)
                {
                    oldestPageNumber = i;
                    oldestPageTime = ((t_Page *)pageMapping[i])->lastAccessed;
                }

                continue;
            }

            // current page is available in pageMapping (oldest page doesn't need to be moved to disk)
            if (pageAvailable == -1)
            {
                pageAvailable = i;
            }
        }

        // if heap is full, create space by moving oldest page to disk
        if (pageAvailable == -1)
        {
            movePageToDisk(oldestPageNumber);
            pageAvailable = oldestPageNumber;
        }

        // move the page from disk to available page in heap
        movePageToHeap(pageNumber, pageAvailable);
    }

    // set last accessed of the requested page to now
    time_t t;
    ((t_Page *)pageMapping[pageNumber])->lastAccessed = time(t);

    pthread_mutex_lock(&heap_access_mutex);

    // return page start address in heap
    int heapPageNumber = ((t_Page *)pageMapping[pageNumber])->pageNumberInHeap;
    void *result = pm_heap + (PAGE_SIZE * heapPageNumber);

    return result;
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
