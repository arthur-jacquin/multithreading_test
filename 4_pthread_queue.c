#include <pthread.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "4_pthread_queue.h"

void
pthread_queue_destroy(struct pthread_queue *queue)
{
    pthread_mutex_destroy(&(queue->mutex));
    for_llist_safe(queue->first_in, elem) {
        free(elem);
    }
    queue->first_in = queue->last_in = NULL;
}

void
pthread_queue_init(struct pthread_queue *queue, size_t data_size)
{
    queue->data_size = data_size;
    pthread_mutex_init(&(queue->mutex), NULL);
    queue->first_in = queue->last_in = NULL;
}

int
pthread_queue_is_empty(struct pthread_queue *queue)
{
    int res;
    pthread_mutex_lock(&(queue->mutex));
    res = !(queue->first_in);
    pthread_mutex_unlock(&(queue->mutex));
    return res;
}

int
pthread_queue_pop(struct pthread_queue *queue, void *dest)
{
    int res;
    struct llist *next;

    pthread_mutex_lock(&(queue->mutex));
    if (queue->first_in) {
        res = 0;
        memcpy(dest, queue->first_in + sizeof(struct llist), queue->data_size);
        next = queue->first_in->next;
        if (!next) {
            queue->last_in = NULL;
        }
        free(queue->first_in);
        queue->first_in = next;
    } else {
        res = -1;
    }
    pthread_mutex_unlock(&(queue->mutex));
    return res;
}

void
pthread_queue_push(struct pthread_queue *queue, const void *src)
{
    struct llist *new = malloc(sizeof(struct llist) + queue->data_size);
    new->next = NULL;
    memcpy(new + sizeof(struct llist), src, queue->data_size);
    pthread_mutex_lock(&(queue->mutex));
    if (queue->last_in) {
        queue->last_in->next = new;
    } else {
        queue->first_in = new;
    }
    queue->last_in = new;
    pthread_mutex_unlock(&(queue->mutex));
}
