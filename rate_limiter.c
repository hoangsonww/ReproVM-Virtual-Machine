#include "rate_limiter.h"
#include "logger.h"
#include "metrics.h"
#include <stdlib.h>
#include <unistd.h>

ResourceLimits *g_resource_limits = NULL;

RateLimiter* rate_limiter_create(uint64_t capacity, uint64_t refill_rate) {
    RateLimiter *limiter = calloc(1, sizeof(RateLimiter));
    limiter->capacity = capacity;
    limiter->tokens = capacity;
    limiter->refill_rate = refill_rate;
    limiter->last_refill = time(NULL);
    limiter->enabled = 1;
    return limiter;
}

static void rate_limiter_refill(RateLimiter *limiter) {
    time_t now = time(NULL);
    time_t elapsed = now - limiter->last_refill;

    if (elapsed > 0) {
        uint64_t new_tokens = elapsed * limiter->refill_rate;
        limiter->tokens += new_tokens;
        if (limiter->tokens > limiter->capacity) {
            limiter->tokens = limiter->capacity;
        }
        limiter->last_refill = now;
    }
}

int rate_limiter_acquire(RateLimiter *limiter, uint64_t tokens) {
    if (!limiter->enabled) return 0;

    rate_limiter_refill(limiter);

    if (limiter->tokens >= tokens) {
        limiter->tokens -= tokens;
        return 0;
    }

    return -1; // Not enough tokens
}

int rate_limiter_wait(RateLimiter *limiter, uint64_t tokens, int timeout_ms) {
    if (!limiter->enabled) return 0;

    int waited = 0;
    while (rate_limiter_acquire(limiter, tokens) != 0) {
        if (waited >= timeout_ms) {
            LOG_WARN("Rate limit timeout after %d ms", waited);
            return -1;
        }

        usleep(100000); // 100ms
        waited += 100;
    }

    return 0;
}

void rate_limiter_free(RateLimiter *limiter) {
    free(limiter);
}

ResourceLimits* resource_limits_create(void) {
    ResourceLimits *limits = calloc(1, sizeof(ResourceLimits));

    // Set defaults
    limits->max_cpu_percent = 80;
    limits->max_memory_bytes = 4ULL * 1024 * 1024 * 1024; // 4GB
    limits->max_disk_bytes = 100ULL * 1024 * 1024 * 1024; // 100GB
    limits->max_concurrent_tasks = 16;
    limits->max_tasks_per_second = 100;
    limits->max_cache_ops_per_second = 1000;
    limits->task_timeout_seconds = 3600;
    limits->cache_timeout_seconds = 60;
    limits->enabled = 1;

    return limits;
}

int resource_limits_check(ResourceLimits *limits) {
    if (!limits->enabled) return 0;

    // Check CPU
    if (limits->current_cpu_percent > limits->max_cpu_percent) {
        LOG_WARN("CPU limit exceeded: %d%% > %d%%",
                 limits->current_cpu_percent, limits->max_cpu_percent);
        return -1;
    }

    // Check memory
    if (limits->current_memory_bytes > limits->max_memory_bytes) {
        LOG_WARN("Memory limit exceeded: %lu > %lu bytes",
                 limits->current_memory_bytes, limits->max_memory_bytes);
        return -1;
    }

    // Check disk
    if (limits->current_disk_bytes > limits->max_disk_bytes) {
        LOG_WARN("Disk limit exceeded: %lu > %lu bytes",
                 limits->current_disk_bytes, limits->max_disk_bytes);
        return -1;
    }

    // Check concurrent tasks
    if (limits->current_concurrent_tasks >= limits->max_concurrent_tasks) {
        LOG_DEBUG("Concurrent task limit reached: %d",
                  limits->max_concurrent_tasks);
        return -1;
    }

    return 0;
}

void resource_limits_update(ResourceLimits *limits) {
    if (!limits->enabled) return;

    limits->current_cpu_percent = (int)get_cpu_usage();
    limits->current_memory_bytes = get_memory_usage();

    LOG_DEBUG("Resource usage: CPU=%d%%, Memory=%lu bytes",
              limits->current_cpu_percent, limits->current_memory_bytes);
}

int resource_limits_wait_for_resources(ResourceLimits *limits, int timeout_ms) {
    if (!limits->enabled) return 0;

    int waited = 0;
    while (resource_limits_check(limits) != 0) {
        if (waited >= timeout_ms) {
            LOG_ERROR("Resource limit timeout after %d ms", waited);
            return -1;
        }

        resource_limits_update(limits);
        usleep(500000); // 500ms
        waited += 500;
    }

    return 0;
}

void resource_limits_free(ResourceLimits *limits) {
    free(limits);
}
