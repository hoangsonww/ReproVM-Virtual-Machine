#include "logger.h"
#include <stdlib.h>
#include <pthread.h>

// Global logger instance
LoggerConfig g_logger = {
    .min_level = LOG_INFO,
    .output_file = NULL,
    .use_colors = 1,
    .log_to_file = 0,
    .log_to_console = 1,
    .log_file_path = ""
};

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void logger_init(const char *log_file_path, LogLevel min_level, int use_colors) {
    g_logger.min_level = min_level;
    g_logger.use_colors = use_colors;

    if (log_file_path && log_file_path[0] != '\0') {
        strncpy(g_logger.log_file_path, log_file_path, sizeof(g_logger.log_file_path) - 1);
        g_logger.output_file = fopen(log_file_path, "a");
        if (g_logger.output_file) {
            g_logger.log_to_file = 1;
        } else {
            fprintf(stderr, "Warning: Could not open log file: %s\n", log_file_path);
        }
    }
}

void logger_close(void) {
    if (g_logger.output_file) {
        fclose(g_logger.output_file);
        g_logger.output_file = NULL;
    }
}

const char* log_level_name(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO ";
        case LOG_WARN:  return "WARN ";
        case LOG_ERROR: return "ERROR";
        case LOG_FATAL: return "FATAL";
        default:        return "UNKNOWN";
    }
}

const char* log_level_color(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return COLOR_GRAY;
        case LOG_INFO:  return COLOR_GREEN;
        case LOG_WARN:  return COLOR_YELLOW;
        case LOG_ERROR: return COLOR_RED;
        case LOG_FATAL: return COLOR_MAGENTA;
        default:        return COLOR_RESET;
    }
}

void log_message(LogLevel level, const char *file, int line, const char *fmt, ...) {
    if (level < g_logger.min_level) {
        return;
    }

    pthread_mutex_lock(&log_mutex);

    // Get current time
    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    char time_buffer[64];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    // Extract filename from path
    const char *filename = strrchr(file, '/');
    filename = filename ? filename + 1 : file;

    // Prepare message
    va_list args;
    va_start(args, fmt);

    // Log to console
    if (g_logger.log_to_console) {
        if (g_logger.use_colors) {
            fprintf(stderr, "%s[%s] %s [%s:%d]%s ",
                    log_level_color(level),
                    time_buffer,
                    log_level_name(level),
                    filename,
                    line,
                    COLOR_RESET);
        } else {
            fprintf(stderr, "[%s] %s [%s:%d] ",
                    time_buffer,
                    log_level_name(level),
                    filename,
                    line);
        }
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    }

    // Log to file
    if (g_logger.log_to_file && g_logger.output_file) {
        va_list args_copy;
        va_copy(args_copy, args);

        fprintf(g_logger.output_file, "[%s] %s [%s:%d] ",
                time_buffer,
                log_level_name(level),
                filename,
                line);
        vfprintf(g_logger.output_file, fmt, args_copy);
        fprintf(g_logger.output_file, "\n");
        fflush(g_logger.output_file);

        va_end(args_copy);
    }

    va_end(args);
    pthread_mutex_unlock(&log_mutex);

    // Exit on fatal error
    if (level == LOG_FATAL) {
        logger_close();
        exit(EXIT_FAILURE);
    }
}
