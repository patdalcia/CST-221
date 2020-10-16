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

    printf("Sleeping...........\n");
    // TODO: Sleep until notified to wake up using the sigwait() method
    FLAG = sigwait(&sigSet, &nSig);
    printf("Awoken\n");
    fflush(stdout);
}

// This method will signal the Other Process to WAKEUP
void wakeupOther()
{
    // TODO: Signal Other Process to wakeup using the kill() method
    printf("Inside wakeupOther()\n");
    
        kill(otherPid, WAKEUP);
}

// Gets a value from the shared buffer
int getValue()
{
    printf("Inside getValue()\n");
    // TODO: Get a value from the Circular Buffer and adjust where to read from next
    int value = 0;      
    value = buffer->buffer[0];  //Getting value from buffer

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
    printf("Inside putValue()\n");
    // TODO: Write to the next available position in the Circular Buffer and adjust where to write next
    buffer->buffer[buffer->upper] = value;
    buffer->upper++;
    
}

void consume(int value){
    printf("Inside consume()\n");
    printf("Value: %d\n", value);
    fflush(stdout);
}

int getItem(int item){
    item = item + 1;
    return item;
}

/****************************************************************************************************/

/**
 * Logic to run to the Consumer Process
 **/
void consumer()
{
    // Set Signal Set to watch for WAKEUP signal
    sigemptyset(&sigSet);
    sigaddset(&sigSet, WAKEUP);

    // Run the Consumer Logic
    printf("Running Consumer Process.....\n");

    // TODO: Implement Consumer Logic (see page 129 in book)
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
void producer()
{
    // Buffer value to write
    int item = 0;

    // Set Signal Set to watch for WAKEUP signal
    sigemptyset(&sigSet);
    sigaddset(&sigSet, WAKEUP);

    // Run the Producer Logic
    printf("Running Producer Process.....\n");

    // TODO: Implement Producer Logic (see page 129 in book)
    while (1) {                                         /*repeat forever*/
    item = getItem(item);                                   /*generate next item*/
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
    pid_t  pid;

    // Create shared memory for the Circular Buffer to be shared between the Parent and Child  Processes
    buffer = (struct CIRCULAR_BUFFER*)mmap(0,sizeof(buffer), PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    buffer->count = 0;
    buffer->lower = 0;
    buffer->upper = 0;

    // Use fork()
    pid = fork();
    if (pid == -1)
    {
        // Error: If fork() returns -1 then an error happened (for example, number of processes reached the limit).
        printf("Can't fork, error %d\n", errno);
        exit(EXIT_FAILURE);
    }
    // OK: If fork() returns non zero then the parent process is running else child process is running
    if (pid == 0)
    {
        // Run Producer Process logic as a Child Process
        otherPid = getppid();
        producer();
    }
    else
    {
        // Run Consumer Process logic as a Parent Process
        otherPid = pid;
        consumer();
    }

    // Return OK
    return 0;
}
