#ifndef SECURITY_H
#define SECURITY_H

#include <stddef.h>

// Input validation
int validate_file_path(const char *path);
int validate_task_name(const char *name);
int validate_command(const char *cmd);
int validate_hash(const char *hash);

// Sanitization
char* sanitize_string(const char *input, size_t max_len);
char* sanitize_path(const char *path);

// Permission checks
int check_file_permissions(const char *path, int write_required);
int check_directory_permissions(const char *path);

// Sandboxing utilities
int enable_sandbox_mode(void);
int disable_sandbox_mode(void);

// Checksum validation
int verify_file_checksum(const char *filepath, const char *expected_hash);

// Size limits
int check_file_size_limit(const char *filepath, size_t max_size_bytes);

// Path safety
int is_path_safe(const char *path);
int is_within_directory(const char *path, const char *base_dir);

// Command safety
int is_command_safe(const char *cmd);
char** get_dangerous_commands(void);

#endif // SECURITY_H
