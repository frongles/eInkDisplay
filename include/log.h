#ifndef LOG_H
#define LOG_H

#include <stdio.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} log_level_t;

void log_set_level(log_level_t level);
void log_enable(void);
void log_disable(void);

// You use this macro in code for logging
#define log_msg(level, fmt, ...) \
    log_log(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

// Internal logging function
void log_log(log_level_t level, const char *file, int line, const char *fmt, ...);

#endif // LOG_H
