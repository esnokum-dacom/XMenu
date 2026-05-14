#include "main.h"
#include "draw.h"
#include "info.h"
#include "player.h"
#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc >= 2 && strcmp(argv[1], "--new-task") == 0) {
    if (argc < 4) {
      printf("Use: XMenu --new-task <name-task> <task-body>\n");
      return 1;
    }

    char dir[128];
    snprintf(dir, sizeof(dir), "%s/.cache/XMenu", get_home());

    char flp[256];
    snprintf(flp, sizeof(flp), "%s/task.txt", dir);

    FILE *fp = fopen(flp, "w");
    if (fp == NULL) {
      printf("Error creating file.\n");
      return 1;
    }

    fprintf(fp, "%s\n", argv[2]);
    fprintf(fp, "%s\n", argv[3]);
    fclose(fp);
    printf("Task created.\n");
    return 0;

  } else if (argc >= 2 && strcmp(argv[1], "--task-complete") == 0) {
    if (argc < 3) {
      printf("Use: XMenu --task-complete <0|1>\n");
      return 1;
    }
    if (strcmp(argv[2], "0") != 0 && strcmp(argv[2], "1") != 0) {
      printf("Error: only 0 or 1 allowed.\n");
      return 1;
    }
    char dir[128];
    snprintf(dir, sizeof(dir), "%s/.cache/XMenu", get_home());
    char flp[256];
    snprintf(flp, sizeof(flp), "%s/task.txt", dir);

    FILE *fp = fopen(flp, "r");
    if (fp == NULL) {
      printf("Error handling file.\n");
      return 1;
    }
    char lines[16][256];
    int count = 0;
    while (count < 16 && fgets(lines[count], sizeof(lines[count]), fp))
      count++;
    fclose(fp);

    if (count < 3) {
      fp = fopen(flp, "a");
      if (fp == NULL) {
        printf("Error handling file.\n");
        return 1;
      }
      fprintf(fp, "%s\n", argv[2]);
      fclose(fp);
    } else {
      snprintf(lines[2], sizeof(lines[2]), "%s\n", argv[2]);
      fp = fopen(flp, "w");
      if (fp == NULL) {
        printf("Error handling file.\n");
        return 1;
      }
      for (int i = 0; i < count; i++)
        fputs(lines[i], fp);
      fclose(fp);
    }

    printf("Task marked as %s.\n",
           strcmp(argv[2], "1") == 0 ? "complete" : "incomplete");
    return 0;
  }

  else if (argc >= 2 && strcmp(argv[1], "--fetch") == 0) {
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
        .event_mask = ExposureMask | ButtonPressMask | PointerMotionMask |
                      LeaveWindowMask,
    };
    int win_w = 550;
    int win_h = 350;
    int win_x = mx + (mw - win_w) / 2;
    int win_y = my + (mh - win_h) / 2;

    Window win = XCreateWindow(
        dpy, root, win_x, win_y, win_w, win_h, 0, CopyFromParent, InputOutput,
        CopyFromParent, CWOverrideRedirect | CWBackPixel | CWEventMask, &attrs);

    XMapRaised(dpy, win);
    XSetInputFocus(dpy, win, RevertToParent, CurrentTime);

    GC gc = XCreateGC(dpy, win, 0, NULL);
    XftDraw *xdraw = XftDrawCreate(dpy, win, visual, cmap);

    XftColor title_color, value_color;
    XftColorAllocName(dpy, visual, cmap, "#ffffff", &title_color);
    XftColorAllocName(dpy, visual, cmap, "#ffffff", &value_color);

    char ram_text[64];
    char os_text[64];
    char kernel_text[64];
    char shell_text[64];
    char user[32];
    char hostname[32];
    char host[64];
    XEvent ev;

    char distro[64];

    char path[PATH_MAX];

    int ram_pct;

    while (1) {
      while (XPending(dpy)) {
        XNextEvent(dpy, &ev);
        if (ev.type == Expose) {
          get_user(user, sizeof(user));
          get_ram(ram_text, sizeof(ram_text), &ram_pct);
          get_os(os_text, sizeof(os_text));
          get_kernel(kernel_text, sizeof(kernel_text));
          get_shell(shell_text, sizeof(shell_text));
          get_hostname(hostname, sizeof(hostname));
          get_distro(distro, sizeof(distro));

          XClearWindow(dpy, win);
        }
      }

      if (ev.type == LeaveNotify) {
        if (ev.xcrossing.mode == NotifyNormal &&
            ev.xcrossing.detail != NotifyInferior)
          break;
      }
      get_user(user, sizeof(user));
      get_ram(ram_text, sizeof(ram_text), &ram_pct);
      get_os(os_text, sizeof(os_text));
      get_kernel(kernel_text, sizeof(kernel_text));
      get_shell(shell_text, sizeof(shell_text));
      get_hostname(hostname, sizeof(hostname));
      get_distro(distro, sizeof(distro));

      snprintf(host, sizeof(host), "%s@%s", user, hostname);

      XClearWindow(dpy, win);

      XSetForeground(dpy, gc, 0x000000);
      XFillRectangle(dpy, win, gc, 10, 10, win_w - 20, win_h - 20);

      if (strcmp(distro, "arch") == 0) {
        snprintf(path, sizeof(path), "%s/.cache/XMenu/src/arch.png",
                 get_home());
      } else if (strcmp(distro, "gentoo") == 0) {
        snprintf(path, sizeof(path), "%s/.cache/XMenu/src/gentoo.png",
                 get_home());
      } else if (strcmp(distro, "debian") == 0) {
        snprintf(path, sizeof(path), "%s/.cache/XMenu/src/debian.png",
                 get_home());
      } else if (strcmp(distro, "ubuntu") == 0) {
        snprintf(path, sizeof(path), "%s/.cache/XMenu/src/ubuntu.png",
                 get_home());
      } else {
        snprintf(path, sizeof(path), "%s/.cache/XMenu/src/linux.png",
                 get_home());
      }

      draw_cover(dpy, win, (win_w - 150) - 50, (win_h - 150) - 50, 150, path);

      draw_text(xdraw, font, &title_color, 10, 10, host);

      draw_text(xdraw, font, &title_color, 10, 28, "--------------");

      draw_text(xdraw, font, &title_color, 10, 45, "Memory: ");
      draw_text(xdraw, font, &title_color, 90, 45, ram_text);

      draw_text(xdraw, font, &title_color, 10, 70, "OS: ");
      draw_text(xdraw, font, &title_color, 90, 70, os_text);

      draw_text(xdraw, font, &title_color, 10, 95, "kernel: ");
      draw_text(xdraw, font, &title_color, 90, 95, kernel_text);

      draw_text(xdraw, font, &title_color, 10, 120, "Shell: ");
      draw_text(xdraw, font, &title_color, 90, 120, shell_text);

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

  else {
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
        .event_mask = ExposureMask | ButtonPressMask | PointerMotionMask |
                      LeaveWindowMask,
    };
    int win_w = 550;
    int win_h = 350;
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
    char task_dir[64];
    char song[32];
    PlayerButtons btns = {0};
    XEvent ev;

    int ram_pct;
    int batt_pct;
    int task_pct;

    snprintf(task_dir, sizeof(task_dir), "%s/.cache/XMenu/task.txt",
             get_home());

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

          task(task_dir, 1, 2, t_title, sizeof(t_title), t_body, sizeof(t_body),
               &task_pct);

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
      task(task_dir, 1, 2, t_title, sizeof(t_title), t_body, sizeof(t_body),
           &task_pct);

      XClearWindow(dpy, win);

      draw_card(dpy, win, gc, xdraw, font, &title_color, background, foreground,
                &value_color, 10, 50, win_w / 2.3, 70, "RAM", ram_text,
                ram_pct);
      draw_card(dpy, win, gc, xdraw, font, &title_color, background, foreground,
                &value_color, win_w / 2.1, 50, win_w / 2, 70, "BATTERY",
                bat_cap, batt_pct);
      draw_player(dpy, win, gc, xdraw, font, &title_color, background,
                  foreground, &value_color, 10, 130, win_w / 1.04, 120,
                  last_art_url, &btns, visual, cmap);
      draw_card(dpy, win, gc, xdraw, font, &title_color, background, foreground,
                &value_color, 10, 260, win_w / 2.3, 70, t_title, t_body,
                task_pct);
      XSetForeground(dpy, gc, 0x000000);
      XFillRectangle(dpy, win, gc, 10, 10, win_w - 20, 30);
      draw_text(xdraw, font, &title_color, 10, 10, hour);
      draw_text(xdraw, font, &title_color, (win_w / 2) - sizeof(date), 10,
                date);
      draw_text(xdraw, font, &title_color, (win_w - 30) - sizeof(user), 10,
                user);

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
}
