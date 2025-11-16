#include "error_handling.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

// Global error context
ErrorContext g_last_error = {0};

void set_error(ErrorCode code, const char *file, int line, const char *func, const char *fmt, ...) {
    g_last_error.code = code;
    strncpy(g_last_error.file, file, sizeof(g_last_error.file) - 1);
    g_last_error.line = line;
    strncpy(g_last_error.function, func, sizeof(g_last_error.function) - 1);

    va_list args;
    va_start(args, fmt);
    vsnprintf(g_last_error.message, sizeof(g_last_error.message), fmt, args);
    va_end(args);

    LOG_ERROR("[%s] %s (in %s at %s:%d)",
              error_code_name(code),
              g_last_error.message,
              func, file, line);
}

ErrorContext* get_last_error(void) {
    return &g_last_error;
}

void clear_error(void) {
    memset(&g_last_error, 0, sizeof(ErrorContext));
}

void print_error(const ErrorContext *error) {
    if (error->code != ERR_SUCCESS) {
        fprintf(stderr, "Error [%s]: %s\n", error_code_name(error->code), error->message);
        fprintf(stderr, "  Location: %s() in %s:%d\n", error->function, error->file, error->line);
    }
}

const char* error_code_name(ErrorCode code) {
    switch (code) {
        case ERR_SUCCESS:          return "SUCCESS";
        case ERR_INVALID_ARG:      return "INVALID_ARGUMENT";
        case ERR_FILE_NOT_FOUND:   return "FILE_NOT_FOUND";
        case ERR_FILE_IO:          return "FILE_IO_ERROR";
        case ERR_MEMORY_ALLOC:     return "MEMORY_ALLOCATION_ERROR";
        case ERR_PARSE_ERROR:      return "PARSE_ERROR";
        case ERR_TASK_FAILED:      return "TASK_FAILED";
        case ERR_CYCLE_DETECTED:   return "CYCLE_DETECTED";
        case ERR_CACHE_ERROR:      return "CACHE_ERROR";
        case ERR_CAS_ERROR:        return "CAS_ERROR";
        case ERR_NETWORK_ERROR:    return "NETWORK_ERROR";
        case ERR_TIMEOUT:          return "TIMEOUT";
        case ERR_PERMISSION_DENIED:return "PERMISSION_DENIED";
        case ERR_INVALID_HASH:     return "INVALID_HASH";
        case ERR_CONFIG_ERROR:     return "CONFIG_ERROR";
        case ERR_SYSTEM_ERROR:     return "SYSTEM_ERROR";
        case ERR_UNKNOWN:          return "UNKNOWN_ERROR";
        default:                   return "UNDEFINED_ERROR";
    }
}

int retry_operation(RetryableFunc func, void *context, int max_attempts, int delay_ms) {
    int attempt = 0;
    int result = -1;

    while (attempt < max_attempts) {
        attempt++;
        LOG_DEBUG("Attempting operation (attempt %d/%d)", attempt, max_attempts);

        result = func(context);
        if (result == 0) {
            if (attempt > 1) {
                LOG_INFO("Operation succeeded after %d attempts", attempt);
            }
            return 0;
        }

        if (attempt < max_attempts) {
            LOG_WARN("Operation failed, retrying in %d ms... (attempt %d/%d)",
                     delay_ms, attempt, max_attempts);
            usleep(delay_ms * 1000);
        }
    }

    LOG_ERROR("Operation failed after %d attempts", max_attempts);
    return -1;
}
