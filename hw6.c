/* compile with clang file.c  -lpthread */

#include<stdbool.h>
#include "stdio.h"
#include <unistd.h>
#include <pthread.h>

void* thread1(void*);
void* thread2(void*);

int counter;

pthread_mutex_t mtx;
pthread_cond_t condvar;

bool quit = false;

typedef struct tThrData
{
    char * strName;
    int iId;

} tThrData;

int main (int argc, char * argv[])
{
    counter = 0;

    /* create threads */
    pthread_t thrs[2];  // store info about threads

    tThrData oThr[2];
    oThr[0].strName = "Counter";
    oThr[0].iId = -1;
    oThr[1].strName = "Printer";
    oThr[1].iId = -2;

    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&condvar, NULL);

    pthread_create(&thrs[0], NULL, thread1, (void *)&oThr[0]);
    pthread_create(&thrs[1], NULL, thread2, (void *)&oThr[1]);

    getchar();

    pthread_mutex_lock(&mtx);   /* protects some shared section of the memory for isolation */
    quit = true;
    pthread_mutex_unlock(&mtx);
    for (int i = 0; i < 2; i++)
    {
        pthread_join(thrs[i], NULL);
    }

    return 0;
}

void* thread1 (void *v)
{
    bool q = false;

    tThrData * pTD;
    pTD = (tThrData *)v;
    printf("Thread %s is on (%d)\n", pTD->strName, pTD->iId);

    while (!q)
    {
        pthread_mutex_lock(&mtx);
            usleep (1000 * 1000);
            counter += 1;
            q = quit;
            pthread_cond_signal(&condvar);  /* send information about finished operation to the other thread */
        pthread_mutex_unlock(&mtx);

        usleep(10);
    }
    return 0;
}

void* thread2 (void *v)
{
    bool q = false;

    tThrData * pTD;
    pTD = (tThrData *)v;
    printf("Thread %s is on (%d)\n", pTD->strName, pTD->iId);

    while (!q)
    {
        pthread_mutex_lock(&mtx);
            pthread_cond_wait(&condvar, &mtx);
            printf("\rCounter %10i", counter);
            fflush(stdout);
            q = quit;
        pthread_mutex_unlock(&mtx);
    }
    return 0;
}