#ifndef RATE_LIMITER_H
#define RATE_LIMITER_H

#include <time.h>
#include <stdint.h>

// Token bucket rate limiter
typedef struct {
    uint64_t capacity;      // Maximum tokens
    uint64_t tokens;        // Current tokens
    uint64_t refill_rate;   // Tokens per second
    time_t last_refill;     // Last refill time
    int enabled;
} RateLimiter;

// Resource limits
typedef struct {
    // CPU limits
    int max_cpu_percent;
    int current_cpu_percent;

    // Memory limits
    uint64_t max_memory_bytes;
    uint64_t current_memory_bytes;

    // Disk limits
    uint64_t max_disk_bytes;
    uint64_t current_disk_bytes;

    // Task limits
    int max_concurrent_tasks;
    int current_concurrent_tasks;

    // Rate limits
    int max_tasks_per_second;
    int max_cache_ops_per_second;

    // Timeouts
    int task_timeout_seconds;
    int cache_timeout_seconds;

    int enabled;
} ResourceLimits;

// Initialize rate limiter
RateLimiter* rate_limiter_create(uint64_t capacity, uint64_t refill_rate);

// Try to acquire tokens
int rate_limiter_acquire(RateLimiter *limiter, uint64_t tokens);

// Wait until tokens are available
int rate_limiter_wait(RateLimiter *limiter, uint64_t tokens, int timeout_ms);

// Free rate limiter
void rate_limiter_free(RateLimiter *limiter);

// Resource limits functions
ResourceLimits* resource_limits_create(void);
int resource_limits_check(ResourceLimits *limits);
void resource_limits_update(ResourceLimits *limits);
int resource_limits_wait_for_resources(ResourceLimits *limits, int timeout_ms);
void resource_limits_free(ResourceLimits *limits);

// Global resource limiter
extern ResourceLimits *g_resource_limits;

#endif // RATE_LIMITER_H
