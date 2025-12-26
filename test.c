
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "os3q.h"
#define PRODUCER_COUNT 3
#define CONSUMER_COUNT 3
#define ITEMS_PER_THREAD 1000
#define QUEUE_SIZE 10 // Small size to force blocking/waiting

QueueOS *queue;

// Producer thread function
void* producer(void* arg) {
    long id = (long)arg;
    for (int i = 0; i < ITEMS_PER_THREAD; i++) {
        enqueue(queue, 1); // Always enqueue '1' for easy summing
    }
    printf("Producer %ld finished.\n", id);
    return NULL;
}

// Consumer thread function
void* consumer(void* arg) {
    long id = (long)arg;
    long local_sum = 0;
    for (int i = 0; i < ITEMS_PER_THREAD; i++) {
        local_sum += dequeue(queue);
    }
    printf("Consumer %ld finished. Local Sum: %ld\n", id, local_sum);
    return (void*)local_sum;
}

int main() {
    printf("--- Starting Test ---\n");

    // 1. Initialize Queue
    queue = init(QUEUE_SIZE);

    pthread_t prod[PRODUCER_COUNT];
    pthread_t cons[CONSUMER_COUNT];

    // 2. Start Consumers
    for (long i = 0; i < CONSUMER_COUNT; i++) {
        pthread_create(&cons[i], NULL, consumer, (void*)i);
    }

    // 3. Start Producers
    for (long i = 0; i < PRODUCER_COUNT; i++) {
        pthread_create(&prod[i], NULL, producer, (void*)i);
    }

    // 4. Wait for Producers to finish
    for (int i = 0; i < PRODUCER_COUNT; i++) {
        pthread_join(prod[i], NULL);
    }

    // 5. Wait for Consumers to finish and collect results
    long total_consumed = 0;
    for (int i = 0; i < CONSUMER_COUNT; i++) {
        void* result;
        pthread_join(cons[i], &result);
        total_consumed += (long)result;
    }

    // 6. Final Validation
    int final_size = size(queue);
    long expected_total = PRODUCER_COUNT * ITEMS_PER_THREAD; // All 1s

    printf("\n--- Results ---\n");
    printf("Final Queue Size: %d (Expected: 0)\n", final_size);
    printf("Total Items Consumed: %ld (Expected: %ld)\n", total_consumed, expected_total);

    if (final_size == 0 && total_consumed == expected_total) {
        printf("SUCCESS: The queue is thread-safe!\n");
    } else {
        printf("FAILURE: Data loss or race condition detected.\n");
    }

    // 7. Cleanup
    destroy(queue);
    return 0;
}