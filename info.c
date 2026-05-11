#include "info.h"
#include <Imlib2.h>
#include <stdio.h>
#include <string.h>

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
