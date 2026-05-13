#ifndef INFO_H
#define INFO_H

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <string.h>

void get_ram(char *buf, int bufsz, int *pct);
void get_battery(char *buf, int bufsz, int *pct);
void get_hour(char *buf, int bufsz, int sec);
void get_date(char *buf, int bufsz);
void get_user(char *buf, int bufsz); // linux

void task(const char *path, int title, int body, char *buft, int bufszt,
          char *bufb, int bufszb, int *pct);

#endif
