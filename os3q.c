// You must implement a queue in a file named `os3q.c`.
// * The queue must be implemented using the **classic method with dynamic allocation** (linked list), where each element holds a value of type `long`.
// * The queue has a maximum size (maximum number of elements) determined during initialization.
// * The queue must support concurrency: all operations must work correctly and safely when multiple threads access the queue simultaneously.
// **Synchronization Requirements**
// * To support concurrency, you must implement the queue using **Mutexes (locks) and Condition Variables** only.
// **Do not** use semaphores or atomic operations.
// **Struct and Functions**
// The list itself should be defined as a struct named `QueueOS` and support the following operations:
// 1. **Init (`init`):**
// * Receives the maximum size and initializes a new queue.
// * Assumption: This function is called exactly once, before any other operation, and without parallel operations on the same queue.
// 2. **Destroy (`destroy`):**
// * Deletes the queue by freeing all relevant allocations (including elements remaining in the queue).
// * Assumption: This function is called exactly once, after all other operations, and without parallel operations on the same queue.
// 3. **Enqueue (`enqueue`):**
// * Receives a value and adds it to the end of the queue.
// **Blocking:** If the queue is at maximum size, the thread must block (wait) until it is possible to add the value.
// 4. **Dequeue (`dequeue`):**
// * Removes the element at the head of the queue and returns its value.
// **Blocking:** If the queue is empty, the thread must block (wait) until it is possible to remove a value.
// 5. **Size (`size`):**
// * Returns the number of elements currently in the queue.
// 6. **Sum (`sum`):**
// * Returns the sum of all elements in the queue.
// **Technical Guidelines**
// * **Header File:** A header file named `os3q.h` containing the queue declaration and operations is provided. **Do not** change this file and do not submit it.
// **Helper Functions:** You may (and should) add helper functions and definitions in your `os3q.c` file.
// **Implementation Style:** Do not implement the list using an array; it must be a list of pointers (classic implementation).
// **Globals:** Avoid defining global or static variables.
// **Multiple Queues:** The solution must support defining multiple queues, where each can be used separately and in parallel to others. Each queue may have a different maximum size.
// * **Submission:** Submit the code **without a `main` function**. Adding a `main` function will cause the check to fail and disqualify the entire section.
// **Error Checking:** You must check the return values of all system calls and memory allocations (specifically `malloc`).
// ### General Submission Instructions (Relevant to Part A)
// * The code will be compiled using: `gcc <codefiles> -Wall -pthread -o <exefile>`.
// * The code must compile without warnings.
// * You must manage resources and memory correctly (freeing memory, etc.).
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
        perror("Failed to allocate queue"); //
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
    pthread_mutex_lock(&q->lock);// LOCK START
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
    pthread_mutex_unlock(&q->lock);//LOCK END
}

// 4. Dequeue
long dequeue(QueueOS* q) {
    pthread_mutex_lock(&q->lock);// LOCK START
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
    pthread_mutex_unlock(&q->lock);// LOCK END
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
    pthread_mutex_lock(&q->lock);// LOCK START
    long total = 0;
    Node* current = q->head;
    while (current != NULL) {
        total += current->data;
        current = current->next;
    }
    pthread_mutex_unlock(&q->lock);// LOCK END
    return total;
}