#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

// Error codes
typedef enum {
    ERR_SUCCESS = 0,
    ERR_INVALID_ARG = 1,
    ERR_FILE_NOT_FOUND = 2,
    ERR_FILE_IO = 3,
    ERR_MEMORY_ALLOC = 4,
    ERR_PARSE_ERROR = 5,
    ERR_TASK_FAILED = 6,
    ERR_CYCLE_DETECTED = 7,
    ERR_CACHE_ERROR = 8,
    ERR_CAS_ERROR = 9,
    ERR_NETWORK_ERROR = 10,
    ERR_TIMEOUT = 11,
    ERR_PERMISSION_DENIED = 12,
    ERR_INVALID_HASH = 13,
    ERR_CONFIG_ERROR = 14,
    ERR_SYSTEM_ERROR = 15,
    ERR_UNKNOWN = 99
} ErrorCode;

// Error context structure
typedef struct {
    ErrorCode code;
    char message[512];
    char file[256];
    int line;
    char function[128];
} ErrorContext;

// Global error context
extern ErrorContext g_last_error;

// Set error with context
void set_error(ErrorCode code, const char *file, int line, const char *func, const char *fmt, ...);

// Get last error
ErrorContext* get_last_error(void);

// Clear error
void clear_error(void);

// Print error
void print_error(const ErrorContext *error);

// Get error code name
const char* error_code_name(ErrorCode code);

// Retry mechanism
typedef int (*RetryableFunc)(void *context);
int retry_operation(RetryableFunc func, void *context, int max_attempts, int delay_ms);

// Macros for error handling
#define SET_ERROR(code, ...) set_error(code, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define CHECK_NULL(ptr, ...) \
    do { \
        if ((ptr) == NULL) { \
            SET_ERROR(ERR_MEMORY_ALLOC, __VA_ARGS__); \
            return -1; \
        } \
    } while(0)

#define CHECK_ERROR(expr, code, ...) \
    do { \
        if (!(expr)) { \
            SET_ERROR(code, __VA_ARGS__); \
            return -1; \
        } \
    } while(0)

#endif // ERROR_HANDLING_H
