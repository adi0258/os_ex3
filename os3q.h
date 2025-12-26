struct QueueOS;

void queueos_init(struct QueueOS *q, int max);

void queueos_destroy(struct QueueOS *q);

int queueos_size(struct QueueOS *q);

long queueos_sum(struct QueueOS *q);

long queueos_dequeue(struct QueueOS *q);

void queueos_enqueue(struct QueueOS *q, long val);
