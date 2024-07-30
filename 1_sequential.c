#include <unistd.h>

#include "termbox2.h"

#define BUFFER_SIZE 32

void
collect_user_input(int x, int y)
{
    char buffer[BUFFER_SIZE] = {0};
    int dx = 0;
    struct tb_event ev;

    while (1) {
        tb_printf(x, y, TB_RED, TB_BLACK, "%s ", buffer);
        tb_set_cursor(x + dx, y);
        tb_present();
        tb_poll_event(&ev);
        if (ev.type == TB_EVENT_KEY && ev.ch && dx + 1 < BUFFER_SIZE) {
            buffer[dx++] = ev.ch;
        } else if (ev.type == TB_EVENT_KEY && ev.key == TB_KEY_BACKSPACE2 && dx > 0) {
            buffer[--dx] = '\0';
        } else if (ev.type == TB_EVENT_KEY && ev.key == TB_KEY_ENTER) {
            tb_hide_cursor();
            tb_present();
            return;
        }
    }
}

void
print_counter_from(int x, int y, int n)
{
    for (; n; n--) {
        tb_printf(x, y, TB_GREEN, TB_BLACK, "%d ", n);
        tb_present();
        sleep(1);
    }
    tb_print(x, y, TB_GREEN, TB_BLACK, "DONE!");
    tb_present();
}

int
main(int argc, char *argv[])
{
    tb_init();
    tb_clear();
    collect_user_input(0, 0);
    print_counter_from(0, 1, 5);
    tb_shutdown();
    return 0;
}
