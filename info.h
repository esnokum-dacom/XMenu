#ifndef INFO_H
#define INFO_H

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <string.h>

void get_ram(char *buf, int bufsz, int *pct);
void get_battery(char *buf, int bufsz, int *pct);

#endif
