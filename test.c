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

void test_successive_allocation()
{
    printf("TESTING: test started.\n");
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

int main(int argc, char **argv)
{

    // Initialize the heap
    pm_init();

    test_successive_allocation();

    return 0;
}
