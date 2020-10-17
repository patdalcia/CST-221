/*
* This program was created by Patrick Garcia for CST-221 on 10/10/2020
* This program utilizes the sigwait() and pThread_kill() to 
* provide syncronization across threads.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include  <signal.h>
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>


// Constants
int MAX = 100;
int WAKEUP = SIGUSR1;
int SLEEP = SIGUSR2;

int FLAG = 0;

// The Child PID if the Parent else the Parent PID if the Child
pid_t otherPid;

// A Signal Set
sigset_t sigSet;

//Thread declarations
static pthread_t producer_thread, consumer_thread;

// Shared Circular Buffer
struct CIRCULAR_BUFFER
{
    int count;          // Number of items in the buffer
    int lower;          // Next slot to read in the buffer
    int upper;          // Next slot to write in the buffer
    int buffer[100];
};
struct CIRCULAR_BUFFER *buffer = NULL;

/****************************************************************************************************/

// This method will put the current Process to sleep forever until it is awoken by the WAKEUP signal
void sleepAndWait()
{
    int nSig;
    if(pthread_self() == producer_thread){ //Producer is attempting to wake consumer
        printf("Putting Producer thread to sleep.......\n");
    }
    else    //Consumer is attempting to wake producer
    {
        printf("Putting Consumer thread to sleep.......\n");
    }
    // TODO: Sleep until notified to wake up using the sigwait() method
    sigwait(&sigSet, &nSig);
}

// This method will signal the Other thread to WAKEUP
void wakeupOther()
{
    if(pthread_self() == producer_thread){ //Producer is attempting to wake consumer
        printf("Waking up the Consumer thread\n");
        pthread_kill(consumer_thread, SIGUSR1);
    }
    else    //Consumer is attempting to wake producer
    {
        printf("Waking up the Producer thread\n");
        pthread_kill(producer_thread, SIGUSR1);
    }       
}

// Gets a value from the shared buffer
int getValue()
{
    int value = 0;      
    value = buffer->buffer[0];  //Getting value from buffer

    //Shifting buffer contents down one index, effectively
    //removing retrived value
    for(int i = 0; i < buffer->upper; i++){
        buffer->buffer[i] = buffer->buffer[i + 1]; 
    }

    buffer->upper = buffer->upper - 1;      //Updating upper index of buffer
    buffer->count = buffer->count - 1;      //decrement count of items in buffer
    return value;                           //Returning the value
}

// Puts a value in the shared buffer
void putValue(struct CIRCULAR_BUFFER* buffer, int value)
{
    // Write to the next available position in the Circular Buffer and adjust where to write next
    buffer->buffer[buffer->upper] = value;
    buffer->upper++;
    
}

//Print the value of buffer to screen
void consume(int value){
    printf("Value: %d\n", value);
    fflush(stdout);
}

//Generates a new item to add to buffer
int getItem(int item){
    item = item + 1;
    return item;
}

/****************************************************************************************************/

/**
 * Logic to run to the Consumer Process
 **/
void *consumer()
{
    // Run the Consumer Logic
    printf("Running Consumer Process.....\n");

    // Implement Consumer Logic
    int value = 0;
    while(1){                                       /*loop forever*/
        if(buffer->count == 0) sleepAndWait();      /*If empty sleep*/
        value = getValue();                         /*Take item out of buffer*/
        if(buffer->count == MAX - 1) wakeupOther(); /*Was buffer full?*/
        consume(value);                             /*Print item*/
    }

    // Exit cleanly from the Consumer Process
    _exit(1);
}

/**
 * Logic to run to the Producer Process
 **/
void *producer()
{
    // Buffer value to write
    int item = 0;

    // Run the Producer Logic
    printf("Running Producer Process.....\n");

    // Implement Producer Logic
    while (1) {                                         /*repeat forever*/
    item = getItem(item);                               /*generate next item*/
    if (buffer->count == MAX) sleepAndWait();           /*if buffer is full, go to sleep*/
    putValue(buffer, item);                             /*put item in buffer*/
    buffer->count = buffer->count + 1;                  /*increment count of items in buffer*/
    if (buffer->count == 1) wakeupOther();              /*wasbuffer empty?*/
    }

    // Exit cleanly from the Consumer Process
    _exit(1);
}

/**
 * Main application entry point to demonstrate forking off a child process that will run concurrently with this process.
 *
 * @return 1 if error or 0 if OK returned to code the caller.
 */
int main(int argc, char* argv[])
{
    //Adding wakeup signal to signal set
    sigemptyset(&sigSet);
    sigaddset(&sigSet, SIGUSR1);
    sigaddset(&sigSet, SIGSEGV);
    pthread_sigmask(SIG_BLOCK, &sigSet, NULL);

    // Create shared memory for the Circular Buffer to be shared between the Parent and Child  Processes
    buffer = (struct CIRCULAR_BUFFER*)mmap(0,sizeof(buffer), PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    buffer->count = 0;
    buffer->lower = 0;
    buffer->upper = 0;

    //Creating threads
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    //destroying threads
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread,NULL);

    // Return OK
    return 0;
}
