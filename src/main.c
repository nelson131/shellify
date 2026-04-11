#include <stdio.h>
#include <stdlib.h>

#include "shellify.h"

int main() {
    while (shellify_is_running) {
        shellify_draw();
        shellify_handle_input();
    }

    shellify_destroy();

    return 0;
}
