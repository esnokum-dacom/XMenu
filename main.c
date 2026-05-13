#include "main.h"
#include "info.h"
#include "player.h"
#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <string.h>

static void draw_card(Display *dpy, Window win, GC gc, XftDraw *xdraw,
                      XftFont *font, XftColor *title_color,
                      unsigned long background, unsigned long foreground,
                      XftColor *value_color, int x, int y, int w, int h,
                      const char *title, const char *value, int pct) {

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

static void draw_text(XftDraw *xdraw, XftFont *font, XftColor *title_color,
                      int x, int y, const char *title) {

  XftDrawStringUtf8(xdraw, title_color, font, x + 10, y + 20, (FcChar8 *)title,
                    strlen(title));
}

int main(void) {
  Display *dpy = XOpenDisplay(NULL);
  if (!dpy) {
    fprintf(stderr, "It could not be open the display\n");
    return 1;
  }

  int screen = DefaultScreen(dpy);
  Window root = DefaultRootWindow(dpy);
  Visual *visual = DefaultVisual(dpy, screen);
  Colormap cmap = DefaultColormap(dpy, screen);
  XftFont *font = XftFontOpenName(dpy, screen, "monospace:size=12");

  int mx = 0, my = 0, mw = DisplayWidth(dpy, screen),
      mh = DisplayHeight(dpy, screen);
  if (XineramaIsActive(dpy)) {
    int n;
    XineramaScreenInfo *info = XineramaQueryScreens(dpy, &n);
    Window dummy_w;
    int dummy_i;
    unsigned int dummy_u;
    int cx, cy;
    XQueryPointer(dpy, root, &dummy_w, &dummy_w, &cx, &cy, &dummy_i, &dummy_i,
                  &dummy_u);
    for (int i = 0; i < n; i++) {
      if (cx >= info[i].x_org && cx < info[i].x_org + info[i].width &&
          cy >= info[i].y_org && cy < info[i].y_org + info[i].height) {
        mx = info[i].x_org;
        my = info[i].y_org;
        mw = info[i].width;
        mh = info[i].height;
        break;
      }
    }
    XFree(info);
  }

  XSetWindowAttributes attrs = {
      .override_redirect = True,
      .background_pixel = 0x151515,
      .event_mask =
          ExposureMask | ButtonPressMask | PointerMotionMask | LeaveWindowMask,
  };
  int win_w = mw / 3;
  int win_h = mh / 2;
  int win_x = mx + (mw - win_w) / 2;
  int win_y = my + (mh - win_h) / 2;

  Window win = XCreateWindow(
      dpy, root, win_x, win_y, win_w, win_h, 0, CopyFromParent, InputOutput,
      CopyFromParent, CWOverrideRedirect | CWBackPixel | CWEventMask, &attrs);

  XMapRaised(dpy, win);
  XSetInputFocus(dpy, win, RevertToParent, CurrentTime);

  GC gc = XCreateGC(dpy, win, 0, NULL);
  XftDraw *xdraw = XftDrawCreate(dpy, win, visual, cmap);

  unsigned long background = 0x000000;
  unsigned long foreground = 0xffffff;
  XftColor title_color, value_color;
  XftColorAllocName(dpy, visual, cmap, "#ffffff", &title_color);
  XftColorAllocName(dpy, visual, cmap, "#ffffff", &value_color);

  char ram_text[64];
  char bat_cap[32];
  char hour[64];
  char date[64];
  char user[32];
  char t_title[16];
  char t_body[16];
  char last_art_url[256] = {0};
  char song[32];
  PlayerButtons btns = {0};
  XEvent ev;

  int ram_pct;
  int batt_pct;
  int task_pct;

  while (1) {
    while (XPending(dpy)) {
      XNextEvent(dpy, &ev);
      if (ev.type == Expose) {
        get_ram(ram_text, sizeof(ram_text), &ram_pct);
        get_battery(bat_cap, sizeof(bat_cap), &batt_pct);
        get_song(song, sizeof(song));
        get_hour(hour, sizeof(hour), 0);
        get_date(date, sizeof(date));
        get_user(user, sizeof(user));

        task("/home/onu/Documents/XMenu/test.txt", 1, 2, t_title,
             sizeof(t_title), t_body, sizeof(t_body), &task_pct);

        XClearWindow(dpy, win);
      }
      if (ev.type == ButtonPress) {
        int cx = ev.xbutton.x;
        int cy = ev.xbutton.y;
        if (cx >= btns.x && cx <= btns.x + btns.w && cy >= btns.y &&
            cy <= btns.y + btns.h) {
          system("playerctl -i chromium,firefox play-pause");
        }
      }
    }

    if (ev.type == LeaveNotify) {
      if (ev.xcrossing.mode == NotifyNormal &&
          ev.xcrossing.detail != NotifyInferior)
        break;
    }
    get_ram(ram_text, sizeof(ram_text), &ram_pct);
    get_battery(bat_cap, sizeof(bat_cap), &batt_pct);
    get_song(song, sizeof(song));
    get_hour(hour, sizeof(hour), 0);
    get_date(date, sizeof(date));
    get_user(user, sizeof(user));
    task("/home/onu/Documents/XMenu/test.txt", 1, 2, t_title, sizeof(t_title),
         t_body, sizeof(t_body), &task_pct);

    XClearWindow(dpy, win);

    draw_card(dpy, win, gc, xdraw, font, &title_color, background, foreground,
              &value_color, 10, 50, win_w / 2.3, 70, "RAM", ram_text, ram_pct);
    draw_card(dpy, win, gc, xdraw, font, &title_color, background, foreground,
              &value_color, win_w / 2.1, 50, win_w / 2, 70, "BATTERY", bat_cap,
              batt_pct);
    draw_player(dpy, win, gc, xdraw, font, &title_color, background, foreground,
                &value_color, 10, 130, win_w / 1.04, 120, last_art_url, &btns,
                visual, cmap);
    draw_card(dpy, win, gc, xdraw, font, &title_color, background, foreground,
              &value_color, 10, 260, win_w / 2, 70, t_title, t_body, task_pct);
    XSetForeground(dpy, gc, 0x000000);
    XFillRectangle(dpy, win, gc, 10, 10, win_w - 20, 30);
    draw_text(xdraw, font, &title_color, 10, 10, hour);
    draw_text(xdraw, font, &title_color, (win_w / 2) - sizeof(date), 10, date);
    draw_text(xdraw, font, &title_color, (win_w - 30) - sizeof(user), 10, user);

    XFlush(dpy);
    sleep(1);
  }

  XftColorFree(dpy, visual, cmap, &title_color);
  XftColorFree(dpy, visual, cmap, &value_color);
  XftDrawDestroy(xdraw);
  XftFontClose(dpy, font);
  XFreeGC(dpy, gc);
  XCloseDisplay(dpy);
  return 0;
}
