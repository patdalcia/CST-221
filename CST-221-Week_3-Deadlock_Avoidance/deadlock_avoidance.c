#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

//Defining max index of buffer array
#define MAX 100;

// Shared Circular Buffer
typedef struct 
{
    int buf;
    int bufferCount;
    pid_t pids[2];
} CIRCULAR_BUFFER_t;
CIRCULAR_BUFFER_t *buffer = NULL;

#define PRODUCER 0
#define CONSUMER 1

// The Child PID if the Parent else the Parent PID if the Child
pid_t otherPid;

//Declaring mutex
pthread_mutex_t myMutex;

//Declaring file pointer
FILE *file;

void *producer(){
    int c = 0;
    for(int i = 0; i < 10; i++){
        while(!pthread_mutex_trylock(&myMutex))
        {
                //If loop is entered, mutex is locked and resource is unavailable for current producer process
                printf("\nProcess %d has been starved.....Sleeping for two seconds\n", buffer->pids[PRODUCER]);
                fprintf(file, "\nLOG: Process %d has been starved.....Sleeping for two seconds\n", buffer->pids[PRODUCER]);
                sleep(2);
                printf("Process %d has been awoken from sleep\n\n", buffer->pids[PRODUCER]);
                fprintf(file, "LOG: Process %d has been awoken from sleep\n\n", buffer->pids[PRODUCER]);
        }
    
        printf("Process %d is acessing the shared buffer resource\n", buffer->pids[PRODUCER]);
        fprintf(file, "LOG: Process %d is acessing the shared buffer resource\n", buffer->pids[PRODUCER]);

        buffer->buf += 1;

        printf("Producer wrote %d to shared buffer memory\n", buffer->buf);
        fprintf(file, "LOG: Producer wrote %d to shared buffer memory\n", buffer->buf);

        pthread_mutex_unlock(&myMutex); //Unlocking mutex
    }
}

void *consumer(){
    int c = 0;
   for(int i = 0; i < 10; i++){
       while(!pthread_mutex_trylock(&myMutex)){
            //If loop is entered, mutex is locked and resource is unavailable for current Consumer process
            printf("\nProcess %d has been starved.....Sleeping for two seconds\n", buffer->pids[CONSUMER]);
            fprintf(file, "\nLOG: Process %d has been starved.....Sleeping for two seconds\n", buffer->pids[CONSUMER]);

            sleep(2);

            printf("Process %d has been awoken from sleep\n\n", buffer->pids[CONSUMER]);
            fprintf(file, "LOG: Process %d has been awoken from sleep\n\n", buffer->pids[CONSUMER]);
       }

        printf("Process %d is acessing the shared buffer resource\n", buffer->pids[CONSUMER]);
        fprintf(file, "LOG: Process %d is acessing the shared buffer resource\n", buffer->pids[CONSUMER]);

        buffer->buf += 1;

        printf("Consumer wrote %d to shared buffer memory\n", buffer->buf);
        fprintf(file, "LOG: Consumer wrote %d to shared buffer memory\n", buffer->buf);

        pthread_mutex_unlock(&myMutex); //Unlocking mutex
   }
}

int main(int argc, char* argv[])
{
    pid_t  pid;

 // Create shared memory for the Circular Buffer to be shared between the Parent and Child  Processes
    buffer = (struct CIRCULAR_BUFFER*)mmap(0,sizeof(buffer), PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    pthread_mutex_init(&myMutex, NULL);

    file = fopen("output.txt", "a");

    pid = fork();
    if (pid == -1)
    {
        // Error: If fork() returns -1 then an error happened (for example, number of processes reached the limit).
        printf("Can't fork, error %d\n", errno);
        fprintf(file, "LOG: Can't fork, error %d\n", errno);

        exit(EXIT_FAILURE);
    }
    // OK: If fork() returns non zero then the parent process is running else child process is running
    if (pid == 0)
    {
        // Run Consumer Process logic as a Child Process
        otherPid = getppid();
        buffer->pids[CONSUMER] = getpid();
        printf("\nconsumer pid = %d\n", buffer->pids[CONSUMER]);
        fprintf(file, "\nLOG: consumer pid = %d\n", buffer->pids[CONSUMER]);
        consumer();
    }
    else
    {
        // Run Producer Process logic as a Parent Process
        pthread_t tid;
        otherPid = pid;
        buffer->pids[PRODUCER] = getpid();

        printf("\nproducer pid = %d\n", buffer->pids[PRODUCER]);
        fprintf(file, "\nLOG: producer pid = %d\n", buffer->pids[PRODUCER]);

        producer();
    }

    sleep(2);

    //Destroying mutex and stopping processes
    pthread_mutex_destroy(&myMutex);

    //Closing file
    fclose(file);

    //killing the processes 
    kill(buffer->pids[PRODUCER], SIGTERM);
    kill(buffer->pids[CONSUMER], SIGTERM);
    
    // Return OK
return 0;
}