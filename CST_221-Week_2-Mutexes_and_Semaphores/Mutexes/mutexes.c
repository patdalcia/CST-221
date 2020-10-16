#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#define MAX 1000

//Declaring mutex
pthread_mutex_t mutex;

//declaring threads
pthread_t thread1, thread2;

// Shared Circular Buffer
struct CIRCULAR_BUFFER
{
    int counter;
};
struct CIRCULAR_BUFFER *buffer = NULL;

enum direction{THREAD1, THREAD2}; 



int iterateCounter(int d){
    pthread_mutex_lock(&mutex);
    if(buffer->counter < MAX){
        buffer->counter = buffer->counter + 1;

        if(d == 0){
            printf("Counter iterated by THREAD1. Counter total: %d\n", buffer->counter);
            fflush(stdout);
        }
        else if(d == 1){
            printf("Counter iterated by THREAD2. Counter total: %d\n", buffer->counter);
            fflush(stdout);
        }
    }
    pthread_mutex_unlock(&mutex);
return 0;
}

void *thread_1(){
    while (1)
    {
        iterateCounter(THREAD1);
    }
    
}

void *thread_2(){
    while (1)
    {
        iterateCounter(THREAD2);
    }
}


/*
* This program uses two threads and a mutex to iterate a shared timer. Each thread is set to 
* loop forever and iterate the counter. An integer value is passed from thread to the iterator function
* indicating which thread the iteration is coming from. The mutex is used to block access to the shared counter 
* while its contents are being manipulated.
*/


int main(int argc, char* argv[])
{
    pthread_mutex_init(&mutex, NULL);

 // Create shared memory for the Circular Buffer to be shared between the Parent and Child  Processes
    buffer = (struct CIRCULAR_BUFFER*)mmap(0,sizeof(buffer), PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    //Initializing threads
    pthread_create(&thread1, NULL, thread_1, NULL);
    pthread_create(&thread2, NULL, thread_2, NULL);

    //Killing threads
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Return OK
return 0;

}
