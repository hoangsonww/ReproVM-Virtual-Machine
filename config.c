#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// Global configuration instance
ReproVMConfig g_config;

void config_init_defaults(ReproVMConfig *config) {
    memset(config, 0, sizeof(ReproVMConfig));

    // Logging defaults
    strcpy(config->log_file, ".reprovm/reprovm.log");
    config->log_level = LOG_INFO;
    config->log_colors = 1;

    // Cache defaults
    strcpy(config->cache_dir, ".reprovm");
    config->max_cache_size_mb = 10240; // 10GB
    config->cache_ttl_hours = 168;     // 1 week

    // Execution defaults
    config->parallel_jobs = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (config->parallel_jobs <= 0) {
        config->parallel_jobs = 4;
    }
    config->retry_attempts = 3;
    config->retry_delay_ms = 1000;
    config->timeout_seconds = 3600; // 1 hour

    // Performance defaults
    config->enable_metrics = 1;
    config->metrics_interval_seconds = 60;
    strcpy(config->metrics_file, ".reprovm/metrics.log");

    // Security defaults
    config->enable_sandboxing = 0;
    config->max_input_size_mb = 1024; // 1GB
    config->validate_checksums = 1;

    // Remote CAS defaults
    config->enable_remote_cas = 0;
    strcpy(config->remote_cas_url, "");
    strcpy(config->remote_cas_access_key, "");
    strcpy(config->remote_cas_secret_key, "");

    // Feature defaults
    config->enable_colors = 1;
    config->enable_progress_bar = 1;
    config->verbose = 0;
    config->force_rebuild = 0;
}

void config_load_from_env(ReproVMConfig *config) {
    char *env;

    // Logging
    if ((env = getenv("REPROVM_LOG_LEVEL"))) {
        if (strcmp(env, "DEBUG") == 0) config->log_level = LOG_DEBUG;
        else if (strcmp(env, "INFO") == 0) config->log_level = LOG_INFO;
        else if (strcmp(env, "WARN") == 0) config->log_level = LOG_WARN;
        else if (strcmp(env, "ERROR") == 0) config->log_level = LOG_ERROR;
    }

    if ((env = getenv("REPROVM_LOG_FILE"))) {
        strncpy(config->log_file, env, sizeof(config->log_file) - 1);
    }

    if ((env = getenv("REPROVM_NO_COLOR"))) {
        config->log_colors = 0;
        config->enable_colors = 0;
    }

    // Cache
    if ((env = getenv("REPROVM_CACHE_DIR"))) {
        strncpy(config->cache_dir, env, sizeof(config->cache_dir) - 1);
    }

    if ((env = getenv("REPROVM_MAX_CACHE_SIZE"))) {
        config->max_cache_size_mb = atoi(env);
    }

    // Execution
    if ((env = getenv("REPROVM_JOBS"))) {
        config->parallel_jobs = atoi(env);
    }

    if ((env = getenv("REPROVM_RETRY_ATTEMPTS"))) {
        config->retry_attempts = atoi(env);
    }

    if ((env = getenv("REPROVM_TIMEOUT"))) {
        config->timeout_seconds = atoi(env);
    }

    // Remote CAS
    if ((env = getenv("REPROVM_REMOTE_CAS_URL"))) {
        strncpy(config->remote_cas_url, env, sizeof(config->remote_cas_url) - 1);
        config->enable_remote_cas = 1;
    }

    if ((env = getenv("REPROVM_REMOTE_CAS_ACCESS_KEY"))) {
        strncpy(config->remote_cas_access_key, env, sizeof(config->remote_cas_access_key) - 1);
    }

    if ((env = getenv("REPROVM_REMOTE_CAS_SECRET_KEY"))) {
        strncpy(config->remote_cas_secret_key, env, sizeof(config->remote_cas_secret_key) - 1);
    }

    // Features
    if ((env = getenv("REPROVM_VERBOSE"))) {
        config->verbose = atoi(env);
    }

    if ((env = getenv("REPROVM_FORCE"))) {
        config->force_rebuild = atoi(env);
    }

    if ((env = getenv("REPROVM_NO_PROGRESS"))) {
        config->enable_progress_bar = 0;
    }
}

int config_load_from_file(ReproVMConfig *config, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return -1;
    }

    char line[1024];
    int line_num = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;

        // Skip comments and empty lines
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\0') continue;

        // Parse key=value
        char key[256], value[512];
        if (sscanf(line, "%255[^=]=%511[^\n]", key, value) != 2) {
            fprintf(stderr, "Warning: Invalid config line %d: %s", line_num, line);
            continue;
        }

        // Trim whitespace
        char *k = key;
        while (*k == ' ' || *k == '\t') k++;
        char *ke = k + strlen(k) - 1;
        while (ke > k && (*ke == ' ' || *ke == '\t')) *ke-- = '\0';

        char *v = value;
        while (*v == ' ' || *v == '\t') v++;
        char *ve = v + strlen(v) - 1;
        while (ve > v && (*ve == ' ' || *ve == '\t' || *ve == '\n')) *ve-- = '\0';

        // Apply configuration
        if (strcmp(k, "log_file") == 0) {
            strncpy(config->log_file, v, sizeof(config->log_file) - 1);
        } else if (strcmp(k, "log_level") == 0) {
            if (strcmp(v, "DEBUG") == 0) config->log_level = LOG_DEBUG;
            else if (strcmp(v, "INFO") == 0) config->log_level = LOG_INFO;
            else if (strcmp(v, "WARN") == 0) config->log_level = LOG_WARN;
            else if (strcmp(v, "ERROR") == 0) config->log_level = LOG_ERROR;
        } else if (strcmp(k, "cache_dir") == 0) {
            strncpy(config->cache_dir, v, sizeof(config->cache_dir) - 1);
        } else if (strcmp(k, "parallel_jobs") == 0) {
            config->parallel_jobs = atoi(v);
        } else if (strcmp(k, "retry_attempts") == 0) {
            config->retry_attempts = atoi(v);
        } else if (strcmp(k, "timeout_seconds") == 0) {
            config->timeout_seconds = atoi(v);
        } else if (strcmp(k, "enable_metrics") == 0) {
            config->enable_metrics = atoi(v);
        } else if (strcmp(k, "remote_cas_url") == 0) {
            strncpy(config->remote_cas_url, v, sizeof(config->remote_cas_url) - 1);
            config->enable_remote_cas = 1;
        }
    }

    fclose(fp);
    return 0;
}

int config_validate(const ReproVMConfig *config) {
    if (config->parallel_jobs < 1 || config->parallel_jobs > 256) {
        fprintf(stderr, "Error: parallel_jobs must be between 1 and 256\n");
        return -1;
    }

    if (config->retry_attempts < 0 || config->retry_attempts > 100) {
        fprintf(stderr, "Error: retry_attempts must be between 0 and 100\n");
        return -1;
    }

    if (config->timeout_seconds < 0) {
        fprintf(stderr, "Error: timeout_seconds cannot be negative\n");
        return -1;
    }

    if (config->max_cache_size_mb < 0) {
        fprintf(stderr, "Error: max_cache_size_mb cannot be negative\n");
        return -1;
    }

    return 0;
}

void config_print(const ReproVMConfig *config) {
    printf("=== ReproVM Configuration ===\n");
    printf("Logging:\n");
    printf("  log_file: %s\n", config->log_file);
    printf("  log_level: %s\n", log_level_name(config->log_level));
    printf("  log_colors: %d\n", config->log_colors);
    printf("\nCache:\n");
    printf("  cache_dir: %s\n", config->cache_dir);
    printf("  max_cache_size_mb: %d\n", config->max_cache_size_mb);
    printf("  cache_ttl_hours: %d\n", config->cache_ttl_hours);
    printf("\nExecution:\n");
    printf("  parallel_jobs: %d\n", config->parallel_jobs);
    printf("  retry_attempts: %d\n", config->retry_attempts);
    printf("  timeout_seconds: %d\n", config->timeout_seconds);
    printf("\nPerformance:\n");
    printf("  enable_metrics: %d\n", config->enable_metrics);
    printf("  metrics_interval: %d seconds\n", config->metrics_interval_seconds);
    printf("\nSecurity:\n");
    printf("  enable_sandboxing: %d\n", config->enable_sandboxing);
    printf("  validate_checksums: %d\n", config->validate_checksums);
    printf("\nRemote CAS:\n");
    printf("  enable_remote_cas: %d\n", config->enable_remote_cas);
    if (config->enable_remote_cas) {
        printf("  remote_cas_url: %s\n", config->remote_cas_url);
    }
    printf("\nFeatures:\n");
    printf("  enable_colors: %d\n", config->enable_colors);
    printf("  enable_progress_bar: %d\n", config->enable_progress_bar);
    printf("  verbose: %d\n", config->verbose);
    printf("=============================\n");
}

int config_save_to_file(const ReproVMConfig *config, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        return -1;
    }

    fprintf(fp, "# ReproVM Configuration File\n");
    fprintf(fp, "# Generated automatically\n\n");

    fprintf(fp, "# Logging\n");
    fprintf(fp, "log_file=%s\n", config->log_file);
    fprintf(fp, "log_level=%s\n", log_level_name(config->log_level));

    fprintf(fp, "\n# Cache\n");
    fprintf(fp, "cache_dir=%s\n", config->cache_dir);
    fprintf(fp, "max_cache_size_mb=%d\n", config->max_cache_size_mb);

    fprintf(fp, "\n# Execution\n");
    fprintf(fp, "parallel_jobs=%d\n", config->parallel_jobs);
    fprintf(fp, "retry_attempts=%d\n", config->retry_attempts);
    fprintf(fp, "timeout_seconds=%d\n", config->timeout_seconds);

    fprintf(fp, "\n# Performance\n");
    fprintf(fp, "enable_metrics=%d\n", config->enable_metrics);

    if (config->enable_remote_cas) {
        fprintf(fp, "\n# Remote CAS\n");
        fprintf(fp, "remote_cas_url=%s\n", config->remote_cas_url);
    }

    fclose(fp);
    return 0;
}
