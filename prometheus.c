#include "prometheus.h"
#include "logger.h"
#include "metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static PrometheusExporter g_prometheus = {0};

int prometheus_init(int port) {
    g_prometheus.port = port > 0 ? port : 9090;
    g_prometheus.enabled = 1;
    g_prometheus.metrics_path = strdup("/metrics");

    LOG_INFO("Prometheus exporter initialized on port %d", g_prometheus.port);
    return 0;
}

void prometheus_export_metrics(FILE *output) {
    if (!output) return;

    fprintf(output, "# HELP reprovm_tasks_total Total number of tasks\n");
    fprintf(output, "# TYPE reprovm_tasks_total counter\n");
    fprintf(output, "reprovm_tasks_total %lu\n", g_metrics.total_tasks);

    fprintf(output, "# HELP reprovm_tasks_executed Number of tasks executed\n");
    fprintf(output, "# TYPE reprovm_tasks_executed counter\n");
    fprintf(output, "reprovm_tasks_executed %lu\n", g_metrics.tasks_executed);

    fprintf(output, "# HELP reprovm_tasks_cached Number of cached tasks\n");
    fprintf(output, "# TYPE reprovm_tasks_cached counter\n");
    fprintf(output, "reprovm_tasks_cached %lu\n", g_metrics.tasks_cached);

    fprintf(output, "# HELP reprovm_tasks_failed Number of failed tasks\n");
    fprintf(output, "# TYPE reprovm_tasks_failed counter\n");
    fprintf(output, "reprovm_tasks_failed %lu\n", g_metrics.tasks_failed);

    fprintf(output, "# HELP reprovm_cache_hit_rate Cache hit rate\n");
    fprintf(output, "# TYPE reprovm_cache_hit_rate gauge\n");
    fprintf(output, "reprovm_cache_hit_rate %.4f\n", g_metrics.cache_hit_rate);

    fprintf(output, "# HELP reprovm_execution_time_ms Total execution time in milliseconds\n");
    fprintf(output, "# TYPE reprovm_execution_time_ms gauge\n");
    fprintf(output, "reprovm_execution_time_ms %.2f\n", g_metrics.total_execution_time_ms);

    fprintf(output, "# HELP reprovm_memory_bytes Peak memory usage in bytes\n");
    fprintf(output, "# TYPE reprovm_memory_bytes gauge\n");
    fprintf(output, "reprovm_memory_bytes %lu\n", g_metrics.peak_memory_bytes);

    fprintf(output, "# HELP reprovm_cpu_usage_percent CPU usage percentage\n");
    fprintf(output, "# TYPE reprovm_cpu_usage_percent gauge\n");
    fprintf(output, "reprovm_cpu_usage_percent %.2f\n", g_metrics.cpu_usage_percent);

    fprintf(output, "# HELP reprovm_errors_total Total number of errors\n");
    fprintf(output, "# TYPE reprovm_errors_total counter\n");
    fprintf(output, "reprovm_errors_total %lu\n", g_metrics.error_count);

    fprintf(output, "# HELP reprovm_retries_total Total number of retries\n");
    fprintf(output, "# TYPE reprovm_retries_total counter\n");
    fprintf(output, "reprovm_retries_total %lu\n", g_metrics.retry_count);
}

char* prometheus_get_metrics_string(void) {
    // Allocate large buffer for metrics
    char *buffer = malloc(8192);
    if (!buffer) return NULL;

    FILE *mem_stream = fmemopen(buffer, 8192, "w");
    if (!mem_stream) {
        free(buffer);
        return NULL;
    }

    prometheus_export_metrics(mem_stream);
    fclose(mem_stream);

    return buffer;
}

int prometheus_start_server(void) {
    if (!g_prometheus.enabled) return 0;

    LOG_INFO("Prometheus metrics server would start on port %d", g_prometheus.port);
    LOG_INFO("Metrics endpoint: http://localhost:%d%s",
             g_prometheus.port, g_prometheus.metrics_path);

    // In production, start HTTP server
    // This would use a library like libmicrohttpd

    return 0;
}

void prometheus_stop_server(void) {
    // Stop HTTP server
}

void prometheus_cleanup(void) {
    free(g_prometheus.metrics_path);
}
