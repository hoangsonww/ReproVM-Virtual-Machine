#include "metrics.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

// Global metrics instance
Metrics g_metrics;

// Array to store task timings
#define MAX_TASK_TIMINGS 1000
static TaskTiming task_timings[MAX_TASK_TIMINGS];
static int task_timing_count = 0;

void metrics_init(void) {
    memset(&g_metrics, 0, sizeof(Metrics));
    memset(task_timings, 0, sizeof(task_timings));
    task_timing_count = 0;

    g_metrics.start_time = time(NULL);
    g_metrics.min_task_time_ms = 999999999.0;
    g_metrics.max_task_time_ms = 0.0;

    LOG_DEBUG("Metrics system initialized");
}

double get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

uint64_t get_memory_usage(void) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // ru_maxrss is in kilobytes on Linux, bytes on macOS
        #ifdef __APPLE__
        return (uint64_t)usage.ru_maxrss;
        #else
        return (uint64_t)usage.ru_maxrss * 1024;
        #endif
    }
    return 0;
}

double get_cpu_usage(void) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        double user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
        double sys_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;
        return (user_time + sys_time) * 100.0;
    }
    return 0.0;
}

void metrics_task_start(const char *task_name) {
    if (task_timing_count < MAX_TASK_TIMINGS) {
        TaskTiming *timing = &task_timings[task_timing_count];
        strncpy(timing->task_name, task_name, sizeof(timing->task_name) - 1);
        timing->start_time = get_time_ms();
        timing->cached = 0;
        timing->failed = 0;
        timing->duration_ms = 0.0;
    }
}

void metrics_task_end(const char *task_name, int cached, int failed, double duration_ms) {
    g_metrics.total_tasks++;

    if (cached) {
        g_metrics.tasks_cached++;
    } else {
        g_metrics.tasks_executed++;
    }

    if (failed) {
        g_metrics.tasks_failed++;
    }

    g_metrics.total_execution_time_ms += duration_ms;

    if (duration_ms < g_metrics.min_task_time_ms) {
        g_metrics.min_task_time_ms = duration_ms;
    }
    if (duration_ms > g_metrics.max_task_time_ms) {
        g_metrics.max_task_time_ms = duration_ms;
    }

    // Store timing
    if (task_timing_count < MAX_TASK_TIMINGS) {
        TaskTiming *timing = &task_timings[task_timing_count];
        timing->duration_ms = duration_ms;
        timing->cached = cached;
        timing->failed = failed;
        task_timing_count++;
    }

    LOG_DEBUG("Task '%s' completed: duration=%.2fms, cached=%d, failed=%d",
              task_name, duration_ms, cached, failed);
}

void metrics_record_cache_hit(void) {
    g_metrics.cache_hits++;
}

void metrics_record_cache_miss(void) {
    g_metrics.cache_misses++;
}

void metrics_record_error(void) {
    g_metrics.error_count++;
}

void metrics_record_retry(void) {
    g_metrics.retry_count++;
}

void metrics_record_timeout(void) {
    g_metrics.timeout_count++;
}

void metrics_update_resources(void) {
    uint64_t mem = get_memory_usage();
    g_metrics.memory_usage_bytes = mem;
    if (mem > g_metrics.peak_memory_bytes) {
        g_metrics.peak_memory_bytes = mem;
    }
    g_metrics.cpu_usage_percent = get_cpu_usage();
}

void metrics_calculate_stats(void) {
    g_metrics.end_time = time(NULL);
    g_metrics.elapsed_seconds = difftime(g_metrics.end_time, g_metrics.start_time);

    if (g_metrics.total_tasks > 0) {
        g_metrics.avg_task_time_ms = g_metrics.total_execution_time_ms / g_metrics.total_tasks;
    }

    uint64_t total_cache_ops = g_metrics.cache_hits + g_metrics.cache_misses;
    if (total_cache_ops > 0) {
        g_metrics.cache_hit_rate = (double)g_metrics.cache_hits / total_cache_ops;
    }

    metrics_update_resources();
}

void metrics_print_summary(void) {
    metrics_calculate_stats();

    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                    REPROVM METRICS SUMMARY                     ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ Execution Summary:                                             ║\n");
    printf("║   Total Tasks:        %-40lu ║\n", g_metrics.total_tasks);
    printf("║   Tasks Executed:     %-40lu ║\n", g_metrics.tasks_executed);
    printf("║   Tasks Cached:       %-40lu ║\n", g_metrics.tasks_cached);
    printf("║   Tasks Failed:       %-40lu ║\n", g_metrics.tasks_failed);
    printf("║   Elapsed Time:       %-38.2f s ║\n", g_metrics.elapsed_seconds);
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ Cache Performance:                                             ║\n");
    printf("║   Cache Hits:         %-40lu ║\n", g_metrics.cache_hits);
    printf("║   Cache Misses:       %-40lu ║\n", g_metrics.cache_misses);
    printf("║   Cache Hit Rate:     %-38.2f%% ║\n", g_metrics.cache_hit_rate * 100.0);
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ Task Timing (ms):                                              ║\n");
    printf("║   Average:            %-40.2f ║\n", g_metrics.avg_task_time_ms);
    printf("║   Minimum:            %-40.2f ║\n", g_metrics.min_task_time_ms);
    printf("║   Maximum:            %-40.2f ║\n", g_metrics.max_task_time_ms);
    printf("║   Total:              %-40.2f ║\n", g_metrics.total_execution_time_ms);
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ Resource Usage:                                                ║\n");
    printf("║   Peak Memory:        %-38.2f MB ║\n", g_metrics.peak_memory_bytes / (1024.0 * 1024.0));
    printf("║   CPU Usage:          %-38.2f%% ║\n", g_metrics.cpu_usage_percent);
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ Errors & Retries:                                              ║\n");
    printf("║   Errors:             %-40lu ║\n", g_metrics.error_count);
    printf("║   Retries:            %-40lu ║\n", g_metrics.retry_count);
    printf("║   Timeouts:           %-40lu ║\n", g_metrics.timeout_count);
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

int metrics_write_to_file(const char *filename) {
    FILE *fp = fopen(filename, "a");
    if (!fp) {
        LOG_ERROR("Failed to open metrics file: %s", filename);
        return -1;
    }

    metrics_calculate_stats();

    fprintf(fp, "\n=== Metrics Report - %s ===\n", ctime(&g_metrics.end_time));
    fprintf(fp, "Total Tasks: %lu\n", g_metrics.total_tasks);
    fprintf(fp, "Tasks Executed: %lu\n", g_metrics.tasks_executed);
    fprintf(fp, "Tasks Cached: %lu\n", g_metrics.tasks_cached);
    fprintf(fp, "Tasks Failed: %lu\n", g_metrics.tasks_failed);
    fprintf(fp, "Cache Hit Rate: %.2f%%\n", g_metrics.cache_hit_rate * 100.0);
    fprintf(fp, "Average Task Time: %.2f ms\n", g_metrics.avg_task_time_ms);
    fprintf(fp, "Total Execution Time: %.2f ms\n", g_metrics.total_execution_time_ms);
    fprintf(fp, "Elapsed Time: %.2f seconds\n", g_metrics.elapsed_seconds);
    fprintf(fp, "Peak Memory: %.2f MB\n", g_metrics.peak_memory_bytes / (1024.0 * 1024.0));
    fprintf(fp, "Errors: %lu\n", g_metrics.error_count);
    fprintf(fp, "Retries: %lu\n", g_metrics.retry_count);
    fprintf(fp, "\n");

    fclose(fp);
    LOG_INFO("Metrics written to %s", filename);
    return 0;
}

int metrics_write_json(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        LOG_ERROR("Failed to open metrics JSON file: %s", filename);
        return -1;
    }

    metrics_calculate_stats();

    fprintf(fp, "{\n");
    fprintf(fp, "  \"timestamp\": %ld,\n", g_metrics.end_time);
    fprintf(fp, "  \"elapsed_seconds\": %.2f,\n", g_metrics.elapsed_seconds);
    fprintf(fp, "  \"tasks\": {\n");
    fprintf(fp, "    \"total\": %lu,\n", g_metrics.total_tasks);
    fprintf(fp, "    \"executed\": %lu,\n", g_metrics.tasks_executed);
    fprintf(fp, "    \"cached\": %lu,\n", g_metrics.tasks_cached);
    fprintf(fp, "    \"failed\": %lu\n", g_metrics.tasks_failed);
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"cache\": {\n");
    fprintf(fp, "    \"hits\": %lu,\n", g_metrics.cache_hits);
    fprintf(fp, "    \"misses\": %lu,\n", g_metrics.cache_misses);
    fprintf(fp, "    \"hit_rate\": %.4f\n", g_metrics.cache_hit_rate);
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"timing_ms\": {\n");
    fprintf(fp, "    \"total\": %.2f,\n", g_metrics.total_execution_time_ms);
    fprintf(fp, "    \"average\": %.2f,\n", g_metrics.avg_task_time_ms);
    fprintf(fp, "    \"min\": %.2f,\n", g_metrics.min_task_time_ms);
    fprintf(fp, "    \"max\": %.2f\n", g_metrics.max_task_time_ms);
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"resources\": {\n");
    fprintf(fp, "    \"peak_memory_bytes\": %lu,\n", g_metrics.peak_memory_bytes);
    fprintf(fp, "    \"cpu_usage_percent\": %.2f\n", g_metrics.cpu_usage_percent);
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"errors\": {\n");
    fprintf(fp, "    \"total\": %lu,\n", g_metrics.error_count);
    fprintf(fp, "    \"retries\": %lu,\n", g_metrics.retry_count);
    fprintf(fp, "    \"timeouts\": %lu\n", g_metrics.timeout_count);
    fprintf(fp, "  }\n");
    fprintf(fp, "}\n");

    fclose(fp);
    LOG_INFO("Metrics JSON written to %s", filename);
    return 0;
}
