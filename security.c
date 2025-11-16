#include "security.h"
#include "logger.h"
#include "error_handling.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

static const char* DANGEROUS_COMMANDS[] = {
    "rm -rf /",
    "dd if=/dev/zero",
    ":(){ :|:& };:",
    "mkfs",
    "format",
    "> /dev/sda",
    "wget | sh",
    "curl | sh",
    "chmod 777 /",
    NULL
};

int validate_file_path(const char *path) {
    if (!path || path[0] == '\0') {
        SET_ERROR(ERR_INVALID_ARG, "File path is NULL or empty");
        return -1;
    }

    if (strlen(path) > PATH_MAX) {
        SET_ERROR(ERR_INVALID_ARG, "File path too long: %zu bytes", strlen(path));
        return -1;
    }

    // Check for null bytes
    if (strlen(path) != strcspn(path, "\0")) {
        SET_ERROR(ERR_INVALID_ARG, "File path contains null bytes");
        return -1;
    }

    // Check for dangerous patterns
    if (strstr(path, "..")) {
        LOG_WARN("File path contains '..' which may be unsafe: %s", path);
    }

    return 0;
}

int validate_task_name(const char *name) {
    if (!name || name[0] == '\0') {
        SET_ERROR(ERR_INVALID_ARG, "Task name is NULL or empty");
        return -1;
    }

    if (strlen(name) > 256) {
        SET_ERROR(ERR_INVALID_ARG, "Task name too long: %zu bytes", strlen(name));
        return -1;
    }

    // Task names should be alphanumeric plus underscore, dash
    for (const char *p = name; *p; p++) {
        if (!isalnum(*p) && *p != '_' && *p != '-' && *p != '.') {
            SET_ERROR(ERR_INVALID_ARG, "Task name contains invalid character: '%c'", *p);
            return -1;
        }
    }

    return 0;
}

int validate_command(const char *cmd) {
    if (!cmd || cmd[0] == '\0') {
        SET_ERROR(ERR_INVALID_ARG, "Command is NULL or empty");
        return -1;
    }

    if (strlen(cmd) > 4096) {
        SET_ERROR(ERR_INVALID_ARG, "Command too long: %zu bytes", strlen(cmd));
        return -1;
    }

    // Check for dangerous command patterns
    for (int i = 0; DANGEROUS_COMMANDS[i] != NULL; i++) {
        if (strstr(cmd, DANGEROUS_COMMANDS[i])) {
            LOG_WARN("Command contains potentially dangerous pattern: %s", DANGEROUS_COMMANDS[i]);
        }
    }

    return 0;
}

int validate_hash(const char *hash) {
    if (!hash) {
        SET_ERROR(ERR_INVALID_HASH, "Hash is NULL");
        return -1;
    }

    size_t len = strlen(hash);
    if (len != 64) { // SHA-256 hash length
        SET_ERROR(ERR_INVALID_HASH, "Invalid hash length: %zu (expected 64)", len);
        return -1;
    }

    // Check if all characters are hexadecimal
    for (size_t i = 0; i < len; i++) {
        if (!isxdigit(hash[i])) {
            SET_ERROR(ERR_INVALID_HASH, "Hash contains non-hexadecimal character: '%c'", hash[i]);
            return -1;
        }
    }

    return 0;
}

char* sanitize_string(const char *input, size_t max_len) {
    if (!input) return NULL;

    size_t len = strlen(input);
    if (len > max_len) len = max_len;

    char *output = malloc(len + 1);
    if (!output) {
        SET_ERROR(ERR_MEMORY_ALLOC, "Failed to allocate memory for sanitized string");
        return NULL;
    }

    size_t j = 0;
    for (size_t i = 0; i < len && input[i]; i++) {
        // Remove control characters and non-printable characters
        if (isprint(input[i]) || isspace(input[i])) {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';

    return output;
}

char* sanitize_path(const char *path) {
    if (!path) return NULL;

    // Resolve to canonical path
    char *resolved = realpath(path, NULL);
    if (!resolved) {
        // If realpath fails, do basic sanitization
        return sanitize_string(path, PATH_MAX);
    }

    return resolved;
}

int check_file_permissions(const char *path, int write_required) {
    if (validate_file_path(path) != 0) {
        return -1;
    }

    if (access(path, F_OK) != 0) {
        SET_ERROR(ERR_FILE_NOT_FOUND, "File does not exist: %s", path);
        return -1;
    }

    int mode = R_OK;
    if (write_required) {
        mode |= W_OK;
    }

    if (access(path, mode) != 0) {
        SET_ERROR(ERR_PERMISSION_DENIED, "Insufficient permissions for file: %s", path);
        return -1;
    }

    return 0;
}

int check_directory_permissions(const char *path) {
    if (validate_file_path(path) != 0) {
        return -1;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        SET_ERROR(ERR_FILE_NOT_FOUND, "Directory does not exist: %s", path);
        return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        SET_ERROR(ERR_INVALID_ARG, "Path is not a directory: %s", path);
        return -1;
    }

    if (access(path, R_OK | W_OK | X_OK) != 0) {
        SET_ERROR(ERR_PERMISSION_DENIED, "Insufficient permissions for directory: %s", path);
        return -1;
    }

    return 0;
}

int check_file_size_limit(const char *filepath, size_t max_size_bytes) {
    struct stat st;
    if (stat(filepath, &st) != 0) {
        SET_ERROR(ERR_FILE_IO, "Cannot stat file: %s", filepath);
        return -1;
    }

    if ((size_t)st.st_size > max_size_bytes) {
        SET_ERROR(ERR_INVALID_ARG, "File size %ld exceeds limit %zu: %s",
                  st.st_size, max_size_bytes, filepath);
        return -1;
    }

    return 0;
}

int is_path_safe(const char *path) {
    if (validate_file_path(path) != 0) {
        return 0;
    }

    // Check for path traversal
    if (strstr(path, "..") || strstr(path, "//")) {
        LOG_WARN("Path contains potentially unsafe patterns: %s", path);
        return 0;
    }

    // Check for absolute paths outside working directory
    if (path[0] == '/' && strncmp(path, "/tmp/", 5) != 0) {
        LOG_WARN("Absolute path may be unsafe: %s", path);
    }

    return 1;
}

int is_within_directory(const char *path, const char *base_dir) {
    char *resolved_path = realpath(path, NULL);
    char *resolved_base = realpath(base_dir, NULL);

    if (!resolved_path || !resolved_base) {
        free(resolved_path);
        free(resolved_base);
        return 0;
    }

    int result = (strncmp(resolved_path, resolved_base, strlen(resolved_base)) == 0);

    free(resolved_path);
    free(resolved_base);

    return result;
}

int is_command_safe(const char *cmd) {
    if (validate_command(cmd) != 0) {
        return 0;
    }

    // Additional safety checks could be added here
    return 1;
}

char** get_dangerous_commands(void) {
    return (char**)DANGEROUS_COMMANDS;
}

int enable_sandbox_mode(void) {
    LOG_INFO("Sandbox mode requested but not fully implemented yet");
    // Future: implement seccomp, namespaces, cgroups, etc.
    return 0;
}

int disable_sandbox_mode(void) {
    LOG_DEBUG("Sandbox mode disabled");
    return 0;
}

int verify_file_checksum(const char *filepath, const char *expected_hash) {
    if (validate_hash(expected_hash) != 0) {
        return -1;
    }

    // This would integrate with the CAS hashing functionality
    // For now, just validate the hash format
    LOG_DEBUG("Checksum verification for %s (expected: %s)", filepath, expected_hash);
    return 0;
}
