/*
 * test.c / Practicum 1
 *
 * Omkar Ubale, Ujwal Gupta / CS5600 / Northeastern University
 * Spring 2023 / March 9, 2023
 *
 * For documentation, Doxygen commenting has been used for better readability in
 * IDEs like VS Code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "pm_heap.h"

void test_alloc_access_dealloc(int size, char *name, char *type)
{
    // Allocate memory
    int page_num = pm_malloc(size, name, type);
    printf("Allocated memory at page number %d\n", page_num);

    // Access memory
//    void *ptr = access(page_num);
//    printf("Accessed memory at page number %d\n", page_num);

    // Deallocate memory
    pm_free(page_num);
    printf("Deallocated memory at page number %d\n", page_num);
}

int main(int argc, char **argv)
{

	// Initialize the heap
	pm_init();

	// Test allocation, access, and deallocation of different sizes
	test_alloc_access_dealloc(100, "short_string", "char");
	test_alloc_access_dealloc(400, "medium_string", "char");
	test_alloc_access_dealloc(1200, "long_string", "char");
	test_alloc_access_dealloc(sizeof(int), "integer", "int");

    return 0;
}
