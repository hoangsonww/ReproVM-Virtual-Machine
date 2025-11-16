#ifndef PROMETHEUS_H
#define PROMETHEUS_H

#include <stdint.h>

// Prometheus exporter
typedef struct {
    int port;
    int enabled;
    char *metrics_path;
} PrometheusExporter;

// Initialize Prometheus exporter
int prometheus_init(int port);

// Start metrics server
int prometheus_start_server(void);

// Stop metrics server
void prometheus_stop_server(void);

// Export metrics in Prometheus format
void prometheus_export_metrics(FILE *output);

// Get metrics as string
char* prometheus_get_metrics_string(void);

// Metric types
void prometheus_counter_inc(const char *name, const char *labels);
void prometheus_gauge_set(const char *name, double value, const char *labels);
void prometheus_histogram_observe(const char *name, double value, const char *labels);

// Cleanup
void prometheus_cleanup(void);

#endif // PROMETHEUS_H
