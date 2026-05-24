#include "sigr1.h"

static volatile int reload_colors = 0;

void handle_sigusr1(int sig) { reload_colors = 1; }
