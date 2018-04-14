#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <zconf.h>

int consumption_count = 15;
int production_count = 15;

int buffer[15];
int buffer_size = 0;

sem_t mutex;
sem_t full;
sem_t empty;


void *produce();

void *consume();

int main() {

//    pthread_t producer_1, producer_2, producer_3,
//              consumer_type_one_1, consumer_type_one_2,
//              consumer_type_two_1, consumer_type_two_2;

    pthread_t producer, consumer;

    sem_init(&mutex, 0, 1);// sem_mutex = 1 at the beginning
    sem_init(&full, 0, 10);// sem_full = 10 at the beginning
    sem_init(&empty, 0, 0);// sem_empty = 0 at the beginning


    pthread_create(&producer, NULL, produce, NULL);
    pthread_create(&consumer, NULL, consume, NULL);

    pthread_exit(NULL);
}


void *produce() {
    srand(time(NULL));

    while (production_count > 0) {
        sem_wait(&full);
        sem_wait(&mutex);
        production_count--;
        printf("production start\n");


        buffer[buffer_size] = rand() % 100;
        printf("-produced value: %d, index: %d \n", buffer[buffer_size], buffer_size);
        buffer_size++;


        printf("production end\n\n");
        sem_post(&empty);
        sem_post(&mutex);
//        sleep(1);
    }

    pthread_exit(NULL);
}

void *consume() {
    int consumed_value;

    while (consumption_count > 0) {
        sem_wait(&empty);
        sem_wait(&mutex);
        consumption_count--;
        printf("consumption start\n");


        buffer_size--;
        consumed_value = buffer[buffer_size];
        printf("*consumed value: %d, index: %d \n", consumed_value, buffer_size );


        printf("consumption end\n\n");
        sem_post(&full);
        sem_post(&mutex);
        sleep(1);
    }

    pthread_exit(NULL);
}
