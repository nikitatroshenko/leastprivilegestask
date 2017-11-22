#ifndef LEASTPRIVELEGETASK_SIGNAL
#define LEASTPRIVELEGETASK_SIGNAL

#include <signal.h>

typedef void(*signal_handler)(int signal, void *data);

void add_signal_handler(int signal, signal_handler signal_handler, void *data);

#endif