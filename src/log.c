#include "log.h"
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>

static log_level_t current_level = LOG_DEBUG;
static bool logging_enabled = true;

static const char *level_to_str(log_level_t level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO";
        case LOG_WARN:  return "WARN";
        case LOG_ERROR: return "ERROR";
        default:        return "UNKNOWN";
    }
}

void log_set_level(log_level_t level) {
    current_level = level;
}

void log_enable(void) {
    logging_enabled = true;
}

void log_disable(void) {
    logging_enabled = false;
}

void log_log(log_level_t level, const char *file, int line, const char *fmt, ...) {
    if (!logging_enabled || level < current_level) return;

    // Get timestamp
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    char time_buf[20]; // "YYYY-MM-DD HH:MM:SS"
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    // Print log message
    fprintf(stderr, "[%s] [%s] %s:%d: ", time_buf, level_to_str(level), file, line);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
}
