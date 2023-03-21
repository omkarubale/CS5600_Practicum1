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
#include <errno.h>
#include <limits.h>
#include "pm_heap.h"

char pm_heap[HEAP_SIZE_IN_MEGA_BYTES * PAGE_SIZE * 256];
int heapUsage[HEAP_SIZE_IN_MEGA_BYTES * 256];
t_VirtualPageTableEntry virtualPageTable[2 * HEAP_SIZE_IN_MEGA_BYTES * 256];

#define MAX_TIME_TV_SEC 67767976233521999;
#define MAX_TIME_TV_USEC 9223372036854775807;

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

    memset(pm_heap, '0', sizeof(HEAP_SIZE_IN_MEGA_BYTES * PAGE_SIZE * 256));
    memset(heapUsage, 0, sizeof(HEAP_SIZE_IN_MEGA_BYTES * 256));
    memset(virtualPageTable, NULL, 2 * HEAP_SIZE_IN_MEGA_BYTES * 256 * sizeof(t_VirtualPageTableEntry));

    printf("init complete\n\n");
}

void pm_files_cleanup()
{
    printf("\nCLEANUP: removing pages temp files created.\n");
    system("exec rm -r ./pages/*");
    printf("CLEANUP: cleanup complete.\n");
}

/// @brief Creates the virtual page table entry and adds the page in the heap
/// @param virtualPageNumber The virtual page number of the page that is being created.
/// @param heapPageNumber The heap page number of the page that is being created.
/// @param name the name of the variable for which the memory is being allocated.
/// @param type the type of the variable for which the memory is being allocated.
void createPage(int virtualPageNumber, int heapPageNumber, char *name, char *type)
{
    printf("CREATE PAGE: create page started\n");
    t_VirtualPageTableEntry *page = &(virtualPageTable[virtualPageNumber]);

    page->isValid = true;
    page->inHeap = true;
    page->name = name;
    page->type = type;
    page->pageNumberInHeap = heapPageNumber;
    page->pageNumberInDisk = virtualPageNumber;
    struct timeval t;
    gettimeofday(&t, NULL);

    page->lastAccessed = t;

    printf("CREATE PAGE: create page complete\n");
    printf("PAGE: name: %s type: %s pageNumberInHeap: %d pageNumberInDisk: %d\n", page->name, page->type, page->pageNumberInHeap, page->pageNumberInDisk);
}

/// @brief Moves page to disk.
/// @param virtualPageNumber the virtual page number of the page that has to be moved to disk.
void movePageToDisk(int virtualPageNumber)
{
    t_VirtualPageTableEntry *page = &(virtualPageTable[virtualPageNumber]);

    FILE *fptr = NULL;

    // write to file with name as virtualPageNumber
    char output_filname[50];
    sprintf(output_filname, "./pages/%d.txt", virtualPageNumber);

    // build page contents to be moved to disk
    char pageContents[PAGE_SIZE];
    int pageStartInHeap = page->pageNumberInHeap * (PAGE_SIZE);

    memcpy(pageContents, pm_heap + pageStartInHeap, PAGE_SIZE);

    // move page contents to disk
    fptr = fopen(output_filname, "w");

    printf("MOVE: movePageToDisk: vpageNumber= %d, filename: %s\n", virtualPageNumber, output_filname);

    if (fptr == NULL)
    {
        printf("\nERROR: file creation failed!\n");
        printf("Error : errno='%s'.\n", strerror(errno));
        exit(1);
    }

    fputs(pageContents, fptr);
    fclose(fptr);

    // set page to in disk
    page->inHeap = false;
    page->pageNumberInDisk = virtualPageNumber;
}

/// @brief Moves the virtual page from disk to heap in the specified page in heap.
/// @param virtualPageNumber the virtual page being moved from the disk to heap.
/// @param heapPageAvailable the empty page in the heap where the page can be moved.
void movePageToHeap(int virtualPageNumber, int heapPageAvailable)
{
    t_VirtualPageTableEntry *page = &(virtualPageTable[virtualPageNumber]);

    FILE *fptr = NULL;

    // write to file with name as virtualPageNumber
    char input_filname[50];
    sprintf(input_filname, "./pages/%d", virtualPageNumber);

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
    struct timeval t;
    gettimeofday(&t, NULL);

    page->lastAccessed = t;
}

/// @brief Finds the first page in the heap that is vacant.
/// @return the heap page number which is vacant.
int findVacantHeapPage()
{
    for (int i = 0; i < (HEAP_SIZE_IN_MEGA_BYTES * 256); i++)
    {
        if (heapUsage[i] == 0)
        {
            return i;
        }
    }

    return -1;
}

/// @brief Allocates memory and gives the page number in the virtual page table.
/// @param size the size of memory requested (cannot be more than 4KB)
/// @return the page number of the allocated memory in the virtual page table.
int pm_malloc(int size, char *name, char *type)
{
    printf("MALLOC: malloc started for size %d\n", size);

    printf("MALLOC: sanity checks started\n");
    if (size > PAGE_SIZE)
    {
        printf("\nERROR: malloc not allowed for size greater than page size.\n");
        return -1;
    }

    printf("MALLOC: sanity checks complete\n");

    // Mutex acquired for allocating memory in the heap
    pthread_mutex_lock(&heap_access_mutex);

    printf("MALLOC: mutex acquired\n");

    int pagesInHeap = 0;

    struct timeval oldestPageTime;
    gettimeofday(&oldestPageTime, NULL);

    int oldestVirtualPageNumber = 0;
    int oldestHeapPageNumber = 0;
    int virtualPageAvailable = -1;

    printf("MALLOC: iteration through entire page table started\n");

    // iterate through entire virtual page table
    for (int i = 0; i < (2 * HEAP_SIZE_IN_MEGA_BYTES * 256); i++)
    {
        t_VirtualPageTableEntry *page = &(virtualPageTable[i]);
        // printf("MALLOC: iteration: %d\n", i);
        // current page is not available in virtualPageTable
        if (page->isValid)
        {
            // printf("MALLOC: page %d not available in virtual page table\n", i);

            // increment pages count in heap
            if (page->inHeap)
            {
                // printf("MALLOC: incrementing pages found so far\n");
                pagesInHeap++;

                // set oldest page to current page if this page is older than oldest page so far
                if (oldestPageTime.tv_sec - page->lastAccessed.tv_sec > 0 ||
                    (oldestPageTime.tv_sec == page->lastAccessed.tv_sec && oldestPageTime.tv_usec - page->lastAccessed.tv_usec > 0))
                {
                    printf("MALLOC: older page found %d\n", i);

                    oldestVirtualPageNumber = i;
                    oldestPageTime = page->lastAccessed;
                    oldestHeapPageNumber = page->pageNumberInHeap;
                }
            }
        }
        // current page is available in virtualPageTable
        else
        {
            // printf("MALLOC: page %d available in virtual page table\n", i);

            if (virtualPageAvailable == -1)
            {
                printf("MALLOC: page %d set as virtual page available\n", i);
                virtualPageAvailable = i;
            }
        }
    }

    printf("MALLOC: iteration through virtual page table complete\n");

    // heap is not full
    if (pagesInHeap < (HEAP_SIZE_IN_MEGA_BYTES * 256))
    {
        int vacantHeapPageNumber = findVacantHeapPage();
        printf("MALLOC: heap is not full: creating page\n");
        createPage(virtualPageAvailable, vacantHeapPageNumber, name, type);
        printf("MALLOC: creation of page complete\n");
        heapUsage[vacantHeapPageNumber] = 1;

        pthread_mutex_unlock(&heap_access_mutex);

        printf("MALLOC: mutex released\n");

        printf("MALLOC: malloc ended for heap not full %d\n\n", size);
        return virtualPageAvailable;
    }

    // virtualPageTable is full
    if (virtualPageAvailable == -1)
    {
        pthread_mutex_unlock(&heap_access_mutex);
        printf("MALLOC: mutex released\n");
        printf("ERROR: no more pages available in the memory heap.\n");
        return -1;
    }

    // heap is full and virtualPageTable is not full

    // write oldest page to disk
    printf("MALLOC: writing old page to disk started\n");
    movePageToDisk(oldestVirtualPageNumber);
    printf("MALLOC: writing old page to disk complete\n");

    // add the new page to heap and add to virtualPageTable
    printf("MALLOC: heap was full, old page moved to disk, creating new page\n");
    createPage(virtualPageAvailable, oldestHeapPageNumber, name, type);
    printf("MALLOC: creation of page complete after page swap\n");

    pthread_mutex_unlock(&heap_access_mutex);

    printf("MALLOC: mutex released\n");

    printf("MALLOC: malloc ended with heap full %d\n\n", size);

    return virtualPageAvailable;
}

/// @brief Access the variable represented by the page number.
/// @param pageNumber the page number in the virtual table for the variable.
/// @return the pointer address for the memory page being accessed.
void *pm_access(int pageNumber)
{
    printf("ACCESS: sanity checks started\n");

    // sanity checks
    if (pageNumber < 0 || pageNumber > (2 * HEAP_SIZE_IN_MEGA_BYTES * 256))
    {
        printf("\nERROR: pageNumber requested is out of bounds!\n");
        exit(1);
    }

    t_VirtualPageTableEntry *page = &(virtualPageTable[pageNumber]);

    if (!page->isValid)
    {
        printf("\nERROR: page does not exist!\n");
        return NULL;
    }

    printf("ACCESS: sanity checks complete\n");

    // NOTE: page replacement algorithm: Least Recently Used

    // get page into heap if it is in disk
    if (!page->inHeap)
    {
        printf("ACCESS: requested page is in disk, need to get it into heap\n");
        pthread_mutex_lock(&heap_access_mutex);

        printf("ACCESS: mutex acquired\n");

        // find oldest page in heap to be moved to disk
        int pagesInHeap = 0;

        struct timeval oldestPageTime;
        gettimeofday(&oldestPageTime, NULL);

        int oldestVirtualPageNumber = 0;
        int oldestHeapPageNumber = 0;
        int virtualPageAvailable = -1;

        printf("ACCESS: iteration through virtual page table started\n");
        for (int i = 0; i < (2 * HEAP_SIZE_IN_MEGA_BYTES * 256); i++)
        {
            t_VirtualPageTableEntry *cpage = &(virtualPageTable[i]);

            // current page is not available in virtualPageTable
            if (cpage->isValid)
            {
                printf("ACCESS: incrementing pages found so far\n");
                pagesInHeap++;

                // set oldest page to current page if this page is older than oldest page so far
                if (oldestPageTime.tv_sec - cpage->lastAccessed.tv_sec > 0 ||
                    (oldestPageTime.tv_sec - cpage->lastAccessed.tv_sec == 0 && oldestPageTime.tv_usec - cpage->lastAccessed.tv_usec > 0))
                {
                    printf("ACCESS: older page found\n");

                    oldestVirtualPageNumber = i;
                    oldestPageTime = cpage->lastAccessed;
                    oldestHeapPageNumber = cpage->pageNumberInHeap;
                }

                continue;
            }

            // current page is available in virtualPageTable (oldest page doesn't need to be moved to disk)
            if (virtualPageAvailable == -1)
            {
                virtualPageAvailable = i;
            }
        }

        printf("ACCESS: iteration through virtual page table complete\n");

        // if heap is full, create space by moving oldest page to disk
        if (pagesInHeap == (HEAP_SIZE_IN_MEGA_BYTES * 256))
        {
            printf("ACCESS: heap is full, swapping oldest page to disk started\n");
            movePageToDisk(oldestVirtualPageNumber);
            virtualPageAvailable = oldestVirtualPageNumber;
            printf("ACCESS: heap is full, swapping oldest page to disk complete\n");

            // move the page from disk to available page in heap
            printf("ACCESS: heap is full, moving requested page to heap started\n");
            movePageToHeap(pageNumber, oldestHeapPageNumber);
            printf("ACCESS: heap is full, moving requested page to heap complete\n");
        }
        // heap is not full, page doesn't need to be moved to disk, but empty space in heap needs to be used
        else
        {
            printf("ACCESS: heap is not full, moving requested page to heap started\n");
            int vacantHeapPageNumber = findVacantHeapPage();
            // move the page from disk to available page in heap
            movePageToHeap(pageNumber, vacantHeapPageNumber);
            printf("ACCESS: heap is not full, moving requested page to heap complete\n");
        }

        pthread_mutex_unlock(&heap_access_mutex);

        printf("ACCESS: mutex released\n");
    }

    printf("ACCESS: page ready.\n");
    // set last accessed of the requested page to now
    struct timeval t;
    gettimeofday(&t, NULL);

    virtualPageTable[pageNumber].lastAccessed = t;

    // return page start address in heap
    printf("ACCESS: building resultant pointer for requested page's start address started\n");
    int heapPageNumber = virtualPageTable[pageNumber].pageNumberInHeap;
    void *result = pm_heap + (PAGE_SIZE * heapPageNumber);
    printf("ACCESS: building resultant pointer for requested page's start address complete\n\n");

    return result;
}

/// @brief Frees up the memory used by this pointer
/// @param ptr The pointer for which the memory is to be freed up
void pm_free(int pageNumber)
{
    printf("FREE: free started\n");

    // Mutex acquired for allocating memory in the heap
    pthread_mutex_lock(&heap_access_mutex);

    // destroy Page object in virtualPageTable array
    // printf("FREE: destroy page object\n");
    // t_VirtualPageTableEntry *pageObjectToBeDeleted = ((t_VirtualPageTableEntry *)virtualPageTable[pageNumber]);
    // free(pageObjectToBeDeleted);

    printf("FREE: set page to available in virtual page table and pageUsage table\n");
    virtualPageTable[pageNumber].isValid = false;
    heapUsage[pageNumber] = 0;

    printf("FREE: page successfully marked as not available\n");

    pthread_mutex_unlock(&heap_access_mutex);

    printf("FREE: free complete\n\n");
}
