#include "signal.h"

volatile sig_atomic_t window_resized = 0;

void handle_sigwinch(int sig) {
    (void)sig;
    window_resized = 1;
}

void init_signals() {
    struct sigaction sa;
    sa.sa_handler = handle_sigwinch;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGWINCH, &sa, NULL);
}
