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
    // Test successive allocation
    pm_malloc(100, "short_string", "char");
    pm_malloc(400, "medium_string", "char");
    pm_malloc(1200, "long_string", "char");
    pm_malloc(sizeof(int), "integer", "int");

    pm_access(2);

    pm_free(0);
    pm_free(1);
    pm_free(2);
    pm_free(3);
}

int main(int argc, char **argv)
{

    // Initialize the heap
    pm_init();

    test_successive_allocation();

    return 0;
}
