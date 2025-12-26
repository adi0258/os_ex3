#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "os3q.h"

// Define the Node struct
typedef struct Node {
    long data;
    struct Node* next;
} Node;

// Define the QueueOS struct (Complete definition needed here)
struct QueueOS {
    Node* head;
    Node* tail;
    int count;
    int max_size;

    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
};

// 1. Init
// Note: 'q' is passed by the caller. We do NOT malloc the queue itself here.
void queueos_init(struct QueueOS* q, int max) {
    if (q == NULL) return;

    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
    q->max_size = max; // Set the max_size from the argument 'max'

    // Initialize synchronization primitives
    if (pthread_mutex_init(&q->lock, NULL) != 0) {
        perror("Mutex init failed");
        exit(1);
    }
    if (pthread_cond_init(&q->not_full, NULL) != 0) {
        perror("Cond init failed");
        pthread_mutex_destroy(&q->lock);
        exit(1);
    }
    if (pthread_cond_init(&q->not_empty, NULL) != 0) {
        perror("Cond init failed");
        pthread_mutex_destroy(&q->lock);
        pthread_cond_destroy(&q->not_full);
        exit(1);
    }
}

// 2. Destroy
void queueos_destroy(struct QueueOS* q) {
    if (q == NULL) return;

    pthread_mutex_lock(&q->lock);
    Node* current = q->head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp); // Free the nodes
    }
    pthread_mutex_unlock(&q->lock);

    // Destroy primitives
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);

    // DO NOT free(q). Since init didn't malloc 'q', destroy shouldn't free it.
}

// 3. Enqueue
void queueos_enqueue(struct QueueOS* q, long val) {
    pthread_mutex_lock(&q->lock);

    // Wait while queue is full
    while (q->count >= q->max_size) {
        pthread_cond_wait(&q->not_full, &q->lock);
    }

    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Failed to allocate node");
        pthread_mutex_unlock(&q->lock);
        exit(1);
    }

    new_node->data = val; // Use 'val', not 'value'
    new_node->next = NULL;

    if (q->tail == NULL) {
        q->head = new_node;
        q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
    q->count++;

    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

// 4. Dequeue
long queueos_dequeue(struct QueueOS* q) {
    pthread_mutex_lock(&q->lock);

    // Wait while queue is empty
    while (q->count == 0) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }

    Node* temp = q->head;
    long value = temp->data;
    q->head = q->head->next;

    if (q->head == NULL) {
        q->tail = NULL;
    }

    free(temp);
    q->count--;

    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->lock);

    return value;
}

// 5. Size
int queueos_size(struct QueueOS* q) {
    pthread_mutex_lock(&q->lock);
    int c = q->count;
    pthread_mutex_unlock(&q->lock);
    return c;
}

// 6. Sum
long queueos_sum(struct QueueOS* q) {
    pthread_mutex_lock(&q->lock);
    long total = 0;
    Node* current = q->head;
    while (current != NULL) {
        total += current->data;
        current = current->next;
    }
    pthread_mutex_unlock(&q->lock);
    return total;
}