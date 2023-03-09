/*
 * pm_heap.h / Practicum 1
 *
 * Omkar Ubale, Ujwal Gupta / CS5600 / Northeastern University
 * Spring 2023 / March 9, 2023
 *
 * For documentation, Doxygen commenting has been used for better readability in
 * IDEs like VS Code.
 */

#define HEAP_SIZE_IN_MEGA_BYTES 10
// 1 page - 4 KB : 4 * 1024
#define PAGE_SIZE 4 * 1024

#define PAGE_DISK_DIRECTORY "TODO"

typedef struct s_Page
{
    char *name;
    char *type;
    bool inHeap;
    int pageNumberInHeap;
    int pageNumberInDisk;
    time_t lastAccessed;
} t_Page;

void pm_init();

int pm_malloc(int size, char *name, char *type);

void *access(int pageNumber);

void pm_free(int pageNumber);