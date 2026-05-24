#ifndef SIGR1_h
#define SIGR1_h

#include <signal.h>

static volatile int reload_colors;

void handle_sigusr1(int sig);

#endif
