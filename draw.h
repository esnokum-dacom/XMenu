#ifndef DRAW_H
#define DRAW_H

#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define COLOR_BLOCK 1
#define COLOR_BAR_UF 5
#define COLOR_BAR_F 0

typedef struct {
  unsigned long colors[16];
  unsigned long background;
  unsigned long foreground;
} ColorScheme;

unsigned long hex_to_xcolor(Display *dpy, const char *hex);

void load_colors(Display *dpy, ColorScheme *col);

void xcolor_to_xftcolor(Display *dpy, Visual *vis, Colormap cmap,
                        unsigned long pixel, XftColor *xft);

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
