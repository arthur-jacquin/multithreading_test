#include <pthread.h>
#include <unistd.h>

#include "termbox2.h"

#define BUFFER_SIZE 32

pthread_mutex_t termbox_mutex;

void *
collect_user_input(void *arg)
{
    char buffer[BUFFER_SIZE] = {0};
    int x = 0, y = 0, dx = 0;
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
            pthread_mutex_lock(&termbox_mutex);
            tb_hide_cursor();
            tb_present();
            pthread_mutex_unlock(&termbox_mutex);
            return NULL;
        }
    }
}

void *
print_countdown(void *arg)
{
    int x = 0, y = 1, n = 5;

    for (; n; n--) {
        pthread_mutex_lock(&termbox_mutex);
        tb_printf(x, y, TB_GREEN, TB_BLACK, "%d ", n);
        tb_present();
        pthread_mutex_unlock(&termbox_mutex);
        sleep(1);
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
    pthread_create(&countdown_thread_id, NULL, print_countdown, NULL);
    pthread_create(&user_input_thread_id, NULL, collect_user_input, NULL);
    pthread_join(countdown_thread_id, NULL);
    pthread_join(user_input_thread_id, NULL);
    pthread_mutex_destroy(&termbox_mutex);
    tb_shutdown();
    return 0;
}
