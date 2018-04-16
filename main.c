#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <zconf.h>
#include <stdbool.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define BLU   "\x1B[34m"
#define RESET "\x1B[0m"

int work_counter = 300;

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

sem_t queue_type1_buff_1;
sem_t queue_type1_buff_2;
sem_t queue_type2;
sem_t action_token;

sem_t notify_queue;

void *produce(void *id);

void *consume(void *id);

void *consume_from_two_buffers(void *id);

void *check_queue();

void random_sleep();

int draw_buffer();

void init_semaphores();

bool is_type_two_queued(int queue_type2_sem_value);

bool can_take_from_buff_1_queue(int queue_type1_buf1_sem_value);

bool can_take_from_buff_2_queue(int queue_type1_buf2_sem_value);

int main() {
    pthread_t producer_1, producer_2, producer_3, consumer, consumer_2, two_consumer, two_consumer_2, queue;
    int one = 1, two = 2, three = 3;

    srand(time(NULL));
    init_semaphores();
    printf("\n\n\n");

    pthread_create(&queue, NULL, check_queue, NULL);

    pthread_create(&producer_1, NULL, produce, (void *) &one);
    pthread_create(&producer_2, NULL, produce, (void *) &two);
    pthread_create(&producer_3, NULL, produce, (void *) &three);

    pthread_create(&consumer, NULL, consume, (void *) &one);
    pthread_create(&consumer_2, NULL, consume, (void *) &two);
    pthread_create(&two_consumer, NULL, consume_from_two_buffers, (void *) &one);
    pthread_create(&two_consumer_2, NULL, consume_from_two_buffers, (void *) &two);

    pthread_exit(NULL);
}

void init_semaphores() {
    sem_init(&mutex_1, 0, 1);// sem_mutex = 1 at the beginning
    sem_init(&mutex_2, 0, 1);

    sem_init(&full_1, 0, 10);// sem_full = 10 at the beginning
    sem_init(&full_2, 0, 10);

    sem_init(&empty_1, 0, 0);// sem_empty = 0 at the beginning
    sem_init(&empty_2, 0, 0);

    sem_init(&queue_type1_buff_1, 0, 0);// nothing in queues at start, sem = 0 - cannot take from queue
    sem_init(&queue_type1_buff_2, 0, 0);
    sem_init(&queue_type2, 0, 0);

    sem_init(&notify_queue, 0, 0);
    sem_init(&action_token, 0, 1);
}


// =====================================   QUEUE  =====================================
void *check_queue() {
    int queue_type1_buf1_sem_value, queue_type1_buf2_sem_value, queue_type2_sem_value;

    while (work_counter > 0) {
        sem_wait(&notify_queue);
        sem_wait(&mutex_1);
        sem_wait(&mutex_2);

        sem_getvalue(&queue_type1_buff_1, &queue_type1_buf1_sem_value);
        sem_getvalue(&queue_type1_buff_2, &queue_type1_buf2_sem_value);
        sem_getvalue(&queue_type2, &queue_type2_sem_value);


//        printf("___ queues: type 2:  "RED"%d"RESET", type_1__1: "RED"%d"RESET", type_1__2: "RED"%d"RESET" "
//                       "buffers: buff_1: "RED"%d"RESET", buff_2: "RED"%d"RESET"\n",
//               queue_type2_sem_value, queue_type1_buf1_sem_value, queue_type1_buf2_sem_value, buffer_size_1,
//               buffer_size_2);
        if (is_type_two_queued(queue_type2_sem_value)) {
            buffer_size_2--;
            buffer_size_1--;
            printf("*"RED"consumer TYPE 2 FROM QUEUE consumed: %d and %d "RESET" from BUFF 1,2; index: %d and %d \n",
                   buffer_1[buffer_size_1],
                   buffer_2[buffer_size_2], buffer_size_1, buffer_size_2);

            sem_post(&full_1);
            sem_post(&full_2);
            sem_wait(&queue_type2);
        }

        if (!is_type_two_queued(queue_type2_sem_value) && can_take_from_buff_1_queue(queue_type1_buf1_sem_value)) {
            buffer_size_1--;
            printf("-"RED"consumer FROM QUEUE consumed: %d  "RESET" from "GRN"BUFF 1 "RESET", index: %d \n",
                   buffer_1[buffer_size_1], buffer_size_1);

            sem_post(&full_1);
            sem_wait(&queue_type1_buff_1);
        }

        if (!is_type_two_queued(queue_type2_sem_value) && can_take_from_buff_2_queue(queue_type1_buf2_sem_value)) {
            buffer_size_2--;
            printf("-"RED"consumer FROM QUEUE consumed: %d  "RESET" from "BLU"BUFF 2 "RESET", index: %d \n",
                   buffer_2[buffer_size_2], buffer_size_2);

            sem_post(&full_2);
            sem_wait(&queue_type1_buff_2);
        }

        sem_post(&mutex_1);
        sem_post(&mutex_2);
        sem_post(&action_token);
    }

    pthread_exit(NULL);
}

bool is_type_two_queued(int queue_type2_sem_value) {
    return queue_type2_sem_value > 0 && buffer_size_1 > 0 && buffer_size_2 > 0;
}

bool can_take_from_buff_1_queue(int queue_type1_buf1_sem_value) {
    return queue_type1_buf1_sem_value > 0 && buffer_size_1 > 0;
}

bool can_take_from_buff_2_queue(int queue_type1_buf2_sem_value) {
    return queue_type1_buf2_sem_value > 0 && buffer_size_2 > 0;
}

// ===================================== PRODUCER ======================================
void *produce(void *id) {
    int ID = *((int *) id);

    while (work_counter > 0) {
        if (draw_buffer() == 1) {
            sem_wait(&full_1);
            sem_wait(&action_token);
            sem_wait(&mutex_1);

            buffer_1[buffer_size_1] = rand() % 10;
            printf("+producer: %d  produced: %d for "GRN"BUFF 1 "RESET", index: %d \n", ID,
                   buffer_1[buffer_size_1], buffer_size_1);
            buffer_size_1++;

            sem_post(&empty_1);
            sem_post(&mutex_1);
        } else {
            sem_wait(&full_2);
            sem_wait(&action_token);
            sem_wait(&mutex_2);

            buffer_2[buffer_size_2] = rand() % 10;
            printf("+producer: %d  produced: %d for "BLU"BUFF 2 "RESET", index: %d \n", ID,
                   buffer_2[buffer_size_2], buffer_size_2);
            buffer_size_2++;

            sem_post(&empty_2);
            sem_post(&mutex_2);
        }

        sem_post(&notify_queue);
        work_counter--;
        random_sleep();
    }

    pthread_exit(NULL);
}


// ======================================== CONSUMER ===============================================================
void *consume(void *id) {
    int ID = *((int *) id);

    while (work_counter > 0) {

        sem_wait(&action_token);
        if (draw_buffer() == 1) {
            sem_wait(&mutex_1);

            if (buffer_size_1 == 0) {
                sem_post(&queue_type1_buff_1);
                printf("-"RED"consumer: %d ADDED TO QUEUE "RESET" of  "GRN"BUFF 1 "RESET" \n", ID);
            } else {
                buffer_size_1--;
                printf("-"RED"consumer: %d  consumed: %d "RESET" from "GRN"BUFF 1 "RESET", index: %d \n", ID,
                       buffer_1[buffer_size_1], buffer_size_1);

                sem_post(&full_1);
            }
            sem_post(&mutex_1);

        } else {
            sem_wait(&mutex_2);

            if (buffer_size_2 == 0) {
                sem_post(&queue_type1_buff_2);
                printf("-"RED"consumer: %d ADDED TO QUEUE "RESET" of  "BLU"BUFF 2 "RESET" \n", ID);
            } else {
                buffer_size_2--;
                printf("-"RED"consumer: %d  consumed: %d "RESET" from "BLU"BUFF 2 "RESET", index: %d \n", ID,
                       buffer_2[buffer_size_2], buffer_size_2);
                sem_post(&full_2);
            }
            sem_post(&mutex_2);
        }

        sem_post(&notify_queue);
        work_counter--;
        random_sleep();
    }

    pthread_exit(NULL);
}

void *consume_from_two_buffers(void *id) {
    int ID = *((int *) id);

    while (work_counter > 0) {

        sem_wait(&action_token);
        sem_wait(&mutex_1);
        sem_wait(&mutex_2);

        if (buffer_size_1 > 0 && buffer_size_2 > 0) {
            buffer_size_1--;
            buffer_size_2--;
            printf("-"RED"consumer_type_2, id=%d consumed: %d,%d"RESET" from BUFF 1 & 2, index1: %d, index2: %d \n",
                   ID, buffer_1[buffer_size_1], buffer_2[buffer_size_2], buffer_size_1, buffer_size_2);

            sem_post(&full_1);
            sem_post(&full_2);
        }

        if (buffer_size_1 == 0 || buffer_size_2 == 0) {
            sem_post(&queue_type2);
            printf("-"RED"consumer_type_2, id=%d ADDED TO QUEUE "RESET" \n", ID);
        }

        sem_post(&mutex_1);
        sem_post(&mutex_2);
        sem_post(&notify_queue);

        work_counter--;
        random_sleep();
        sleep(2);
    }

    pthread_exit(NULL);
}

//=======================================================================================

int draw_buffer() {
    return rand() % 2 + 1;
}

void random_sleep() {
    if (rand() % 2 == 1) {
        sleep(1);
    }
}
