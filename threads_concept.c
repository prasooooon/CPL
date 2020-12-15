#include <stdio.h>
#include <pthread.h>

#define NUM_LOOPS 20000000
long long sum = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // generate mutex to work with

void* counting_thread (void *arg)
{
    int offset = *(int *) arg;   // takes arg, makes it into pointer to an int, then dereference it to put the value in 'offset'
    for (int i = 0; i < NUM_LOOPS; i++)
    {
        // a critical section happens when you have shared data between multiple process or threads
        // start critical section to protect access to 'sum'

        pthread_mutex_lock(&mutex);

        // only one thread can access or execute this line at a time
        sum += offset;

        pthread_mutex_unlock(&mutex);

        // end critical section

    }

    pthread_exit(NULL);     // what you pass in here, is what's going to be returned by thread function
}

int main(void)
{
    // Spawn threads

    int offset1 = 1;
    pthread_t id1;
    pthread_create(&id1, NULL, counting_thread, &offset1);

    int offset2 = -1;
    pthread_t id2;
    pthread_create(&id2, NULL, counting_thread, &offset2);

    // Wait for threads to finish
    pthread_join(id1, NULL);   // NULL is thread return
    pthread_join(id2, NULL);   // NULL is thread return

    printf("Sum = %lld\n", sum);

    return 0;
}