#include "lockfile.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/file.h>

static int is_process_alive(pid_t pid) {
    return (kill(pid, 0) == 0);
}

LockFile* lockfile_acquire(const char *path) {
    LockFile *lock = calloc(1, sizeof(LockFile));
    strncpy(lock->path, path, sizeof(lock->path) - 1);
    lock->pid = getpid();

    // Create lock file
    lock->fd = open(path, O_CREAT | O_RDWR, 0644);
    if (lock->fd < 0) {
        LOG_ERROR("Failed to create lock file: %s", path);
        free(lock);
        return NULL;
    }

    // Try to acquire exclusive lock
    while (flock(lock->fd, LOCK_EX) != 0) {
        if (errno != EINTR) {
            LOG_ERROR("Failed to acquire lock: %s", strerror(errno));
            close(lock->fd);
            free(lock);
            return NULL;
        }
    }

    // Write PID to lock file
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d\n", lock->pid);
    if (ftruncate(lock->fd, 0) != 0 ||
        write(lock->fd, pid_str, strlen(pid_str)) != (ssize_t)strlen(pid_str)) {
        LOG_ERROR("Failed to write PID to lock file");
        flock(lock->fd, LOCK_UN);
        close(lock->fd);
        free(lock);
        return NULL;
    }

    lock->locked = 1;
    LOG_DEBUG("Lock acquired: %s (PID: %d)", path, lock->pid);

    return lock;
}

LockFile* lockfile_try_acquire(const char *path) {
    LockFile *lock = calloc(1, sizeof(LockFile));
    strncpy(lock->path, path, sizeof(lock->path) - 1);
    lock->pid = getpid();

    // Create lock file
    lock->fd = open(path, O_CREAT | O_RDWR, 0644);
    if (lock->fd < 0) {
        free(lock);
        return NULL;
    }

    // Try to acquire exclusive lock (non-blocking)
    if (flock(lock->fd, LOCK_EX | LOCK_NB) != 0) {
        close(lock->fd);
        free(lock);
        return NULL;
    }

    // Write PID
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d\n", lock->pid);
    ftruncate(lock->fd, 0);
    write(lock->fd, pid_str, strlen(pid_str));

    lock->locked = 1;
    LOG_DEBUG("Lock acquired (non-blocking): %s (PID: %d)", path, lock->pid);

    return lock;
}

void lockfile_release(LockFile *lock) {
    if (!lock) return;

    if (lock->locked) {
        flock(lock->fd, LOCK_UN);
        close(lock->fd);
        unlink(lock->path);
        LOG_DEBUG("Lock released: %s", lock->path);
    }

    free(lock);
}

pid_t lockfile_get_holder(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    pid_t pid = -1;
    fscanf(fp, "%d", &pid);
    fclose(fp);

    return pid;
}

int lockfile_is_locked(const char *path, pid_t *holder_pid) {
    // Try to open lock file
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return 0; // No lock file = not locked
    }

    // Try to acquire lock (non-blocking)
    if (flock(fd, LOCK_EX | LOCK_NB) == 0) {
        // We got the lock, so it wasn't locked
        flock(fd, LOCK_UN);
        close(fd);
        return 0;
    }

    // Get holder PID
    if (holder_pid) {
        *holder_pid = lockfile_get_holder(path);
    }

    close(fd);
    return 1;
}

int lockfile_force_remove(const char *path) {
    pid_t holder = lockfile_get_holder(path);

    if (holder > 0 && is_process_alive(holder)) {
        LOG_WARN("Lock holder (PID %d) is still alive, not removing", holder);
        return -1;
    }

    if (unlink(path) == 0) {
        LOG_INFO("Removed stale lock file: %s", path);
        return 0;
    }

    return -1;
}
