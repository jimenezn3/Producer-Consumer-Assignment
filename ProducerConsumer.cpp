// Nick Diaz

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFF_SIZE   5		/* total number of slots */
#define NP          3		/* total number of producers */
#define NC          3		/* total number of consumers */
#define NITERS      4		/* number of items produced/consumed */

typedef struct {
    int buf[BUFF_SIZE];   /* shared var */
    int in;         	  /* buf[in%BUFF_SIZE] is the first empty slot */
    int out;        	  /* buf[out%BUFF_SIZE] is the first full slot */
    sem_t full;     	  /* keep track of the number of full spots */
    sem_t empty;    	  /* keep track of the number of empty spots */
    pthread_mutex_t mutex;    	  /* enforce mutual exclusion to shared data */
} sbuf_t;

sbuf_t shared;

void *Producer(void *arg)
{
    int i, item, index;
    index = *(int*)arg;

    for (i=0; i < NITERS; i++) {
        //produce an item
        item = i;

        //check for empty slots, if none then wait
        sem_wait(&shared.empty);

        //lock and write to empty slot
        pthread_mutex_lock(&shared.mutex);
        shared.buf[shared.in] = item;
        //move to next empty slot
        shared.in = (shared.in + 1)% BUFF_SIZE;
        printf("Producer %d producing %d \n", index, item);
        fflush(stdout);

        //release buffer
        pthread_mutex_unlock(&shared.mutex);

        //increment # of full slots
        sem_post(&shared.full);

        /* Interleave  producer and consumer execution */
        if (i % 2 == 1) sleep(1);
    }
    return NULL;
}

void *Consumer(void *arg)
{
    int i, item, index;
    index = *(int*)arg;

    for(i = NITERS; i > 0; i--){
        //check for an occupied slot
        sem_wait(&shared.full);

        //read from the buffer
        pthread_mutex_lock(&shared.mutex);
        item = i;
        item = shared.buf[shared.out];
        //move to next full slot
        shared.out = (shared.out + 1) % BUFF_SIZE;
        printf("Consumer %d consuming %d \n", index, item);
        fflush(stdout);

        //release buffer
        pthread_mutex_unlock(&shared.mutex);

        //increment # of empty slots
        sem_post(&shared.empty);

        //interleave producer and consumer execution
        if(i % 2 == 1) sleep(1);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t idP, idC;
    int index;

    //initialize semaphores
    sem_init(&shared.full, 0, 0);
    sem_init(&shared.empty, 0, BUFF_SIZE);
    pthread_mutex_init(&shared.mutex, NULL);

    for (index = 0; index < NP; index++)
    {  
       /* Create a new producer */
       pthread_create(&idP, NULL, Producer, (void*)&index);
    }
    //create new consumer
    for(index = 0; index < NC; index++){
        pthread_create(&idC, NULL, Consumer, (void*)&index);
    }

    pthread_exit(NULL);
}

/*
Important note: You must link the pthread library to any program that uses pthreads. You must also link the runtime library rt to any program that uses semaphores. So the compile command for the program above (call it prodcon.c) would be
    gcc -o prodcon prodcon.c -lpthread -lrt
*/
