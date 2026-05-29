#include "main.h"
#include "draw.h"
#include "info.h"
#include "player.h"
#include "src/modules/sigr1.h"
#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <X11/extensions/Xrender.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

static int wait_event(Display *dpy) {
  if (XPending(dpy))
    return 1;
  int fd = ConnectionNumber(dpy);
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  struct timeval tv = {1, 0};
  return select(fd + 1, &fds, NULL, NULL, &tv) > 0;
}

Window create_win(Display *dpy, int win_w, int win_h) {
  int screen = DefaultScreen(dpy);
  Window root = DefaultRootWindow(dpy);
  int mx = 0, my = 0;
  int mw = DisplayWidth(dpy, screen);
  int mh = DisplayHeight(dpy, screen);

  if (XineramaIsActive(dpy)) {
    int n;
    XineramaScreenInfo *info = XineramaQueryScreens(dpy, &n);
    Window dw;
    int di;
    unsigned int du;
    int cx, cy;
    XQueryPointer(dpy, root, &dw, &dw, &cx, &cy, &di, &di, &du);
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
      .event_mask = ExposureMask | ButtonPressMask | LeaveWindowMask,
  };

  return XCreateWindow(dpy, root, mx + (mw - win_w) / 2, my + (mh - win_h) / 2,
                       win_w, win_h, 0, CopyFromParent, InputOutput,
                       CopyFromParent,
                       CWOverrideRedirect | CWBackPixel | CWEventMask, &attrs);
}

static int cmd_new_task(int argc, char *argv[]) {
  if (argc < 4) {
    printf("Use: XMenu --new-task <name> <body>\n");
    return 1;
  }
  char flp[256];
  snprintf(flp, sizeof(flp), "%s/.cache/XMenu/task.txt", get_home());
  FILE *fp = fopen(flp, "w");
  if (!fp) {
    printf("Error creating file.\n");
    return 1;
  }
  fprintf(fp, "%s\n%s\n", argv[2], argv[3]);
  fclose(fp);
  printf("Task created.\n");
  return 0;
}

static int cmd_task_complete(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Use: XMenu --task-complete <0|1>\n");
    return 1;
  }
  if (strcmp(argv[2], "0") && strcmp(argv[2], "1")) {
    printf("Error: only 0 or 1 allowed.\n");
    return 1;
  }

  char flp[256];
  snprintf(flp, sizeof(flp), "%s/.cache/XMenu/task.txt", get_home());
  FILE *fp = fopen(flp, "r");
  if (!fp) {
    printf("Error handling file.\n");
    return 1;
  }

  char lines[16][256];
  int count = 0;
  while (count < 16 && fgets(lines[count], sizeof(lines[count]), fp))
    count++;
  fclose(fp);

  fp = fopen(flp, count < 3 ? "a" : "w");
  if (!fp) {
    printf("Error handling file.\n");
    return 1;
  }
  if (count < 3) {
    fprintf(fp, "%s\n", argv[2]);
  } else {
    snprintf(lines[2], sizeof(lines[2]), "%s\n", argv[2]);
    for (int i = 0; i < count; i++)
      fputs(lines[i], fp);
  }
  fclose(fp);
  printf("Task marked as %s.\n",
         strcmp(argv[2], "1") == 0 ? "complete" : "incomplete");
  return 0;
}

static int run_fetch(Display *dpy, Window win, int win_w, int win_h) {
  int screen = DefaultScreen(dpy);
  Visual *vis = DefaultVisual(dpy, screen);
  Colormap cmap = DefaultColormap(dpy, screen);
  XftFont *font = XftFontOpenName(dpy, screen, "monospace:size=12");

  ColorScheme col = {0};

  XMapRaised(dpy, win);
  XSetInputFocus(dpy, win, RevertToParent, CurrentTime);

  GC gc = XCreateGC(dpy, win, 0, NULL);

  Pixmap buf = XCreatePixmap(dpy, win, win_w, win_h, DefaultDepth(dpy, screen));
  XftDraw *xdraw = XftDrawCreate(dpy, buf, vis, cmap);

  load_colors(dpy, &col);
  signal(SIGUSR1, handle_sigusr1);

  XftColor color;

  char ram_text[64], os_text[64], kernel_text[64];
  char shell_text[64], user[32], hostname[32], host[64];
  char distro[64], path[PATH_MAX];

  int ram_pct;
  XEvent ev = {0};

  int running = 1;
  int first = 1;

  while (running) {
    if (!first)
      wait_event(dpy);
    first = 0;

    while (XPending(dpy)) {
      XNextEvent(dpy, &ev);
      if (ev.type == LeaveNotify && ev.xcrossing.mode == NotifyNormal &&
          ev.xcrossing.detail != NotifyInferior)
        running = 0;
    }
    if (!running)
      break;

    xcolor_to_xftcolor(dpy, vis, cmap, col.foreground, &color);

    get_user(user, sizeof(user));
    get_ram(ram_text, sizeof(ram_text), &ram_pct);
    get_os(os_text, sizeof(os_text));
    get_kernel(kernel_text, sizeof(kernel_text));
    get_shell(shell_text, sizeof(shell_text));
    get_hostname(hostname, sizeof(hostname));
    get_distro(distro, sizeof(distro));
    snprintf(host, sizeof(host), "%s@%s", user, hostname);

    const char *logo = "linux";
    if (!strcmp(distro, "arch"))
      logo = "arch";
    else if (!strcmp(distro, "gentoo"))
      logo = "gentoo";
    else if (!strcmp(distro, "debian"))
      logo = "debian";
    else if (!strcmp(distro, "ubuntu"))
      logo = "ubuntu";
    snprintf(path, sizeof(path), "%s/.cache/XMenu/src/%s.png", get_home(),
             logo);

    XSetForeground(dpy, gc, col.colors[1]);
    XFillRectangle(dpy, buf, gc, 0, 0, win_w, win_h);
    XSetForeground(dpy, gc, col.background);
    XFillRectangle(dpy, buf, gc, 10, 10, win_w - 20, win_h - 20);

    draw_cover(dpy, buf, (win_w - 150) - 50, (win_h - 150) - 50, 150, path);
    draw_text(dpy, xdraw, font, &color, 20, 30, host, 0);
    draw_text(dpy, xdraw, font, &color, 20, 48, "--------------", 0);
    draw_text(dpy, xdraw, font, &color, 20, 65, "Memory: ", 0);
    draw_text(dpy, xdraw, font, &color, 100, 65, ram_text, 0);
    draw_text(dpy, xdraw, font, &color, 20, 90, "OS: ", 0);
    draw_text(dpy, xdraw, font, &color, 100, 90, os_text, 0);
    draw_text(dpy, xdraw, font, &color, 20, 115, "Kernel: ", 0);
    draw_text(dpy, xdraw, font, &color, 100, 115, kernel_text, 0);
    draw_text(dpy, xdraw, font, &color, 20, 140, "Shell: ", 0);
    draw_text(dpy, xdraw, font, &color, 100, 140, shell_text, 0);

    XCopyArea(dpy, buf, win, gc, 0, 0, win_w, win_h, 0, 0);
    XFlush(dpy);
  }

  XftColorFree(dpy, vis, cmap, &color);
  XftDrawDestroy(xdraw);
  XFreePixmap(dpy, buf);
  XftFontClose(dpy, font);
  XFreeGC(dpy, gc);
  return 0;
}

static int run_default(Display *dpy, Window win, int win_w, int win_h) {
  int screen = DefaultScreen(dpy);
  Visual *vis = DefaultVisual(dpy, screen);
  Colormap cmap = DefaultColormap(dpy, screen);
  XftFont *font = XftFontOpenName(dpy, screen, "monospace:size=12");
  ColorScheme col = {0};

  XMapRaised(dpy, win);
  XSetInputFocus(dpy, win, RevertToParent, CurrentTime);

  GC gc = XCreateGC(dpy, win, 0, NULL);

  Pixmap buf = XCreatePixmap(dpy, win, win_w, win_h, DefaultDepth(dpy, screen));
  XftDraw *xdraw = XftDrawCreate(dpy, buf, vis, cmap);

  load_colors(dpy, &col);
  signal(SIGUSR1, handle_sigusr1);

  XftColor title_color, value_color;

  char ram_text[64], bat_cap[32], hour[64], date_str[64];
  char user[32], t_title[2048], t_body[2048], song[32], test[16];
  char task_dir[256];
  char last_art_url[256] = {0};
  PlayerButtons btns = {0};
  XEvent ev = {0};

  int ram_pct, batt_pct, task_pct;
  snprintf(task_dir, sizeof(task_dir), "%s/.cache/XMenu/task.txt", get_home());
  snprintf(test, sizeof(test), "Hello world");

  int color_block = col.colors[COLOR_BLOCK];

  int running = 1;
  int first = 1;
  while (running) {
    if (!first)
      wait_event(dpy);
    first = 0;

    while (XPending(dpy)) {
      XNextEvent(dpy, &ev);
      if (ev.type == ButtonPress) {
        int cx = ev.xbutton.x, cy = ev.xbutton.y;
        if (cx >= btns.x && cx <= btns.x + btns.w && cy >= btns.y &&
            cy <= btns.y + btns.h)
          system("playerctl -i chromium,firefox play-pause");
      }
      if (ev.type == LeaveNotify && ev.xcrossing.mode == NotifyNormal &&
          ev.xcrossing.detail != NotifyInferior)
        running = 0;
    }
    if (!running)
      break;

    get_ram(ram_text, sizeof(ram_text), &ram_pct);
    get_battery(bat_cap, sizeof(bat_cap), &batt_pct);
    get_song(song, sizeof(song));
    get_hour(hour, sizeof(hour), 0);
    get_date(date_str, sizeof(date_str));
    get_user(user, sizeof(user));
    task(task_dir, 1, 2, t_title, sizeof(t_title), t_body, sizeof(t_body),
         &task_pct);

    int line_h = font->ascent + font->descent + 2;
    int padding = 20;

    xcolor_to_xftcolor(dpy, vis, cmap, col.foreground, &title_color);
    xcolor_to_xftcolor(dpy, vis, cmap, col.foreground, &value_color);

    XSetForeground(dpy, gc, col.background);
    XFillRectangle(dpy, buf, gc, 0, 0, win_w, win_h);

    draw_card(dpy, buf, gc, xdraw, font, &title_color, color_block,
              col.foreground, &value_color, 10, 50, win_w / 2.3, 70, "RAM",
              ram_text, ram_pct);
    draw_card(dpy, buf, gc, xdraw, font, &title_color, color_block,
              col.foreground, &value_color, win_w / 2.1, 50, win_w / 2, 70,
              "BATTERY", bat_cap, batt_pct);
    draw_player(dpy, buf, gc, xdraw, font, &title_color, color_block,
                col.foreground, &value_color, 10, 130, win_w / 1.04, 120,
                last_art_url, &btns, vis, cmap);
    draw_card(dpy, buf, gc, xdraw, font, &title_color, color_block,
              col.foreground, &value_color, 10, 260 + line_h + padding,
              win_w - 20, 95, t_title, t_body, task_pct);

    XSetForeground(dpy, gc, color_block);
    XFillRectangle(dpy, buf, gc, 10, 10, win_w - 23, 30);
    draw_text(dpy, xdraw, font, &title_color, 20, 31, hour, 0);
    draw_text(dpy, xdraw, font, &title_color, (win_w / 2) + 30, 31, date_str,
              1);

    draw_text(dpy, xdraw, font, &title_color, win_w - 20, 31, user, 1);

    XCopyArea(dpy, buf, win, gc, 0, 0, win_w, win_h, 0, 0);
    XFlush(dpy);
  }

  XftColorFree(dpy, vis, cmap, &title_color);
  XftColorFree(dpy, vis, cmap, &value_color);
  XftDrawDestroy(xdraw);
  XFreePixmap(dpy, buf);
  XftFontClose(dpy, font);
  XFreeGC(dpy, gc);
  return 0;
}

int 
main(int argc, char *argv[]) {
  Display *dpy = XOpenDisplay(NULL);
  if (!dpy) {
    fprintf(stderr, "Could not open display\n");
    return 1;
  }

  if (argc >= 2 && strcmp(argv[1], "--new-task") == 0)
    return cmd_new_task(argc, argv);
  if (argc >= 2 && strcmp(argv[1], "--task-complete") == 0)
    return cmd_task_complete(argc, argv);

  int win_w = 550, win_h = 450;
  Window win = create_win(dpy, win_w, win_h);

  int ret;
  if (argc >= 2 && strcmp(argv[1], "--fetch") == 0)
    ret = run_fetch(dpy, win, win_w, win_h);
  else
    ret = run_default(dpy, win, win_w, win_h);

  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
  return ret;
}
