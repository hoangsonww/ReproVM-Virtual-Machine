#ifndef METRICS_H
#define METRICS_H

#include <time.h>
#include <stdint.h>

// Metric types
typedef struct {
    uint64_t total_tasks;
    uint64_t tasks_executed;
    uint64_t tasks_cached;
    uint64_t tasks_failed;
    uint64_t cache_hits;
    uint64_t cache_misses;
    double cache_hit_rate;

    // Timing metrics (in milliseconds)
    double total_execution_time_ms;
    double avg_task_time_ms;
    double min_task_time_ms;
    double max_task_time_ms;

    // Storage metrics
    uint64_t cas_objects_count;
    uint64_t cas_total_size_bytes;
    uint64_t cache_entries_count;

    // Resource metrics
    double cpu_usage_percent;
    uint64_t memory_usage_bytes;
    uint64_t peak_memory_bytes;

    // Error metrics
    uint64_t retry_count;
    uint64_t timeout_count;
    uint64_t error_count;

    // Timestamp
    time_t start_time;
    time_t end_time;
    double elapsed_seconds;
} Metrics;

// Task timing structure
typedef struct {
    char task_name[256];
    double start_time;
    double duration_ms;
    int cached;
    int failed;
} TaskTiming;

// Global metrics instance
extern Metrics g_metrics;

// Initialize metrics
void metrics_init(void);

// Record task start
void metrics_task_start(const char *task_name);

// Record task end
void metrics_task_end(const char *task_name, int cached, int failed, double duration_ms);

// Record cache hit/miss
void metrics_record_cache_hit(void);
void metrics_record_cache_miss(void);

// Record error
void metrics_record_error(void);
void metrics_record_retry(void);
void metrics_record_timeout(void);

// Update resource metrics
void metrics_update_resources(void);

// Calculate final statistics
void metrics_calculate_stats(void);

// Print metrics summary
void metrics_print_summary(void);

// Write metrics to file
int metrics_write_to_file(const char *filename);

// Write metrics in JSON format
int metrics_write_json(const char *filename);

// Get current time in milliseconds
double get_time_ms(void);

// Get memory usage
uint64_t get_memory_usage(void);

// Get CPU usage
double get_cpu_usage(void);

#endif // METRICS_H
