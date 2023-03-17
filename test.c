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

int main(int argc, char **argv)
{

    // Initialize the heap
    pm_init();

    test_successive_allocation();

    return 0;
}
