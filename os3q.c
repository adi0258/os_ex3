#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "os3q.h"

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

// 1. Init
QueueOS* init(int max_size) {
    QueueOS* q = (QueueOS*)malloc(sizeof(QueueOS));
    if (q == NULL) {
        perror("Failed to allocate queue");
        exit(1);
    }
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
    q->max_size = max_size;

    if (pthread_mutex_init(&q->lock, NULL) != 0) {
        perror("Mutex init failed");
        free(q);
        exit(1);
    }
    if (pthread_cond_init(&q->not_full, NULL) != 0) {
        perror("Cond init failed");
        pthread_mutex_destroy(&q->lock);
        free(q);
        exit(1);
    }
    if (pthread_cond_init(&q->not_empty, NULL) != 0) {
        perror("Cond init failed");
        pthread_mutex_destroy(&q->lock);
        pthread_cond_destroy(&q->not_full);
        free(q);
        exit(1);
    }
    return q;
}

// 2. Destroy
void destroy(QueueOS* q) {
    if (q == NULL) return;

    pthread_mutex_lock(&q->lock);
    Node* current = q->head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
    pthread_mutex_unlock(&q->lock);
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
    free(q);
}

// 3. Enqueue
void enqueue(QueueOS* q, long value) {
    pthread_mutex_lock(&q->lock);
    while (q->count >= q->max_size) {
        pthread_cond_wait(&q->not_full, &q->lock);
    }
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Failed to allocate node");
        pthread_mutex_unlock(&q->lock);
        exit(1);
    }

    new_node->data = value;
    new_node->next = NULL;

    if (q->tail == NULL) {
        q->head = new_node;
        q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
    q->count++;
    pthread_cond_signal(&q->not_empty);//signal consumers that data is available
    pthread_mutex_unlock(&q->lock);
}

// 4. Dequeue
long dequeue(QueueOS* q) {
    pthread_mutex_lock(&q->lock);
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
    pthread_cond_signal(&q->not_full);// Signal producers that space is available
    pthread_mutex_unlock(&q->lock);
    return value;
}

// 5. Size
int size(QueueOS* q) {
    pthread_mutex_lock(&q->lock);
    int c = q->count;
    pthread_mutex_unlock(&q->lock);
    return c;
}

// 6. Sum
long sum(QueueOS* q) {
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