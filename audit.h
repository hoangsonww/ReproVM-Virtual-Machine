#ifndef AUDIT_H
#define AUDIT_H

#include <time.h>

// Audit event types
typedef enum {
    AUDIT_TASK_START,
    AUDIT_TASK_COMPLETE,
    AUDIT_TASK_FAIL,
    AUDIT_CACHE_HIT,
    AUDIT_CACHE_MISS,
    AUDIT_CACHE_STORE,
    AUDIT_CONFIG_LOAD,
    AUDIT_CONFIG_CHANGE,
    AUDIT_SECURITY_VIOLATION,
    AUDIT_PERMISSION_DENIED,
    AUDIT_RESOURCE_LIMIT,
    AUDIT_SYSTEM_START,
    AUDIT_SYSTEM_STOP,
    AUDIT_ERROR,
    AUDIT_WARNING
} AuditEventType;

// Audit event
typedef struct {
    AuditEventType type;
    time_t timestamp;
    char *user;
    char *process;
    char *resource;
    char *action;
    char *details;
    int success;
    char *error_message;
} AuditEvent;

// Audit logger
typedef struct {
    FILE *log_file;
    int enabled;
    int log_to_console;
    int log_to_syslog;
    char log_file_path[512];
} AuditLogger;

// Global audit logger
extern AuditLogger *g_audit_logger;

// Initialize audit logging
int audit_init(const char *log_file_path);

// Log audit event
void audit_log_event(AuditEventType type, const char *resource,
                     const char *action, int success, const char *details);

// Specialized audit functions
void audit_task_start(const char *task_name);
void audit_task_complete(const char *task_name, int success);
void audit_cache_operation(const char *operation, const char *hash, int success);
void audit_security_event(const char *event, const char *details);
void audit_config_change(const char *key, const char *old_value, const char *new_value);

// Audit queries
int audit_get_events_by_type(AuditEventType type, time_t start, time_t end,
                             AuditEvent **events, int *count);
int audit_get_events_by_resource(const char *resource, time_t start, time_t end,
                                 AuditEvent **events, int *count);

// Generate audit report
void audit_generate_report(const char *output_file, time_t start, time_t end);

// Close audit logging
void audit_close(void);

// Get event type name
const char* audit_event_type_name(AuditEventType type);

#endif // AUDIT_H
