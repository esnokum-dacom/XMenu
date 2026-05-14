#ifndef INFO_H
#define INFO_H

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

void get_ram(char *buf, int bufsz, int *pct);
void get_battery(char *buf, int bufsz, int *pct);
void get_hour(char *buf, int bufsz, int sec);
void get_date(char *buf, int bufsz);
void get_user(char *buf, int bufsz); // linux
void get_os(char *buf, int bufsz);
void get_kernel(char *buf, int bufsz);
void get_shell(char *buf, int bufsz);
void get_hostname(char *buf, int bufsz);

void get_distro(char *buf, int bufsz);

void task(const char *path, int title, int body, char *buft, int bufszt,
          char *bufb, int bufszb, int *pct);

#endif
