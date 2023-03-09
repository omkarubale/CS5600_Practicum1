/*
 * test.c / Add Thread Safety
 *
 * Omkar Ubale / CS5600 / Northeastern University
 * Spring 2023 / Feb 27, 2023
 *
 * For documentation, Doxygen commenting has been used for better readability in
 * IDEs like VS Code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "pm_heap.h"

void *addMemoryAllocation_ShortString()
{
    char *shorterStringType = pm_malloc(100 * sizeof(char));
    printf("Page Number %d allocated to shorterStringType\n\n", (pm_getPageNumber(shorterStringType)));

    return NULL;
}

void *addMemoryAllocation_MediumString()
{
    char *mediumStringType = pm_malloc(5000 * sizeof(char));
    printf("Page Number %d allocated to mediumStringType\n\n", (pm_getPageNumber(mediumStringType)));

    return NULL;
}

void *addMemoryAllocation_LongString()
{
    char *longerStringType = pm_malloc(50000 * sizeof(char));
    printf("Page Number %d allocated to longerStringType\n\n", (pm_getPageNumber(longerStringType)));

    return NULL;
}

void *addMemoryAllocation_Int()
{
    char *intType = pm_malloc(sizeof(int));
    printf("Page Number %d allocated to intType\n\n", (pm_getPageNumber((void *)intType)));

    return NULL;
}

int main(int argc, char **argv)
{
    pm_init();

    pthread_t thread_array[40];

    int err = 0;
    for (int j = 0; j < 10; j++)
    {
        err = pthread_create(&(thread_array[4 * j]), NULL, addMemoryAllocation_ShortString, NULL);
        if (err != 0)
        {
            printf("\nERROR: Can't create thread :[%s]", strerror(err));
            exit(1);
        }

        err = pthread_create(&(thread_array[4 * j + 1]), NULL, addMemoryAllocation_MediumString, NULL);
        if (err != 0)
        {
            printf("\nERROR: Can't create thread :[%s]", strerror(err));
            exit(1);
        }

        err = pthread_create(&(thread_array[4 * j + 2]), NULL, addMemoryAllocation_LongString, NULL);
        if (err != 0)
        {
            printf("\nERROR: Can't create thread :[%s]", strerror(err));
            exit(1);
        }

        err = pthread_create(&(thread_array[4 * j + 3]), NULL, addMemoryAllocation_Int, NULL);
        if (err != 0)
        {
            printf("\nERROR: Can't create thread :[%s]", strerror(err));
            exit(1);
        }
    }

    for (int j = 0; j < 40; j++)
    {
        pthread_join(thread_array[j], NULL);
    }

    printf("\nDONE: Demo complete.\n");

    return 0;
}