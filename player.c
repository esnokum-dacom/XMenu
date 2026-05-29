#include "player.h"
#include "draw.h"
#include "src/modules/sigr1.h"
#include <stdio.h>

const char *get_home() {
  const char *home = getenv("HOME");
  if (home)
    return home;
  struct passwd *pw = getpwuid(getuid());
  return pw ? pw->pw_dir : NULL;
}

void ndir(const char *path) {
  char tmp[512];
  strncpy(tmp, path, sizeof(tmp));

  for (char *p = tmp + 1; *p; p++) {
    if (*p == '/') {
      *p = '\0';
      mkdir(tmp, 0755);
      *p = '/';
    }
  }
  mkdir(tmp, 0755);
}

static void download_cover(const char *url) {
  char cmd[512];
  char dir[512];
  snprintf(dir, sizeof(dir), "%s/.cache/XMenu/covers", get_home());
  ndir(dir);
  snprintf(cmd, sizeof(cmd),
           "curl -s -o '%s/.cache/XMenu/covers/cover.jpg' '%s'", get_home(),
           url);
  system(cmd);
}

static void get_art_url(char *buf, int bufsz) {
  FILE *f = popen("playerctl -i chromium,firefox metadata mpris:artUrl", "r");
  if (!f) {
    buf[0] = '\0';
    return;
  }
  if (!fgets(buf, bufsz, f))
    buf[0] = '\0';
  else
    buf[strcspn(buf, "\n")] = '\0';
  pclose(f);
}

void get_song(char *buf, int bufsz) {
  FILE *f = popen("playerctl -i chromium,firefox metadata --format '{{artist}} "
                  "- {{title}}'",
                  "r");
  if (!f) {
    snprintf(buf, bufsz, "N/A");
    return;
  }
  if (!fgets(buf, bufsz, f))
    snprintf(buf, bufsz, "N/A");
  else {
    int len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
      buf[len - 1] = '\0';
  }
  pclose(f);
}

void get_time(char *buf, int bufsz) {
  FILE *f = popen("playerctl -i chromium,firefox metadata --format "
                  "'{{duration(position)}} - {{duration(mpris:length)}} '",
                  "r");
  if (!f) {
    snprintf(buf, bufsz, "N/A");
    return;
  }
  if (!fgets(buf, bufsz, f))
    snprintf(buf, bufsz, "N/A");
  else {
    int len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
      buf[len - 1] = '\0';
  }
  pclose(f);
}

static int get_progress_pct(void) {
  FILE *f = popen("playerctl -i chromium,firefox metadata --format "
                  "'{{position}} {{mpris:length}}'",
                  "r");
  if (!f)
    return 0;
  long pos = 0, len = 0;
  fscanf(f, "%ld %ld", &pos, &len);
  pclose(f);
  if (len <= 0)
    return 0;
  return (int)(pos * 100 / len);
}

void draw_cover(Display *dpy, Window win, int x, int y, int size,
                const char *art_url) {
  if (!art_url || strlen(art_url) == 0)
    return;

  char filepath[256];

  if (strncmp(art_url, "https://", 8) == 0 ||
      strncmp(art_url, "http://", 7) == 0) {
    download_cover(art_url);
    snprintf(filepath, sizeof(filepath), "%s/.cache/XMenu/covers/cover.jpg",
             get_home());
  } else if (strncmp(art_url, "file://", 7) == 0) {
    strncpy(filepath, art_url + 7, sizeof(filepath) - 1);
    filepath[sizeof(filepath) - 1] = '\0';
  } else {
    strncpy(filepath, art_url, sizeof(filepath) - 1);
    filepath[sizeof(filepath) - 1] = '\0';
  }

  imlib_context_set_display(dpy);
  imlib_context_set_visual(DefaultVisual(dpy, DefaultScreen(dpy)));
  imlib_context_set_colormap(DefaultColormap(dpy, DefaultScreen(dpy)));
  imlib_context_set_drawable(win);

  Imlib_Image img = imlib_load_image(filepath);
  if (!img)
    return;

  imlib_context_set_image(img);
  imlib_render_image_on_drawable_at_size(x, y, size, size);
  imlib_free_image();
}

static int is_playing(void) {
  FILE *f = popen("playerctl -i chromium,firefox status", "r");
  if (!f)
    return 0;
  char status[32];
  fscanf(f, "%31s", status);
  pclose(f);
  return strcmp(status, "Playing") == 0;
}

void draw_player(Display *dpy, Window win, GC gc, XftDraw *xdraw, XftFont *font,
                 XftColor *title_color, unsigned long foreground,
                 unsigned long background, XftColor *value_color, int x, int y,
                 int w, int h, char *last_art_url, PlayerButtons *btns,
                 Visual *visual, Colormap cmap) {

  ColorScheme col = {0};

  load_colors(dpy, &col);
  signal(SIGUSR1, handle_sigusr1);

  char song[256], time_act[256], art_url[256];

  get_song(song, sizeof(song));
  get_time(time_act, sizeof(time_act));
  get_art_url(art_url, sizeof(art_url));

  int line_h = font->ascent + font->descent + 2;
  int n_song = count_wrapped_lines(dpy, font, w - 20, song);
  int padding = 20;

  h = h + line_h + padding;

  XSetForeground(dpy, gc, foreground);
  XFillRectangle(dpy, win, gc, x, y, w, h);

  int cover_size = (h - 20) / 1.2;

  if (strcmp(art_url, last_art_url) != 0) {
    strncpy(last_art_url, art_url, 255);
    last_art_url[255] = '\0';

    if (strncmp(art_url, "https://", 8) == 0 ||
        strncmp(art_url, "http://", 7) == 0)
      download_cover(art_url);
  }

  draw_cover(dpy, win, x + 10, y + 10, cover_size, art_url);

  int text_x = x + 10 + cover_size + 10;
  XftDrawStringUtf8(xdraw, title_color, font, text_x, y + 20,
                    (FcChar8 *)"Listening", strlen("Listening"));
  // XftDrawStringUtf8(xdraw, value_color, font, text_x + 70, y + 50,
  //                  (FcChar8 *)song, strlen(song));

  char buf[1024];
  snprintf(buf, sizeof(buf), "%s", song);
  int ty = y + 50;
  draw_wrapped_text(dpy, xdraw, font, title_color, text_x + 70, ty, w - 220,
                    buf);

  XftDrawStringUtf8(xdraw, value_color, font, w / 2 + 50, y + 20,
                    (FcChar8 *)time_act, strlen(time_act));

  int btn_x = text_x;
  int btn_y = y + 38;
  int btn_w = 60;
  int btn_h = 60;

  unsigned long color = is_playing() ? col.colors[10] : col.colors[15];
  XSetForeground(dpy, gc, color);
  XFillRectangle(dpy, win, gc, btn_x, btn_y, btn_w, btn_h);

  XftColor color_t;
  int tm_x;

  if (is_playing()) {
    tm_x = 15;
    xcolor_to_xftcolor(dpy, visual, cmap, col.foreground, &color_t);
  } else {
    tm_x = 10;
    xcolor_to_xftcolor(dpy, visual, cmap, col.colors[2], &color_t);
  }

  const char *label = is_playing() ? " >" : "||";
  XftDrawStringUtf8(xdraw, &color_t, font, btn_x + (btn_w / 2) - tm_x,
                    btn_y + 35, (FcChar8 *)label, strlen(label));

  if (btns) {
    btns->x = btn_x;
    btns->y = btn_y;
    btns->w = btn_w;
    btns->h = btn_h;
  }

  int bar_x = x + 10;
  int bar_w = w - 20;
  int bar_h = 6;
  int bar_y = y + h - 20;
  int pct = get_progress_pct();

  XSetForeground(dpy, gc, col.colors[COLOR_BAR_F]);
  XFillRectangle(dpy, win, gc, bar_x, bar_y, bar_w, bar_h);

  XSetForeground(dpy, gc, col.colors[COLOR_BAR_UF]);
  XFillRectangle(dpy, win, gc, bar_x, bar_y, bar_w * pct / 100, bar_h);

  XftColorFree(dpy, visual, cmap, &color_t);
}
