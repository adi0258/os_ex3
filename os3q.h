#ifndef OS3Q_H
#define OS3Q_H

// The list itself is defined as a struct named QueueOS
typedef struct QueueOS QueueOS;


// Init: Receives max size and initializes a new queue
QueueOS* init(int max_size);

// Destroy: Deletes queue and frees allocations
void destroy(QueueOS* q);

// Enqueue: Receives a value (long) and adds it to the end
void enqueue(QueueOS* q, long value);

// Dequeue: Removes element from head and returns its value
long dequeue(QueueOS* q);

// Size: Returns the number of elements
int size(QueueOS* q);

// Sum: Returns the sum of all elements
long sum(QueueOS* q);

#endif