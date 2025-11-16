#include "audit.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

AuditLogger *g_audit_logger = NULL;

const char* audit_event_type_name(AuditEventType type) {
    switch (type) {
        case AUDIT_TASK_START:        return "TASK_START";
        case AUDIT_TASK_COMPLETE:     return "TASK_COMPLETE";
        case AUDIT_TASK_FAIL:          return "TASK_FAIL";
        case AUDIT_CACHE_HIT:          return "CACHE_HIT";
        case AUDIT_CACHE_MISS:         return "CACHE_MISS";
        case AUDIT_CACHE_STORE:        return "CACHE_STORE";
        case AUDIT_CONFIG_LOAD:        return "CONFIG_LOAD";
        case AUDIT_CONFIG_CHANGE:      return "CONFIG_CHANGE";
        case AUDIT_SECURITY_VIOLATION: return "SECURITY_VIOLATION";
        case AUDIT_PERMISSION_DENIED:  return "PERMISSION_DENIED";
        case AUDIT_RESOURCE_LIMIT:     return "RESOURCE_LIMIT";
        case AUDIT_SYSTEM_START:       return "SYSTEM_START";
        case AUDIT_SYSTEM_STOP:        return "SYSTEM_STOP";
        case AUDIT_ERROR:              return "ERROR";
        case AUDIT_WARNING:            return "WARNING";
        default:                       return "UNKNOWN";
    }
}

int audit_init(const char *log_file_path) {
    g_audit_logger = calloc(1, sizeof(AuditLogger));
    g_audit_logger->enabled = 1;
    g_audit_logger->log_to_console = 0;
    g_audit_logger->log_to_syslog = 0;

    if (log_file_path) {
        strncpy(g_audit_logger->log_file_path, log_file_path,
                sizeof(g_audit_logger->log_file_path) - 1);
        g_audit_logger->log_file = fopen(log_file_path, "a");
        if (!g_audit_logger->log_file) {
            LOG_ERROR("Failed to open audit log file: %s", log_file_path);
            return -1;
        }
    }

    // Open syslog
    openlog("reprovm", LOG_PID, LOG_USER);
    g_audit_logger->log_to_syslog = 1;

    LOG_INFO("Audit logging initialized: %s", log_file_path);
    audit_log_event(AUDIT_SYSTEM_START, "reprovm", "start", 1, "Audit system started");

    return 0;
}

void audit_log_event(AuditEventType type, const char *resource,
                     const char *action, int success, const char *details) {
    if (!g_audit_logger || !g_audit_logger->enabled) return;

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    char user[256];
    getlogin_r(user, sizeof(user));

    // Log to file
    if (g_audit_logger->log_file) {
        fprintf(g_audit_logger->log_file,
                "[%s] type=%s user=%s resource=%s action=%s success=%d details=%s\n",
                timestamp, audit_event_type_name(type), user,
                resource ? resource : "N/A",
                action ? action : "N/A",
                success,
                details ? details : "N/A");
        fflush(g_audit_logger->log_file);
    }

    // Log to syslog
    if (g_audit_logger->log_to_syslog) {
        int priority = success ? LOG_INFO : LOG_WARNING;
        if (type == AUDIT_SECURITY_VIOLATION || type == AUDIT_PERMISSION_DENIED) {
            priority = LOG_ERR;
        }

        syslog(priority, "[AUDIT] type=%s user=%s resource=%s action=%s success=%d details=%s",
               audit_event_type_name(type), user,
               resource ? resource : "N/A",
               action ? action : "N/A",
               success,
               details ? details : "N/A");
    }

    // Log to console if enabled
    if (g_audit_logger->log_to_console) {
        printf("[AUDIT] [%s] %s: %s - %s (%s)\n",
               timestamp,
               audit_event_type_name(type),
               resource ? resource : "N/A",
               action ? action : "N/A",
               success ? "SUCCESS" : "FAILURE");
    }
}

void audit_task_start(const char *task_name) {
    audit_log_event(AUDIT_TASK_START, task_name, "execute", 1,
                    "Task execution started");
}

void audit_task_complete(const char *task_name, int success) {
    audit_log_event(success ? AUDIT_TASK_COMPLETE : AUDIT_TASK_FAIL,
                    task_name, "execute", success,
                    success ? "Task completed successfully" : "Task failed");
}

void audit_cache_operation(const char *operation, const char *hash, int success) {
    char details[512];
    snprintf(details, sizeof(details), "Cache %s for hash %s",
             operation, hash ? hash : "unknown");

    AuditEventType type = AUDIT_CACHE_MISS;
    if (strcmp(operation, "hit") == 0) type = AUDIT_CACHE_HIT;
    else if (strcmp(operation, "store") == 0) type = AUDIT_CACHE_STORE;

    audit_log_event(type, hash, operation, success, details);
}

void audit_security_event(const char *event, const char *details) {
    audit_log_event(AUDIT_SECURITY_VIOLATION, event, "security_check", 0, details);
    LOG_WARN("Security event: %s - %s", event, details);
}

void audit_config_change(const char *key, const char *old_value, const char *new_value) {
    char details[512];
    snprintf(details, sizeof(details), "Config changed: %s = %s -> %s",
             key, old_value ? old_value : "null", new_value ? new_value : "null");
    audit_log_event(AUDIT_CONFIG_CHANGE, key, "update", 1, details);
}

void audit_generate_report(const char *output_file, time_t start, time_t end) {
    FILE *report = fopen(output_file, "w");
    if (!report) {
        LOG_ERROR("Failed to open audit report file: %s", output_file);
        return;
    }

    fprintf(report, "=== Audit Report ===\n");
    fprintf(report, "Generated: %s", ctime(&end));
    fprintf(report, "Period: %s to %s\n", ctime(&start), ctime(&end));
    fprintf(report, "========================\n\n");

    // This would read from the audit log file and generate statistics
    // For now, just a placeholder
    fprintf(report, "See audit log at: %s\n", g_audit_logger->log_file_path);

    fclose(report);
    LOG_INFO("Audit report generated: %s", output_file);
}

void audit_close(void) {
    if (!g_audit_logger) return;

    audit_log_event(AUDIT_SYSTEM_STOP, "reprovm", "stop", 1, "Audit system stopped");

    if (g_audit_logger->log_file) {
        fclose(g_audit_logger->log_file);
    }

    if (g_audit_logger->log_to_syslog) {
        closelog();
    }

    free(g_audit_logger);
    g_audit_logger = NULL;
}
