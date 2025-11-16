#ifndef HEALTH_CHECK_H
#define HEALTH_CHECK_H

#include <time.h>

// Health status
typedef enum {
    HEALTH_OK = 0,
    HEALTH_WARNING = 1,
    HEALTH_CRITICAL = 2,
    HEALTH_UNKNOWN = 3
} HealthStatus;

// Health check result
typedef struct {
    HealthStatus status;
    char message[256];
    time_t timestamp;
    double response_time_ms;
} HealthCheckResult;

// System health information
typedef struct {
    HealthStatus overall_status;
    HealthCheckResult cache_health;
    HealthCheckResult cas_health;
    HealthCheckResult disk_health;
    HealthCheckResult memory_health;

    // System info
    double cpu_usage;
    uint64_t memory_usage_bytes;
    uint64_t disk_usage_bytes;
    uint64_t disk_available_bytes;

    time_t last_check;
} SystemHealth;

// Global health status
extern SystemHealth g_system_health;

// Initialize health check system
void health_init(void);

// Perform all health checks
void health_check_all(void);

// Individual health checks
HealthCheckResult health_check_cache(void);
HealthCheckResult health_check_cas(void);
HealthCheckResult health_check_disk(void);
HealthCheckResult health_check_memory(void);

// Get overall health status
HealthStatus health_get_overall_status(void);

// Print health report
void health_print_report(void);

// Write health status to file
int health_write_to_file(const char *filename);

// Write health status as JSON
int health_write_json(const char *filename);

// Get health status name
const char* health_status_name(HealthStatus status);

#endif // HEALTH_CHECK_H
