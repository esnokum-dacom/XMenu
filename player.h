#ifndef PLAYER_H
#define PLAYER_H

#include <Imlib2.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <libgen.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
  int x, y, w, h;
} PlayerButtons;

const char *get_home();

void get_song(char *buf, int bufsz);

void get_time(char *buf, int bufsz);

void draw_cover(Display *dpy, Window win, int x, int y, int size,
                const char *art_url);

void draw_player(Display *dpy, Window win, GC gc, XftDraw *xdraw, XftFont *font,
                 XftColor *title_color, unsigned long foreground,
                 unsigned long background, XftColor *value_color, int x, int y,
                 int w, int h, char *last_art_url, PlayerButtons *btns,
                 Visual *visual, Colormap cmap);

#endif
