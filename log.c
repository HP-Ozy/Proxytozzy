#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

static struct {
    LogLevel  min_level;
    FILE     *out;
} g_log = { LOG_INFO, NULL };

void log_init(LogLevel min_level, const char *filepath) {
    g_log.min_level = min_level;
    if (filepath && filepath[0] != '\0') {
        g_log.out = fopen(filepath, "a");
        if (!g_log.out) {
            fprintf(stderr, "[WARN] cannot open log file '%s', using stderr\n", filepath);
            g_log.out = stderr;
        }
    } else {
        g_log.out = stderr;
    }
}

void log_close(void) {
    if (g_log.out && g_log.out != stderr) {
        fclose(g_log.out);
    }
    g_log.out = stderr;
}

void log_write(LogLevel level, const char *src_file, int line,
               const char *fmt, ...) {
    if (level < g_log.min_level) return;

    static const char *labels[] = { "DEBUG", "INFO ", "WARN ", "ERROR" };

    time_t    now = time(NULL);
    struct tm tm_buf;
    localtime_r(&now, &tm_buf);

    char ts[24];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm_buf);

    FILE *out = g_log.out ? g_log.out : stderr;

    if (level == LOG_DEBUG) {
        /* src_file può essere un path lungo, stampa solo il basename */
        const char *base = strrchr(src_file, '/');
        base = base ? base + 1 : src_file;
        fprintf(out, "[%s] %s (%s:%d) ", ts, labels[level], base, line);
    } else {
        fprintf(out, "[%s] %s ", ts, labels[level]);
    }

    va_list ap;
    va_start(ap, fmt);
    vfprintf(out, fmt, ap);
    va_end(ap);

    fputc('\n', out);
    fflush(out);
}
