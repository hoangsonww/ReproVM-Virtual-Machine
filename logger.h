#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

// Log levels
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARN = 2,
    LOG_ERROR = 3,
    LOG_FATAL = 4
} LogLevel;

// Logger configuration
typedef struct {
    LogLevel min_level;
    FILE *output_file;
    int use_colors;
    int log_to_file;
    int log_to_console;
    char log_file_path[256];
} LoggerConfig;

// Global logger instance
extern LoggerConfig g_logger;

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_GRAY    "\033[90m"

// Initialize logger
void logger_init(const char *log_file_path, LogLevel min_level, int use_colors);

// Close logger
void logger_close(void);

// Log functions
void log_message(LogLevel level, const char *file, int line, const char *fmt, ...);

// Convenience macros
#define LOG_DEBUG(...) log_message(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  log_message(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  log_message(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_message(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) log_message(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

// Get log level name
const char* log_level_name(LogLevel level);

// Get log level color
const char* log_level_color(LogLevel level);

#endif // LOGGER_H
