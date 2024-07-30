#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "termbox2.h"

#define BUFFER_SIZE 32

struct int_llist {
    struct int_llist *next;
    int value;
};

pthread_mutex_t termbox_mutex, queue_mutex;
struct int_llist *queue_first_in, *queue_last_in;

void
push_to_queue(int value)
{
    struct int_llist *new = malloc(sizeof(struct int_llist));
    new->value = value;
    new->next = NULL;
    pthread_mutex_lock(&queue_mutex);
    if (queue_last_in) {
        queue_last_in->next = new;
    } else {
        queue_first_in = new;
    }
    queue_last_in = new;
    pthread_mutex_unlock(&queue_mutex);
}

int
pop_from_queue(int *value)
{
    int res;
    struct int_llist *next;

    pthread_mutex_lock(&queue_mutex);
    if (queue_first_in) {
        res = 0;
        next = queue_first_in->next;
        *value = queue_first_in->value;
        free(queue_first_in);
        queue_first_in = next;
        if (!queue_first_in) {
            queue_last_in = NULL;
        }
    } else {
        res = -1;
    }
    pthread_mutex_unlock(&queue_mutex);
    return res;
}

void *
collect_user_input(void *arg)
{
    char buffer[BUFFER_SIZE] = {0};
    int x = 0, y = 0, dx = 0, new_value;
    struct tb_event ev;

    while (1) {
        pthread_mutex_lock(&termbox_mutex);
        tb_printf(x, y, TB_RED, TB_BLACK, "%s ", buffer);
        tb_set_cursor(x + dx, y);
        tb_present();
        pthread_mutex_unlock(&termbox_mutex);
        tb_poll_event(&ev);
        if (ev.type == TB_EVENT_KEY && ev.ch && dx + 1 < BUFFER_SIZE) {
            buffer[dx++] = ev.ch;
        } else if (ev.type == TB_EVENT_KEY && ev.key == TB_KEY_BACKSPACE2 && dx > 0) {
            buffer[--dx] = '\0';
        } else if (ev.type == TB_EVENT_KEY && ev.key == TB_KEY_ENTER) {
            if (!strcmp(buffer, "exit")) {
                pthread_mutex_lock(&termbox_mutex);
                tb_hide_cursor();
                tb_present();
                pthread_mutex_unlock(&termbox_mutex);
                return NULL;
            } else if ((new_value = atoi(buffer))) {
                pthread_mutex_lock(&termbox_mutex);
                tb_printf(x, y, TB_BLACK, TB_BLACK, "%s ", buffer);
                tb_present();
                pthread_mutex_unlock(&termbox_mutex);
                memset(buffer, 0, sizeof(buffer));
                dx = 0;
                push_to_queue(new_value);
            }
        }
    }
}

void *
print_countdown(void *arg)
{
    int x = 0, y = 1, n;

    while (pop_from_queue(&n) >= 0) {
        for (; n; n--) {
            pthread_mutex_lock(&termbox_mutex);
            tb_printf(x, y, TB_GREEN, TB_BLACK, "%d ", n);
            tb_present();
            pthread_mutex_unlock(&termbox_mutex);
            sleep(1);
        }
    }
    pthread_mutex_lock(&termbox_mutex);
    tb_print(x, y, TB_GREEN, TB_BLACK, "DONE!");
    tb_present();
    pthread_mutex_unlock(&termbox_mutex);
    return NULL;
}

int
main(int argc, char *argv[])
{
    pthread_t countdown_thread_id, user_input_thread_id;

    tb_init();
    tb_clear();
    pthread_mutex_init(&termbox_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);
    push_to_queue(5);
    pthread_create(&user_input_thread_id, NULL, collect_user_input, NULL);
    pthread_create(&countdown_thread_id, NULL, print_countdown, NULL);
    pthread_join(countdown_thread_id, NULL);
    pthread_join(user_input_thread_id, NULL);
    pthread_mutex_destroy(&termbox_mutex);
    pthread_mutex_destroy(&queue_mutex);
    tb_shutdown();
    return 0;
}
