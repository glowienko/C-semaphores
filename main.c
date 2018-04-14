#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <zconf.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define BLU   "\x1B[34m"
#define RESET "\x1B[0m"

//int consumption_count = 100;
//int production_count = 100;
int work_counter = 50;

int buffer_1[10];
int buffer_size_1 = 0;

int buffer_2[10];
int buffer_size_2 = 0;

sem_t mutex_1;
sem_t full_1;
sem_t empty_1;

sem_t mutex_2;
sem_t full_2;
sem_t empty_2;


void *produce(void *id);

void *consume(void *id);

void *consume_from_two_buffers(void *id);

void random_sleep();

int draw_buffer();

int main() {

//    pthread_t producer_1, producer_2, producer_3,
//              consumer_type_one_1, consumer_type_one_2,
//              consumer_type_two_1, consumer_type_two_2;

    pthread_t producer_1, producer_2, producer_3, consumer, two_consumer;
    int one = 1, two = 2, three = 3;

    sem_init(&mutex_1, 0, 1);// sem_mutex = 1 at the beginning
    sem_init(&mutex_2, 0, 1);// sem_mutex = 1 at the beginning

    sem_init(&full_1, 0, 10);// sem_full = 10 at the beginning
    sem_init(&full_2, 0, 10);// sem_full = 10 at the beginning

    sem_init(&empty_1, 0, 0);// sem_empty = 0 at the beginning
    sem_init(&empty_2, 0, 0);// sem_empty = 0 at the beginning
    srand(time(NULL));


    pthread_create(&producer_1, NULL, produce, (void *) &one);
    pthread_create(&producer_2, NULL, produce, (void *) &two);
//    pthread_create(&producer_3, NULL, produce, (void *) &three);

    pthread_create(&consumer, NULL, consume, (void *) &one);
    pthread_create(&two_consumer, NULL, consume_from_two_buffers, (void *) &one);

    pthread_exit(NULL);
}


void *produce(void *id) {
    int ID = *((int *) id);

    while (work_counter > 0) {

        //randomly select buffer
        if (draw_buffer() == 1) { // produce to buffer 1
            sem_wait(&full_1);
            sem_wait(&mutex_1);

            buffer_1[buffer_size_1] = rand() % 10;
            printf("-producer: %d  produced: %d for "GRN"BUFF 1 "RESET", index: %d \n", ID,
                   buffer_1[buffer_size_1], buffer_size_1);
            buffer_size_1++;

            sem_post(&empty_1);
            sem_post(&mutex_1);
        } else { // produce to buffer 2
            sem_wait(&full_2);
            sem_wait(&mutex_2);

            buffer_2[buffer_size_2] = rand() % 10;
            printf("-producer: %d  produced: %d for "BLU"BUFF 2 "RESET", index: %d \n", ID,
                   buffer_2[buffer_size_2], buffer_size_2);
            buffer_size_2++;

            sem_post(&empty_2);
            sem_post(&mutex_2);
        }

        work_counter--;
        random_sleep();
    }

    pthread_exit(NULL);
}

void *consume(void *id) {
    int ID = *((int *) id);

    while (work_counter > 0) {

        //randomly select buffer
        if (draw_buffer() == 1) { //consume from buffer 1
            sem_wait(&empty_1);
            sem_wait(&mutex_1);

            buffer_size_1--;
            printf("*"RED"consumer: %d  consumed: %d "RESET" from "GRN"BUFF 1 "RESET", index: %d \n", ID, buffer_1[buffer_size_1], buffer_size_1);

            sem_post(&full_1);
            sem_post(&mutex_1);

        } else { //consume from buffer 2
            sem_wait(&empty_2);
            sem_wait(&mutex_2);

            buffer_size_2--;
            printf("*"RED"consumer: %d  consumed: %d "RESET" from "BLU"BUFF 2 "RESET", index: %d \n", ID, buffer_2[buffer_size_2], buffer_size_2);

            sem_post(&full_2);
            sem_post(&mutex_2);
        }

        work_counter--;
        random_sleep();
    }

    pthread_exit(NULL);
}

void *consume_from_two_buffers(void *id) {
    int ID = *((int *) id);

    while (work_counter > 0) {
        sem_wait(&empty_1);
        sem_wait(&empty_2);
        sem_wait(&mutex_1);
        sem_wait(&mutex_2);

        buffer_size_1--;
        buffer_size_2--;
        printf("*"RED"consumer_type_two: %d  consumed: %d,%d"RESET" from "GRN"BUFF 1 & 2 "RESET", index1: %d, index2: %d \n",
               ID, buffer_1[buffer_size_1], buffer_2[buffer_size_2], buffer_size_1, buffer_size_2);

        sem_post(&full_1);
        sem_post(&full_2);
        sem_post(&mutex_1);
        sem_post(&mutex_2);

        work_counter--;
        random_sleep();
        sleep(2);

    }

    pthread_exit(NULL);
}

int draw_buffer() {
    return rand() % 2 + 1;
}

void random_sleep() {
    if (rand() % 2 == 1) {
        sleep(1);
    }
}
