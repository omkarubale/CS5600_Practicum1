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
#include <assert.h>
#include <pthread.h>
#include "pm_heap.h"

void test_successive_allocation()
{
    printf("***TESTING: test started.\n");
    // Test successive allocation
    int a = pm_malloc(100, "short_string", "char");
    int b = pm_malloc(400, "medium_string", "char");
    int c = pm_malloc(1200, "long_string", "char");
    int d = pm_malloc(sizeof(int), "integer", "int");

    printf("\n\n*** TESTING: pages requested: %d %d %d %d\n", a, b, c, d);

    void *t = pm_access(c);

    printf("\n\n*** TESTING: access for page %d requested\n", c);

    pm_free(0);
    pm_free(1);
    pm_free(2);
    pm_free(3);

    printf("\n\n*** TESTING: pages freed: %d %d %d %d\n", a, b, c, d);
    printf("TESTING: test complete.\n");
}

void test_write_read()
{

    // Allocate memory for a string and store it in the heap
    char *str_ptr = (char *)pm_access(pm_malloc(sizeof(char) * 10, "my_str", "string"));
    strcpy(str_ptr, "hello");

    // Check if we can access the memory we just allocated
    assert(strcmp(str_ptr, "hello") == 0);

    printf("\n\n***Test Complete: Data written to allocated space validated");
}

void test_access_invalid()
{
    int a = pm_malloc(100, "some_string", "char");

    void *space = (char *)pm_access(a);

    pm_free(a);

    bool error_occurred = false;
    // attempt to access a page that's no longer in the heap
    if ((void *)pm_access(a) == NULL)
    {
        error_occurred = true;
    }
    assert(error_occurred);
    printf("\nTest Complete: Trying to access an invalid allocation.\n\n");
}

void test_out_of_memory_pages()
{
    // Test out of memory error
    int ptrs[(2 * HEAP_SIZE_IN_MEGA_BYTES * 256) + 1];

    int i;
    bool error_occurred = false;

    for (i = 0; i < (2 * HEAP_SIZE_IN_MEGA_BYTES * 256) + 1; i++)
    {
        ptrs[i] = pm_malloc(1000, "ptr", "char");
        if (ptrs[i] == -1)
        {
            printf("TESTING: Out of memory error at allocation %d\n", i);
            error_occurred = true;
            break;
        }
    }

    assert(error_occurred);

    printf("TESTING: pages allocated so far: %d\n", i);

    for (int j = 0; j < i; j++)
    {
        pm_free(ptrs[j]);
    }

    printf("TESTING: mallocing a page after entire virtual page has been emptied:\n");
    int r = pm_malloc(1000, "ptr", "char");

    assert(r == 0);
    printf("\nTest Complete: Out of virtual pages complete.\n\n");
}

void *thread_routine(void *arg)
{
    // Allocate memory
    int id = pm_malloc(sizeof(int), "data", "int");

    // Access the data
    pm_access(id);

    // Free the memory
    pm_free(id);

    return NULL;
}

void test_request_greater_than_page_size()
{
    int p = pm_malloc(10000, "ptr", "char");

    assert(p == -1);

    printf("Test Complete: Memory request greater than page size successful.\n");
}

void test_thread_safety()
{
    const int NUM_THREADS = 8;
    pthread_t threads[NUM_THREADS];
    int thread_args[NUM_THREADS];

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_args[i] = i;
        pthread_create(&threads[i], NULL, thread_routine, &thread_args[i]);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    printf("\n\nTests Passed: Thread safety test passed.\n\n");
}

int main(int argc, char **argv)
{
    // tests for different possible scenarios
    pm_init();
    test_successive_allocation();

    pm_init();
    test_write_read();

    pm_init();
    test_access_invalid();

    pm_init();
    test_thread_safety();

    pm_init();
    test_request_greater_than_page_size();

    pm_init();
    test_out_of_memory_pages();

    pm_files_cleanup();

    return 0;
}
