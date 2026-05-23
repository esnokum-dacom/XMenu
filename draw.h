#ifndef DRAW_H
#define DRAW_H

#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void draw_wrapped_text(Display *dpy, XftDraw *xdraw, XftFont *font,
                       XftColor *color, int x, int y, int max_width,
                       const char *text);

int count_wrapped_lines(Display *dpy, XftFont *font, int max_width,
                        const char *text);

void draw_card(Display *dpy, Window win, GC gc, XftDraw *xdraw, XftFont *font,
               XftColor *title_color, unsigned long background,
               unsigned long foreground, XftColor *value_color, int x, int y,
               int w, int h, const char *title, const char *value, int pct);

void draw_text(Display *dpy, XftDraw *xdraw, XftFont *font,
               XftColor *title_color, int x, int y, const char *title,
               int16_t type);

#endif
