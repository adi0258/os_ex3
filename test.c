#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "os3q.h"

// --- DEFINITIONS FOR TESTING ---
typedef struct Node {
    long data;
    struct Node* next;
} Node;

struct QueueOS {
    Node* head;
    Node* tail;
    int count;
    int max_size;

    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
};

// **CRITICAL FIX:** This allows you to use 'QueueOS' without the 'struct' keyword
typedef struct QueueOS QueueOS;
// -----------------------------

#define PRODUCER_COUNT 3
#define CONSUMER_COUNT 3
#define ITEMS_PER_THREAD 1000
#define QUEUE_SIZE 10

QueueOS *queue; // Now this works because of the typedef above

void* producer(void* arg) {
    long id = (long)arg;
    for (int i = 0; i < ITEMS_PER_THREAD; i++) {
        queueos_enqueue(queue, 1);
    }
    return NULL;
}

void* consumer(void* arg) {
    long local_sum = 0;
    for (int i = 0; i < ITEMS_PER_THREAD; i++) {
        local_sum += queueos_dequeue(queue);
    }
    return (void*)local_sum;
}

int main() {
    printf("--- Starting Test ---\n");

    // 1. Allocate memory
    queue = (QueueOS*)malloc(sizeof(QueueOS));
    if (!queue) return 1;

    // 2. Init
    queueos_init(queue, QUEUE_SIZE);

    pthread_t prod[PRODUCER_COUNT];
    pthread_t cons[CONSUMER_COUNT];

    // Start Threads
    for (long i = 0; i < CONSUMER_COUNT; i++)
        pthread_create(&cons[i], NULL, consumer, (void*)i);

    for (long i = 0; i < PRODUCER_COUNT; i++)
        pthread_create(&prod[i], NULL, producer, (void*)i);

    // Join Threads
    for (int i = 0; i < PRODUCER_COUNT; i++)
        pthread_join(prod[i], NULL);

    long total_consumed = 0;
    for (int i = 0; i < CONSUMER_COUNT; i++) {
        void* result;
        pthread_join(cons[i], &result);
        total_consumed += (long)result;
    }

    // Results
    int final_size = queueos_size(queue);
    long expected = PRODUCER_COUNT * ITEMS_PER_THREAD;

    printf("Final Size: %d (Expected: 0)\n", final_size);
    printf("Total Consumed: %ld (Expected: %ld)\n", total_consumed, expected);

    if (final_size == 0 && total_consumed == expected)
        printf("SUCCESS\n");
    else
        printf("FAILURE\n");

    // Cleanup
    queueos_destroy(queue);
    free(queue);

    return 0;
}