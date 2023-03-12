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
int heapUsage[HEAP_SIZE_IN_MEGA_BYTES * 256];
void *virtualPageTable[2 * HEAP_SIZE_IN_MEGA_BYTES * 256];

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

    memset(pm_heap, '0', sizeof(HEAP_SIZE_IN_MEGA_BYTES * PAGE_SIZE * 256));
    memset(heapUsage, 0, sizeof(HEAP_SIZE_IN_MEGA_BYTES * 256));
    memset(virtualPageTable, NULL, sizeof(2 * HEAP_SIZE_IN_MEGA_BYTES * 256));

    printf("init complete\n\n");
}

/// @brief Creates the virtual page table entry and adds the page in the heap
/// @param virtualPageNumber The virtual page number of the page that is being created.
/// @param heapPageNumber The heap page number of the page that is being created.
/// @param name the name of the variable for which the memory is being allocated.
/// @param type the type of the variable for which the memory is being allocated.
void createPage(int virtualPageNumber, int heapPageNumber, char *name, char *type)
{
	printf("Hy11\n");
	t_VirtualPageTableEntry *page = &virtualPageTable[virtualPageNumber];
	page->inHeap = true;
	page->name = name;
	page->type = type;
	page->pageNumberInHeap = heapPageNumber;
	page->lastAccessed = time(NULL);
}

/// @brief Moves page to disk.
/// @param virtualPageNumber the virtual page number of the page that has to be moved to disk.
void movePageToDisk(int virtualPageNumber)
{
    t_VirtualPageTableEntry *page = ((t_VirtualPageTableEntry *)virtualPageTable[virtualPageNumber]);

    FILE *fptr = NULL;

    // write to file with name as virtualPageNumber
    char output_filname[50];
    sprintf(output_filname, "./pages/%d", virtualPageNumber);

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
    page->pageNumberInDisk = virtualPageNumber;
}

/// @brief Moves the virtual page from disk to heap in the specified page in heap.
/// @param virtualPageNumber the virtual page being moved from the disk to heap.
/// @param heapPageAvailable the empty page in the heap where the page can be moved.
void movePageToHeap(int virtualPageNumber, int heapPageAvailable)
{
    t_VirtualPageTableEntry *page = ((t_VirtualPageTableEntry *)virtualPageTable[virtualPageNumber]);

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
    printf("malloc started for size %d\n", size);

    // Mutex acquired for allocating memory in the heap
    pthread_mutex_lock(&heap_access_mutex);

    int pagesInHeap = 0;
    time_t oldestPageTime = MAX_TIME;
    int oldestVirtualPageNumber = 0;
    int oldestHeapPageNumber = 0;
    int virtualPageAvailable = -1;

    // iterate through entire virtual page table
    for (int i = 0; i < (2 * HEAP_SIZE_IN_MEGA_BYTES * 256); i++)
    {
    	printf("Hy20\n");
        // current page is not available in virtualPageTable
        if (virtualPageTable[i] != NULL)
        {
        	printf("Hy21\n");
            // increment pages count in heap
            if (((t_VirtualPageTableEntry *)virtualPageTable[i])->inHeap == true)
            {
            	printf("Hy22\n");
                pagesInHeap++;

                // set oldest page to current page if this page is older than oldest page so far
                if (difftime(((t_VirtualPageTableEntry *)virtualPageTable[i])->lastAccessed, oldestPageTime) > 0)
                {
                    oldestVirtualPageNumber = i;
                    oldestPageTime = ((t_VirtualPageTableEntry *)virtualPageTable[i])->lastAccessed;
                    oldestHeapPageNumber = ((t_VirtualPageTableEntry *)virtualPageTable[i])->pageNumberInHeap;
                }
            }

            continue;
        }

        // current page is available in virtualPageTable
        if (virtualPageAvailable == -1)
        {
            virtualPageAvailable = i;
        }
    }

    // heap is not full
    if (pagesInHeap < (HEAP_SIZE_IN_MEGA_BYTES * 256))
    {
        int vacantHeapPageNumber = findVacantHeapPage();
        printf("Hy21\n");
        createPage(virtualPageAvailable, vacantHeapPageNumber, name, type);
        printf("Hy22\n");
        heapUsage[vacantHeapPageNumber] = 1;

        pthread_mutex_unlock(&heap_access_mutex);

        printf("malloc ended for heap not full %d\n", size);
        return virtualPageAvailable;
    }

    // virtualPageTable is full
    if (virtualPageAvailable == -1)
    {
        return -1;
    }


    // heap is full and virtualPageTable is not full

    // write oldest page to disk
    movePageToDisk(oldestVirtualPageNumber);

    // add the new page to heap and add to virtualPageTable
    createPage(virtualPageAvailable, oldestHeapPageNumber, name, type);

    printf("malloc ended with heap full %d\n", size);

    pthread_mutex_unlock(&heap_access_mutex);

    return virtualPageAvailable;
}

/// @brief Access the variable represented by the page number.
/// @param pageNumber the page number in the virtual table for the variable.
/// @return the pointer address for the memory page being accessed.
void *access(int pageNumber)
{
    // sanity checks
    if (pageNumber < 0 || pageNumber > (2 * HEAP_SIZE_IN_MEGA_BYTES * 256))
    {
        printf("\nERROR: pageNumber requested is out of bounds!\n");
        exit(1);
    }

    if (virtualPageTable[pageNumber] == NULL)
    {
        printf("\nERROR: page does not exist!\n");
        exit(1);
    }
    printf("Hy21\n");
    // NOTE: page replacement algorithm: Least Recently Used

    pthread_mutex_lock(&heap_access_mutex);
    printf("Hy22\n");
    // get page into heap if it is in disk
    if (((t_VirtualPageTableEntry *)virtualPageTable[pageNumber])->inHeap == false)
    {
        // find oldest page in heap to be moved to disk
    	printf("Hy23\n");
        int pagesInHeap = 0;
        time_t oldestPageTime = MAX_TIME;
        int oldestVirtualPageNumber = 0;
        int oldestHeapPageNumber = 0;
        int virtualPageAvailable = -1;

        for (int i = 0; i < (2 * HEAP_SIZE_IN_MEGA_BYTES * 256); i++)
        {
        	printf("Hy24\n");
            // current page is not available in virtualPageTable
            if (virtualPageTable[i] != NULL)
            {
            	printf("Hy25\n");
                // set oldest page to current page if this page is older than oldest page so far
                if (difftime(((t_VirtualPageTableEntry *)virtualPageTable[i])->lastAccessed, oldestPageTime) > 0)
                {
                	printf("Hy26\n");
                    oldestVirtualPageNumber = i;
                    oldestPageTime = ((t_VirtualPageTableEntry *)virtualPageTable[i])->lastAccessed;
                    oldestHeapPageNumber = ((t_VirtualPageTableEntry *)virtualPageTable[i])->pageNumberInHeap;
                }
                printf("Hy27\n");
                continue;
            }

            printf("Hy28\n");
            // current page is available in virtualPageTable (oldest page doesn't need to be moved to disk)
            if (virtualPageAvailable == -1)
            {
                virtualPageAvailable = i;
            }
        }

        printf("Hy29\n");
        // if heap is full, create space by moving oldest page to disk
        if (pagesInHeap == (HEAP_SIZE_IN_MEGA_BYTES * 256))
        {
            movePageToDisk(oldestVirtualPageNumber);
            virtualPageAvailable = oldestVirtualPageNumber;

            // move the page from disk to available page in heap
            movePageToHeap(pageNumber, oldestHeapPageNumber);
        }
        // heap is not full, page doesn't need to be moved to disk, but empty space in heap needs to be used
        else
        {
            int vacantHeapPageNumber = findVacantHeapPage();
            // move the page from disk to available page in heap
            movePageToHeap(pageNumber, vacantHeapPageNumber);
        }
    }

    // set last accessed of the requested page to now
    time_t t;
    ((t_VirtualPageTableEntry *)virtualPageTable[pageNumber])->lastAccessed = time(t);

    pthread_mutex_lock(&heap_access_mutex);

    // return page start address in heap
    int heapPageNumber = ((t_VirtualPageTableEntry *)virtualPageTable[pageNumber])->pageNumberInHeap;
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
    printf("Hy31\n");
    // destroy Page object in virtualPageTable array
    //    free(((t_VirtualPageTableEntry *)virtualPageTable[pageNumber]));
    printf("Hy32\n");
    virtualPageTable[pageNumber] = NULL;

    heapUsage[pageNumber] = 0;
    printf("Hy33\n");

    // Zero out the memory in the heap
    // Get the page number in heap and mark it as free in the heapUsage array
	//	int heapPageNumber = ((t_VirtualPageTableEntry *)virtualPageTable[pageNumber])->pageNumberInHeap;
	//	printf("Hy34\n");
	//	heapUsage[heapPageNumber] = 0;
	//    memset(pm_heap + (heapPageNumber * PAGE_SIZE), 0, PAGE_SIZE);

    pthread_mutex_unlock(&heap_access_mutex);

    printf("free complete\n\n");
}
