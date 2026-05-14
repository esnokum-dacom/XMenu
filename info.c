#include "info.h"
#include <Imlib2.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void get_battery(char *buf, int bufsz, int *pct) {
  int cap = -1;
  char raw[32] = "N/A";

  FILE *f = fopen("/sys/class/power_supply/BAT0/capacity", "r");
  if (f) {
    fscanf(f, "%d", &cap);
    fclose(f);
  }

  f = fopen("/sys/class/power_supply/BAT0/status", "r");
  if (f) {
    fscanf(f, "%31s", raw);
    fclose(f);
  }

  const char *estado;
  if (strcmp(raw, "Charging") == 0)
    estado = "Charging";
  else if (strcmp(raw, "Discharging") == 0)
    estado = "Discharging";
  else if (strcmp(raw, "Full") == 0)
    estado = "Full";
  else
    estado = raw;

  *pct = cap >= 0 ? cap : 0; /* la batería ya es 0-100 directamente */

  if (cap >= 0)
    snprintf(buf, bufsz, "%d%% | %s", cap, estado);
  else
    snprintf(buf, bufsz, "N/A");
}

void get_ram(char *buf, int bufsz, int *pct) {
  FILE *f = fopen("/proc/meminfo", "r");
  if (!f) {
    snprintf(buf, bufsz, "N/A");
    *pct = 0;
    return;
  }

  long total = 0, available = 0;
  char line[128];
  while (fgets(line, sizeof(line), f)) {
    if (strncmp(line, "MemTotal:", 9) == 0)
      sscanf(line, "MemTotal: %ld", &total);
    else if (strncmp(line, "MemAvailable:", 13) == 0)
      sscanf(line, "MemAvailable: %ld", &available);
  }
  fclose(f);

  long used = total - available;
  *pct = total > 0 ? (int)(used * 100 / total) : 0;
  snprintf(buf, bufsz, "%ld MB / %ld MB", used / 1024, total / 1024);
}

void get_hour(char *buf, int bufsz, int sec) {
  int secon = sec;

  time_t now = time(NULL);
  struct tm *tm_struct = localtime(&now);
  int hour = tm_struct->tm_hour;
  int minutes = tm_struct->tm_min;
  int seconds = tm_struct->tm_sec;

  if (secon == 0)
    snprintf(buf, bufsz, "%02d:%02d", hour, minutes);
  else if (secon == 1)
    snprintf(buf, bufsz, "%02d:%02d:%02d", hour, minutes, seconds);
}

void get_date(char *buf, int bufsz) {
  time_t t = time(NULL);
  struct tm *currentTime = localtime(&t);

  int d = currentTime->tm_mday;
  int m = currentTime->tm_mon + 1;
  int y = currentTime->tm_year + 1900;

  snprintf(buf, bufsz, "%02d/%02d/%04d", d, m, y);
}

const char *get_username() {
  uid_t uid = getuid();

  struct passwd *pw = getpwuid(uid);

  if (pw != NULL) {
    return pw->pw_name;
  }
  return NULL;
}

void get_user(char *buf, int bufsz) {
  const char *username = get_username();
  if (username) {
    snprintf(buf, bufsz, "%s", username);
  } else {
    perror("Failed to get username");
  }
}

void task(const char *path, int title, int body, char *buft, int bufszt,
          char *bufb, int bufszb, int *pct) {
  FILE *f = fopen(path, "r");
  if (!f) {
    snprintf(buft, bufszt, "No task yet");
    snprintf(bufb, bufszb, "Use --new-task");
    *pct = 0;
    return;
  }
  char linea[256];
  int num = 0;
  *pct = 0;
  char lt[64] = {0}, lb[64] = {0}, cl[32] = {0};
  int maxline = title > body ? title : body;
  maxline = maxline > 3 ? maxline : 3;

  while (fgets(linea, sizeof(linea), f)) {
    num++;
    linea[strcspn(linea, "\n")] = '\0';
    if (num == title)
      strcpy(lt, linea);
    if (num == body)
      strcpy(lb, linea);
    if (num == 3)
      strcpy(cl, linea);
    if (num >= maxline)
      break;
  }
  fclose(f);

  *pct = atoi(cl) ? 100 : 0;

  snprintf(buft, bufszt, "task: %s", lt);
  snprintf(bufb, bufszb, "%s", lb);
}
