#include <unistd.h>

#include "shellify.h"

int main() {
    shellify_init();
    while (shellify->is_running) {
        if (!shellify_screen_size()) continue;
        shellify_handle_input();
        shellify_update();
        shellify_draw();
        usleep(10000);
    }

    shellify_destroy();

    return 0;
}
