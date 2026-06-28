#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>

extern volatile sig_atomic_t window_resized;

void handle_sigwinch(int sig);
void init_signals();

#endif
