#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

//Defining max index of buffer array
#define MAX 100;

// Shared Circular Buffer
struct CIRCULAR_BUFFER
{
    int buffer[100];
};
struct CIRCULAR_BUFFER *buffer = NULL;

//declaring Sephamores
sem_t empty;
sem_t filled;
sem_t lock;


//declaring threads
pthread_t thread1, thread2;



//Adds value to buffer
void putValue(int value){
    int index = 0;
    sem_getvalue(&filled, &index);
    buffer->buffer[index] = value;
}

//Gets value from buffer
int getValue(){
    int index = 0;
    sem_getvalue(&filled, &index);      //getting current upper index of array
    int value = buffer->buffer[0];      //Taking first value
    for(int i = 0; i < index; i++){
        buffer->buffer[i] = buffer->buffer[i + 1];  //Shifting array down
    }

    return value;
}

//Creates a new item to add to buffer
int create_item(int item){
    return (item = item + 1);
}

//prints value to console
void consume(int item){
    printf("\nValue: %d\n", item);
}

void printBufferCount(){
    int count = 0;
    sem_getvalue(&filled, &count);
    printf("Buffer Count: %d\n", count);
}

void *producer(){

    int item = 0;
    int count = 0;
    while (count < 100)             /*Loops 100 times. I added this loop to prevent infinite 
                                    looping. Changing 'count < 100' to ' 1' will loop forever*/
    {
        item = create_item(item);   //getting item to be added
        sem_wait(&empty);           //Decrement empty space count
        sem_wait(&lock);            //lock critical space
        putValue(item);             //Add item to buffer
        sem_post(&lock);            //unlock critical space
        sem_post(&filled);          //incrementing filled count
        count++;
    }
    
}

void *consumer(){
    int item;
    int count = 0;
    while (count < 100)             /*Loops 100 times. I added this loop to prevent infinite 
                                    looping. Changing 'count < 100' to ' 1' will loop forever*/
    {
        sem_wait(&filled);          //decrement filled count
        sem_wait(&lock);            //locking critical space
        item = getValue();          //Getting value from buffer
        sem_post(&lock);            //unlocking critical space
        sem_post(&empty);           //Incrementing empty space count
        consume(item);              //print item
        printBufferCount();         //print current number of items in buffer
        count++;
    }
    
}

int main(int argc, char* argv[])
{
    //iniitlaizing semephores
    sem_init(&lock, 0, 1);
    sem_init(&filled, 0, 0);
    sem_init(&empty, 0, 100);

 // Create shared memory for the Circular Buffer to be shared between the Parent and Child  Processes
    buffer = (struct CIRCULAR_BUFFER*)mmap(0,sizeof(buffer), PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    //Initializing threads
    pthread_create(&thread1, NULL, producer, NULL);
    pthread_create(&thread2, NULL, consumer, NULL);

    //Killing threads
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    //Destroying sephamores
    sem_destroy(&lock);
    sem_destroy(&filled);
    sem_destroy(&empty);

    // Return OK
return 0;
}