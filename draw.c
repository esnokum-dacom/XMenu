#include "draw.h"
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

void draw_wrapped_text(Display *dpy, XftDraw *xdraw, XftFont *font,
                       XftColor *color, int x, int y, int max_width,
                       const char *text) {

  char line[1280] = {0};
  char word[256];
  char test[1280];

  int line_h = font->ascent + font->descent + 2;
  int cy = y;
  const char *p = text;

  while (*p) {

    int i = 0;

    while (*p && *p != ' ' && *p != '\n')
      word[i++] = *p++;

    word[i] = '\0';

    if (strlen(line) == 0)
      snprintf(test, sizeof(test), "%s", word);
    else
      snprintf(test, sizeof(test), "%s %s", line, word);

    XGlyphInfo ext;
    XftTextExtentsUtf8(dpy, font, (FcChar8 *)test, strlen(test), &ext);

    if (ext.xOff > max_width) {

      XftDrawStringUtf8(xdraw, color, font, x, cy, (FcChar8 *)line,
                        strlen(line));
      cy += line_h;
      snprintf(line, sizeof(line), "%s", word);
    } else
      snprintf(line, sizeof(line), "%s", test);

    if (*p == ' ')
      p++;

    if (*p == '\n') {
      XftDrawStringUtf8(xdraw, color, font, x, cy, (FcChar8 *)line,
                        strlen(line));
      cy += line_h;
      line[0] = '\0';
      p++;
    }
  }

  if (strlen(line) > 0) {
    XftDrawStringUtf8(xdraw, color, font, x, cy, (FcChar8 *)line, strlen(line));
  }
}

int count_wrapped_lines(Display *dpy, XftFont *font, int max_width,
                        const char *text) {
  char line[1280] = {0};
  char word[256];
  char test[1280];

  int lines = 1;
  const char *p = text;

  while (*p) {
    int i = 0;

    while (*p && *p != ' ' && *p != '\n')
      word[i++] = *p++;

    word[i] = '\0';
    if (strlen(line) == 0)
      snprintf(test, sizeof(test), "%s", word);
    else
      snprintf(test, sizeof(test), "%s %s", line, word);

    XGlyphInfo ext;
    XftTextExtentsUtf8(dpy, font, (FcChar8 *)test, strlen(test), &ext);

    if (ext.xOff > max_width) {
      lines++;
      snprintf(line, sizeof(line), "%s", word);
    } else
      snprintf(line, sizeof(line), "%s", test);

    if (*p == ' ')
      p++;

    if (*p == '\n') {
      lines++;
      line[0] = '\0';
      p++;
    }
  }

  return lines;
}

void draw_card(Display *dpy, Window win, GC gc, XftDraw *xdraw, XftFont *font,
               XftColor *title_color, unsigned long background,
               unsigned long foreground, XftColor *value_color, int x, int y,
               int w, int h, const char *title, const char *value, int pct) {

  /* That is for add extra height to the card*/

  int line_h = font->ascent + font->descent + 2;
  int title_lines = count_wrapped_lines(dpy, font, w - 20, title);
  int value_lines = count_wrapped_lines(dpy, font, w - 20, value);
  int padding = 20;

  h = (title_lines + value_lines) * line_h + padding;

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

  char buf[1024];
  snprintf(buf, sizeof(buf), "%s", title);
  int ty = y + 20;
  draw_wrapped_text(dpy, xdraw, font, title_color, x + 10, ty, w - 20, buf);

  char vbuf[1024];
  snprintf(vbuf, sizeof(vbuf), "%s", value);
  int vy = ty + (title_lines * line_h);
  draw_wrapped_text(dpy, xdraw, font, value_color, x + 10, vy, w - 20, vbuf);
}

void draw_text(Display *dpy, XftDraw *xdraw, XftFont *font,
               XftColor *title_color, int x, int y, const char *title,
               int16_t type) {

  XGlyphInfo extents;
  XftTextExtentsUtf8(dpy, font, (FcChar8 *)title, strlen(title), &extents);

  int w_s = extents.xOff;

  if (type == 1)
    XftDrawStringUtf8(xdraw, title_color, font, x - w_s, y, (FcChar8 *)title,
                      strlen(title));
  else if (type == 2)
    XftDrawStringUtf8(xdraw, title_color, font, x + w_s, y, (FcChar8 *)title,
                      strlen(title));
  else if (type == 3)
    XftDrawStringUtf8(xdraw, title_color, font, x / w_s, y, (FcChar8 *)title,
                      strlen(title));
  else if (type == 0)
    XftDrawStringUtf8(xdraw, title_color, font, x, y, (FcChar8 *)title,
                      strlen(title));
}
