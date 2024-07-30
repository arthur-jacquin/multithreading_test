#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "termbox2.h"
#include "4_pthread_queue.h"

#define BUFFER_SIZE 32

int finished;
pthread_cond_t countdown_cond;
pthread_mutex_t finished_mutex, termbox_mutex;
struct pthread_queue durations_queue;

int
is_finished(void)
{
    pthread_mutex_lock(&finished_mutex);
    int res = finished;
    pthread_mutex_unlock(&finished_mutex);
    return res;
}

void
set_finished(void)
{
    pthread_mutex_lock(&finished_mutex);
    finished = 1;
    pthread_mutex_unlock(&finished_mutex);
}

void
pthread_tb_printf_d(int x, int y, uintattr_t fg, uintattr_t bg, const char *fmt,
    int d)
{
    pthread_mutex_lock(&termbox_mutex);
    tb_printf(x, y, fg, bg, fmt, d);
    tb_present();
    pthread_mutex_unlock(&termbox_mutex);
}

void
pthread_tb_printf_s(int x, int y, uintattr_t fg, uintattr_t bg, const char *fmt,
    const char *s)
{
    pthread_mutex_lock(&termbox_mutex);
    tb_printf(x, y, fg, bg, fmt, s);
    tb_present();
    pthread_mutex_unlock(&termbox_mutex);
}

void *
collect_user_input(void *arg)
{
    char buffer[BUFFER_SIZE] = {0};
    int x = 0, y = 0, dx = 0, new_value;
    struct tb_event ev;

    while (1) {
        pthread_tb_printf_s(x, y, TB_RED, TB_BLACK, "%s ", buffer);
        tb_poll_event(&ev);
        if (ev.type == TB_EVENT_KEY && ev.ch && dx + 1 < BUFFER_SIZE) {
            buffer[dx++] = ev.ch;
        } else if (ev.type == TB_EVENT_KEY && ev.key == TB_KEY_BACKSPACE2 &&
            dx > 0) {
            buffer[--dx] = '\0';
        } else if (ev.type == TB_EVENT_KEY && ev.key == TB_KEY_ENTER) {
            if (!strcmp(buffer, "exit")) {
                // end of user input
                set_finished();
                pthread_cond_signal(&countdown_cond);
                return NULL;

            } else if ((new_value = atoi(buffer))) {
                // pushing new duration to queue
                pthread_tb_printf_s(x, y, TB_BLACK, TB_BLACK, "%s ", buffer);
                memset(buffer, 0, sizeof(buffer));
                dx = 0;
                pthread_queue_push(&durations_queue, &new_value);
                pthread_cond_signal(&countdown_cond);
            }
        }
    }
}

void *
print_countdown(void *arg)
{
    int x = 0, y = 1, c, n;
    const char wait_msg[] = "waiting for another duration...";

    do {
        // display a countdown
        if ((c = pthread_queue_pop(&durations_queue, &n)) >= 0) {
            for (; n; n--) {
                pthread_tb_printf_d(x, y, TB_GREEN, TB_BLACK, "%d ", n);
                sleep(1);
            }
        }
        if (c < 0 || pthread_queue_is_empty(&durations_queue)) {
            pthread_tb_printf_s(x, y, TB_GREEN, TB_BLACK, wait_msg, NULL);
        }

        // wait for another event
        pthread_mutex_lock(&finished_mutex);
        while (!finished && pthread_queue_is_empty(&durations_queue)) {
            pthread_cond_wait(&countdown_cond, &finished_mutex);
        }
        pthread_mutex_unlock(&finished_mutex);

        // clear
        pthread_tb_printf_s(x, y, TB_BLACK, TB_BLACK, wait_msg, NULL);
    } while (!is_finished());

    // final information
    pthread_tb_printf_s(x, y, TB_GREEN, TB_BLACK, "DONE!", NULL);
    sleep(3);

    return NULL;
}

int
main(int argc, char *argv[])
{
    // TODO: error handling accross the program

    // For demonstration purposes, conditional variable and mutexes are
    // dynamically initialised, and explicitely destroyed before main()
    // termination.

    // All threads are explicitely created as joinable, and joined before main()
    // termination (to correctly cleanup ressources): no need to call
    // pthread_exit().

    pthread_attr_t attr;
    pthread_t countdown_thread_id, user_input_thread_id;

init:
    pthread_cond_init(&countdown_cond, NULL);
    pthread_mutex_init(&finished_mutex, NULL);
    pthread_mutex_init(&termbox_mutex, NULL);
    pthread_queue_init(&durations_queue, sizeof(int));
    tb_init();

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&countdown_thread_id, &attr, print_countdown, NULL);
    pthread_create(&user_input_thread_id, &attr, collect_user_input, NULL);
    pthread_join(user_input_thread_id, NULL);
    pthread_join(countdown_thread_id, NULL);

cleanup:
    pthread_cond_destroy(&countdown_cond);
    pthread_mutex_destroy(&finished_mutex);
    pthread_mutex_destroy(&termbox_mutex);
    pthread_queue_destroy(&durations_queue);
    tb_shutdown();

    return 0;
}
