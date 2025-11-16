#include "health_check.h"
#include "logger.h"
#include "config.h"
#include "metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

SystemHealth g_system_health = {0};

void health_init(void) {
    memset(&g_system_health, 0, sizeof(SystemHealth));
    g_system_health.overall_status = HEALTH_UNKNOWN;
    LOG_DEBUG("Health check system initialized");
}

const char* health_status_name(HealthStatus status) {
    switch (status) {
        case HEALTH_OK:       return "OK";
        case HEALTH_WARNING:  return "WARNING";
        case HEALTH_CRITICAL: return "CRITICAL";
        case HEALTH_UNKNOWN:  return "UNKNOWN";
        default:              return "INVALID";
    }
}

HealthCheckResult health_check_cache(void) {
    HealthCheckResult result = {0};
    double start = get_time_ms();
    result.timestamp = time(NULL);

    char cache_path[512];
    snprintf(cache_path, sizeof(cache_path), "%s/cache", g_config.cache_dir);

    struct stat st;
    if (stat(cache_path, &st) == 0 && S_ISDIR(st.st_mode)) {
        // Count cache entries
        DIR *dir = opendir(cache_path);
        if (dir) {
            int count = 0;
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_name[0] != '.') {
                    count++;
                }
            }
            closedir(dir);

            result.status = HEALTH_OK;
            snprintf(result.message, sizeof(result.message),
                     "Cache operational with %d entries", count);
        } else {
            result.status = HEALTH_WARNING;
            snprintf(result.message, sizeof(result.message),
                     "Cache directory exists but cannot be opened");
        }
    } else {
        result.status = HEALTH_WARNING;
        snprintf(result.message, sizeof(result.message),
                 "Cache directory does not exist or is not accessible");
    }

    result.response_time_ms = get_time_ms() - start;
    return result;
}

HealthCheckResult health_check_cas(void) {
    HealthCheckResult result = {0};
    double start = get_time_ms();
    result.timestamp = time(NULL);

    char cas_path[512];
    snprintf(cas_path, sizeof(cas_path), "%s/cas/objects", g_config.cache_dir);

    struct stat st;
    if (stat(cas_path, &st) == 0 && S_ISDIR(st.st_mode)) {
        // Count CAS objects
        DIR *dir = opendir(cas_path);
        if (dir) {
            int count = 0;
            uint64_t total_size = 0;
            struct dirent *entry;

            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_name[0] != '.') {
                    char subdir[1024];
                    snprintf(subdir, sizeof(subdir), "%s/%s", cas_path, entry->d_name);

                    DIR *subdir_dir = opendir(subdir);
                    if (subdir_dir) {
                        struct dirent *subentry;
                        while ((subentry = readdir(subdir_dir)) != NULL) {
                            if (subentry->d_name[0] != '.') {
                                count++;

                                char filepath[2048];
                                snprintf(filepath, sizeof(filepath), "%s/%s", subdir, subentry->d_name);
                                struct stat file_st;
                                if (stat(filepath, &file_st) == 0) {
                                    total_size += file_st.st_size;
                                }
                            }
                        }
                        closedir(subdir_dir);
                    }
                }
            }
            closedir(dir);

            result.status = HEALTH_OK;
            snprintf(result.message, sizeof(result.message),
                     "CAS operational with %d objects (%.2f MB)",
                     count, total_size / (1024.0 * 1024.0));
        } else {
            result.status = HEALTH_WARNING;
            snprintf(result.message, sizeof(result.message),
                     "CAS directory exists but cannot be opened");
        }
    } else {
        result.status = HEALTH_WARNING;
        snprintf(result.message, sizeof(result.message),
                 "CAS directory does not exist or is not accessible");
    }

    result.response_time_ms = get_time_ms() - start;
    return result;
}

HealthCheckResult health_check_disk(void) {
    HealthCheckResult result = {0};
    double start = get_time_ms();
    result.timestamp = time(NULL);

    struct statvfs stat;
    if (statvfs(g_config.cache_dir, &stat) == 0) {
        uint64_t available = stat.f_bavail * stat.f_frsize;
        uint64_t total = stat.f_blocks * stat.f_frsize;
        uint64_t used = total - available;

        double usage_percent = (double)used / total * 100.0;

        g_system_health.disk_usage_bytes = used;
        g_system_health.disk_available_bytes = available;

        if (usage_percent > 90.0) {
            result.status = HEALTH_CRITICAL;
            snprintf(result.message, sizeof(result.message),
                     "Disk usage critical: %.1f%% used", usage_percent);
        } else if (usage_percent > 75.0) {
            result.status = HEALTH_WARNING;
            snprintf(result.message, sizeof(result.message),
                     "Disk usage high: %.1f%% used", usage_percent);
        } else {
            result.status = HEALTH_OK;
            snprintf(result.message, sizeof(result.message),
                     "Disk usage normal: %.1f%% used (%.2f GB available)",
                     usage_percent, available / (1024.0 * 1024.0 * 1024.0));
        }
    } else {
        result.status = HEALTH_CRITICAL;
        snprintf(result.message, sizeof(result.message),
                 "Cannot stat filesystem");
    }

    result.response_time_ms = get_time_ms() - start;
    return result;
}

HealthCheckResult health_check_memory(void) {
    HealthCheckResult result = {0};
    double start = get_time_ms();
    result.timestamp = time(NULL);

    uint64_t mem_usage = get_memory_usage();
    g_system_health.memory_usage_bytes = mem_usage;

    double mem_mb = mem_usage / (1024.0 * 1024.0);

    if (mem_mb > 1024.0) { // > 1GB
        result.status = HEALTH_WARNING;
        snprintf(result.message, sizeof(result.message),
                 "High memory usage: %.2f MB", mem_mb);
    } else {
        result.status = HEALTH_OK;
        snprintf(result.message, sizeof(result.message),
                 "Memory usage normal: %.2f MB", mem_mb);
    }

    result.response_time_ms = get_time_ms() - start;
    return result;
}

void health_check_all(void) {
    g_system_health.cache_health = health_check_cache();
    g_system_health.cas_health = health_check_cas();
    g_system_health.disk_health = health_check_disk();
    g_system_health.memory_health = health_check_memory();

    g_system_health.cpu_usage = get_cpu_usage();
    g_system_health.last_check = time(NULL);

    // Determine overall status
    HealthStatus max_status = HEALTH_OK;
    if (g_system_health.cache_health.status > max_status)
        max_status = g_system_health.cache_health.status;
    if (g_system_health.cas_health.status > max_status)
        max_status = g_system_health.cas_health.status;
    if (g_system_health.disk_health.status > max_status)
        max_status = g_system_health.disk_health.status;
    if (g_system_health.memory_health.status > max_status)
        max_status = g_system_health.memory_health.status;

    g_system_health.overall_status = max_status;

    LOG_DEBUG("Health check completed: overall status = %s",
              health_status_name(max_status));
}

HealthStatus health_get_overall_status(void) {
    return g_system_health.overall_status;
}

void health_print_report(void) {
    health_check_all();

    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                     SYSTEM HEALTH REPORT                       ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ Overall Status: %-47s║\n", health_status_name(g_system_health.overall_status));
    printf("║ Last Check: %-51s║\n", ctime(&g_system_health.last_check));
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ Cache:     [%-8s] %-40s║\n",
           health_status_name(g_system_health.cache_health.status),
           g_system_health.cache_health.message);
    printf("║ CAS:       [%-8s] %-40s║\n",
           health_status_name(g_system_health.cas_health.status),
           g_system_health.cas_health.message);
    printf("║ Disk:      [%-8s] %-40s║\n",
           health_status_name(g_system_health.disk_health.status),
           g_system_health.disk_health.message);
    printf("║ Memory:    [%-8s] %-40s║\n",
           health_status_name(g_system_health.memory_health.status),
           g_system_health.memory_health.message);
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

int health_write_to_file(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        LOG_ERROR("Failed to open health file: %s", filename);
        return -1;
    }

    fprintf(fp, "=== System Health Report ===\n");
    fprintf(fp, "Timestamp: %s", ctime(&g_system_health.last_check));
    fprintf(fp, "Overall Status: %s\n\n", health_status_name(g_system_health.overall_status));

    fprintf(fp, "Cache: [%s] %s\n",
            health_status_name(g_system_health.cache_health.status),
            g_system_health.cache_health.message);
    fprintf(fp, "CAS: [%s] %s\n",
            health_status_name(g_system_health.cas_health.status),
            g_system_health.cas_health.message);
    fprintf(fp, "Disk: [%s] %s\n",
            health_status_name(g_system_health.disk_health.status),
            g_system_health.disk_health.message);
    fprintf(fp, "Memory: [%s] %s\n",
            health_status_name(g_system_health.memory_health.status),
            g_system_health.memory_health.message);

    fclose(fp);
    return 0;
}

int health_write_json(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        LOG_ERROR("Failed to open health JSON file: %s", filename);
        return -1;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"timestamp\": %ld,\n", g_system_health.last_check);
    fprintf(fp, "  \"overall_status\": \"%s\",\n",
            health_status_name(g_system_health.overall_status));
    fprintf(fp, "  \"cache\": {\n");
    fprintf(fp, "    \"status\": \"%s\",\n",
            health_status_name(g_system_health.cache_health.status));
    fprintf(fp, "    \"message\": \"%s\",\n", g_system_health.cache_health.message);
    fprintf(fp, "    \"response_time_ms\": %.2f\n", g_system_health.cache_health.response_time_ms);
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"cas\": {\n");
    fprintf(fp, "    \"status\": \"%s\",\n",
            health_status_name(g_system_health.cas_health.status));
    fprintf(fp, "    \"message\": \"%s\",\n", g_system_health.cas_health.message);
    fprintf(fp, "    \"response_time_ms\": %.2f\n", g_system_health.cas_health.response_time_ms);
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"disk\": {\n");
    fprintf(fp, "    \"status\": \"%s\",\n",
            health_status_name(g_system_health.disk_health.status));
    fprintf(fp, "    \"message\": \"%s\",\n", g_system_health.disk_health.message);
    fprintf(fp, "    \"response_time_ms\": %.2f\n", g_system_health.disk_health.response_time_ms);
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"memory\": {\n");
    fprintf(fp, "    \"status\": \"%s\",\n",
            health_status_name(g_system_health.memory_health.status));
    fprintf(fp, "    \"message\": \"%s\",\n", g_system_health.memory_health.message);
    fprintf(fp, "    \"response_time_ms\": %.2f\n", g_system_health.memory_health.response_time_ms);
    fprintf(fp, "  }\n");
    fprintf(fp, "}\n");

    fclose(fp);
    return 0;
}
