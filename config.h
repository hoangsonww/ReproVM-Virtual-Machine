#ifndef CONFIG_H
#define CONFIG_H

#include "logger.h"

// Configuration structure
typedef struct {
    // Logging
    char log_file[256];
    LogLevel log_level;
    int log_colors;

    // Cache
    char cache_dir[256];
    int max_cache_size_mb;
    int cache_ttl_hours;

    // Execution
    int parallel_jobs;
    int retry_attempts;
    int retry_delay_ms;
    int timeout_seconds;

    // Performance
    int enable_metrics;
    int metrics_interval_seconds;
    char metrics_file[256];

    // Security
    int enable_sandboxing;
    int max_input_size_mb;
    int validate_checksums;

    // Remote CAS
    int enable_remote_cas;
    char remote_cas_url[512];
    char remote_cas_access_key[256];
    char remote_cas_secret_key[256];

    // Features
    int enable_colors;
    int enable_progress_bar;
    int verbose;
    int force_rebuild;
} ReproVMConfig;

// Global configuration
extern ReproVMConfig g_config;

// Initialize configuration with defaults
void config_init_defaults(ReproVMConfig *config);

// Load configuration from file
int config_load_from_file(ReproVMConfig *config, const char *filename);

// Load configuration from environment variables
void config_load_from_env(ReproVMConfig *config);

// Validate configuration
int config_validate(const ReproVMConfig *config);

// Print configuration (for debugging)
void config_print(const ReproVMConfig *config);

// Save configuration to file
int config_save_to_file(const ReproVMConfig *config, const char *filename);

#endif // CONFIG_H
