#ifndef BACKUP_H
#define BACKUP_H

#include <time.h>

// Backup configuration
typedef struct {
    char backup_dir[512];
    int max_backups;
    int compression_enabled;
    time_t last_backup;
} BackupConfig;

// Initialize backup system
int backup_init(const char *backup_dir);

// Create full backup
int backup_create_full(const char *backup_name);

// Create incremental backup
int backup_create_incremental(const char *base_backup, const char *backup_name);

// List available backups
int backup_list(char ***backup_names, int *count);

// Restore from backup
int backup_restore(const char *backup_name, const char *restore_path);

// Delete backup
int backup_delete(const char *backup_name);

// Verify backup integrity
int backup_verify(const char *backup_name);

// Get backup info
int backup_get_info(const char *backup_name, time_t *created, size_t *size);

// Cleanup old backups
int backup_cleanup_old(int keep_count);

// Export backup metadata
void backup_export_metadata(const char *backup_name, const char *output_file);

#endif // BACKUP_H
