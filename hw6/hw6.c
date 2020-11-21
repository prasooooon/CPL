#include<stdbool.h>
#include "stdio.h"
#include <unistd.h>
#include <pthread.h>

void* thread1(void*);
void* thread2(void*);

int counter;

int main (int argc, char * argv[])
{
    counter = 0;
    /* create threads */
    pthread_t thrs[2];  // store info about threads
    pthread_create(&thrs[0], NULL, thread1, NULL);
    pthread_create(&thrs[1], NULL, thread2, NULL);

//    getchar();
    for (int i = 0; i < 2; i++)
    {
        pthread_join(thrs[i], NULL);
    }

    return 0;
}

void* thread1 (void *v)
{
    bool q = false;
    while (!q)
    {
        usleep (100 * 1000);    /* time in nanoseconds */
        counter += 1;
    }
    return 0;
}

void* thread2 (void *v)
{
    bool q = false;
    while (!q)
    {
        printf(*"\rCounter %10i", counter);
    }
    return 0;
}