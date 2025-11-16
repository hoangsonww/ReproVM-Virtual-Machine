#ifndef LOCKFILE_H
#define LOCKFILE_H

#include <sys/types.h>

// Lock file handle
typedef struct {
    int fd;
    char path[512];
    pid_t pid;
    int locked;
} LockFile;

// Create and acquire lock
LockFile* lockfile_acquire(const char *path);

// Try to acquire lock (non-blocking)
LockFile* lockfile_try_acquire(const char *path);

// Release lock
void lockfile_release(LockFile *lock);

// Check if lock is held
int lockfile_is_locked(const char *path, pid_t *holder_pid);

// Force remove stale lock
int lockfile_force_remove(const char *path);

// Get lock holder PID
pid_t lockfile_get_holder(const char *path);

#endif // LOCKFILE_H
