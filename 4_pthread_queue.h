#ifndef PTHREAD_QUEUE_H
#define PTHREAD_QUEUE_H

#include <stddef.h>

struct llist {
    struct llist *next;
};

#define for_llist_safe(list, ptr) \
    for (typeof(*list) *(ptr) = (list), *next; (ptr) && \
        (next = (ptr)->next, 1); (ptr) = next)

struct pthread_queue {
    size_t data_size;
    pthread_mutex_t mutex;
    struct llist *first_in, *last_in;
};

void pthread_queue_destroy(struct pthread_queue *queue);
void pthread_queue_init(struct pthread_queue *queue, size_t data_size);
int pthread_queue_is_empty(struct pthread_queue *queue);
int pthread_queue_pop(struct pthread_queue *queue, void *dest);
void pthread_queue_push(struct pthread_queue *queue, const void *src);

#endif // PTHREAD_QUEUE_H
