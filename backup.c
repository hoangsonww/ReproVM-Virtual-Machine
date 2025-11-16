#include "backup.h"
#include "logger.h"
#include "compression.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

static BackupConfig g_backup_config = {0};

int backup_init(const char *backup_dir) {
    if (backup_dir) {
        strncpy(g_backup_config.backup_dir, backup_dir,
                sizeof(g_backup_config.backup_dir) - 1);
    } else {
        strcpy(g_backup_config.backup_dir, ".reprovm/backups");
    }

    g_backup_config.max_backups = 10;
    g_backup_config.compression_enabled = 1;

    // Create backup directory
    mkdir(g_backup_config.backup_dir, 0755);

    LOG_INFO("Backup system initialized: dir=%s", g_backup_config.backup_dir);
    return 0;
}

int backup_create_full(const char *backup_name) {
    char backup_path[1024];
    snprintf(backup_path, sizeof(backup_path), "%s/%s",
             g_backup_config.backup_dir, backup_name);

    LOG_INFO("Creating full backup: %s", backup_name);

    // Create backup directory
    mkdir(backup_path, 0755);

    // Backup CAS objects
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cp -r .reprovm/cas %s/", backup_path);
    if (system(cmd) != 0) {
        LOG_ERROR("Failed to backup CAS");
        return -1;
    }

    // Backup cache
    snprintf(cmd, sizeof(cmd), "cp -r .reprovm/cache %s/", backup_path);
    if (system(cmd) != 0) {
        LOG_ERROR("Failed to backup cache");
        return -1;
    }

    // Create metadata
    char metadata_path[1024];
    snprintf(metadata_path, sizeof(metadata_path), "%s/metadata.txt", backup_path);
    FILE *meta = fopen(metadata_path, "w");
    if (meta) {
        fprintf(meta, "type=full\n");
        fprintf(meta, "created=%ld\n", time(NULL));
        fprintf(meta, "name=%s\n", backup_name);
        fclose(meta);
    }

    g_backup_config.last_backup = time(NULL);

    LOG_INFO("Backup created successfully: %s", backup_name);
    return 0;
}

int backup_create_incremental(const char *base_backup, const char *backup_name) {
    LOG_INFO("Creating incremental backup: %s (base: %s)", backup_name, base_backup);

    // In production, this would:
    // 1. Compare current state with base backup
    // 2. Only backup changed files
    // 3. Create delta metadata

    return backup_create_full(backup_name);
}

int backup_list(char ***backup_names, int *count) {
    DIR *dir = opendir(g_backup_config.backup_dir);
    if (!dir) {
        *count = 0;
        return -1;
    }

    // Count backups
    *count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.' && entry->d_type == DT_DIR) {
            (*count)++;
        }
    }

    rewinddir(dir);

    // Allocate array
    *backup_names = malloc(sizeof(char*) * (*count));
    int i = 0;
    while ((entry = readdir(dir)) != NULL && i < *count) {
        if (entry->d_name[0] != '.' && entry->d_type == DT_DIR) {
            (*backup_names)[i] = strdup(entry->d_name);
            i++;
        }
    }

    closedir(dir);
    return 0;
}

int backup_restore(const char *backup_name, const char *restore_path) {
    char backup_path[1024];
    snprintf(backup_path, sizeof(backup_path), "%s/%s",
             g_backup_config.backup_dir, backup_name);

    LOG_INFO("Restoring backup: %s to %s", backup_name, restore_path);

    // Restore CAS
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cp -r %s/cas %s/.reprovm/", backup_path, restore_path);
    if (system(cmd) != 0) {
        LOG_ERROR("Failed to restore CAS");
        return -1;
    }

    // Restore cache
    snprintf(cmd, sizeof(cmd), "cp -r %s/cache %s/.reprovm/", backup_path, restore_path);
    if (system(cmd) != 0) {
        LOG_ERROR("Failed to restore cache");
        return -1;
    }

    LOG_INFO("Backup restored successfully");
    return 0;
}

int backup_delete(const char *backup_name) {
    char backup_path[1024];
    snprintf(backup_path, sizeof(backup_path), "%s/%s",
             g_backup_config.backup_dir, backup_name);

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", backup_path);

    LOG_INFO("Deleting backup: %s", backup_name);
    return system(cmd);
}

int backup_verify(const char *backup_name) {
    char backup_path[1024];
    snprintf(backup_path, sizeof(backup_path), "%s/%s",
             g_backup_config.backup_dir, backup_name);

    // Check if backup exists
    struct stat st;
    if (stat(backup_path, &st) != 0) {
        LOG_ERROR("Backup not found: %s", backup_name);
        return -1;
    }

    // Verify structure
    char cas_path[1024];
    snprintf(cas_path, sizeof(cas_path), "%s/cas", backup_path);
    if (stat(cas_path, &st) != 0) {
        LOG_ERROR("Backup CAS missing: %s", backup_name);
        return -1;
    }

    LOG_INFO("Backup verified: %s", backup_name);
    return 0;
}

int backup_cleanup_old(int keep_count) {
    char **backups;
    int count;

    if (backup_list(&backups, &count) != 0) {
        return -1;
    }

    if (count <= keep_count) {
        // Free backup list
        for (int i = 0; i < count; i++) {
            free(backups[i]);
        }
        free(backups);
        return 0;
    }

    // Delete oldest backups
    int to_delete = count - keep_count;
    for (int i = 0; i < to_delete; i++) {
        backup_delete(backups[i]);
        free(backups[i]);
    }

    // Free remaining
    for (int i = to_delete; i < count; i++) {
        free(backups[i]);
    }
    free(backups);

    LOG_INFO("Cleaned up %d old backups", to_delete);
    return 0;
}

void backup_export_metadata(const char *backup_name, const char *output_file) {
    char backup_path[1024];
    snprintf(backup_path, sizeof(backup_path), "%s/%s/metadata.txt",
             g_backup_config.backup_dir, backup_name);

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cp %s %s", backup_path, output_file);
    system(cmd);
}
