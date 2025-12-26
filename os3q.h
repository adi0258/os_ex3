#ifndef OS3Q_H
#define OS3Q_H

typedef struct QueueOS QueueOS;

QueueOS* init(int max_size);
void destroy(QueueOS* q);
void enqueue(QueueOS* q, long value);
long dequeue(QueueOS* q);
int size(QueueOS* q);
long sum(QueueOS* q);

#endif