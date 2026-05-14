#include "draw.h"
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>

void draw_card(Display *dpy, Window win, GC gc, XftDraw *xdraw, XftFont *font,
               XftColor *title_color, unsigned long background,
               unsigned long foreground, XftColor *value_color, int x, int y,
               int w, int h, const char *title, const char *value, int pct) {

  XSetForeground(dpy, gc, background);
  XFillRectangle(dpy, win, gc, x, y, w, h);

  int bar_h = 6;
  int bar_y = y + h - bar_h - 10;
  int bar_x = x + 10;
  int bar_w = w - 20;

  XSetForeground(dpy, gc, 0x303030);
  XFillRectangle(dpy, win, gc, bar_x, bar_y, bar_w, bar_h);

  int fill_w = bar_w * pct / 100;
  XSetForeground(dpy, gc, foreground);
  XFillRectangle(dpy, win, gc, bar_x, bar_y, fill_w, bar_h);

  XftDrawStringUtf8(xdraw, title_color, font, x + 10, y + 20, (FcChar8 *)title,
                    strlen(title));

  XftDrawStringUtf8(xdraw, value_color, font, x + 10, y + h - 25,
                    (FcChar8 *)value, strlen(value));
}

void draw_text(XftDraw *xdraw, XftFont *font, XftColor *title_color, int x,
               int y, const char *title) {

  XftDrawStringUtf8(xdraw, title_color, font, x + 10, y + 20, (FcChar8 *)title,
                    strlen(title));
}
