// Dependency-aware parallel task executor for ReproVM. Works without modifying existing code.
// Usage: call execute_tasks_parallel(...) instead of serial loop.

#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "task.h"

// ready queue node
typedef struct ready_node {
    Task *task;
    struct ready_node *next;
} ready_node;

typedef struct {
    Task **tasks;          // subset
    int n;
    int *pending_deps;     // length n
    pthread_mutex_t mu;    // protects queue + counters + pending_deps + failed
    pthread_cond_t cv;     // signaled when new ready task or completion
    int remaining;         // tasks left to finish (success/skipped/failed)
    int failed;            // nonzero if any task failed
    ready_node *ready_head;
    ready_node *ready_tail;
    pthread_mutex_t print_mu; // serialize prints of graph
} parallel_ctx_t;

// forward
static int find_task_index(Task **tasks, int n, const char *name);

// push task into ready queue (caller holds mu)
static void push_ready(parallel_ctx_t *ctx, Task *t) {
    ready_node *node = malloc(sizeof(ready_node));
    node->task = t;
    node->next = NULL;
    if (!ctx->ready_head) {
        ctx->ready_head = ctx->ready_tail = node;
    } else {
        ctx->ready_tail->next = node;
        ctx->ready_tail = node;
    }
}

// pop a task from ready queue, returns NULL if empty (caller holds mu)
static Task *pop_ready(parallel_ctx_t *ctx) {
    if (!ctx->ready_head) return NULL;
    ready_node *node = ctx->ready_head;
    Task *t = node->task;
    ctx->ready_head = node->next;
    if (!ctx->ready_head) ctx->ready_tail = NULL;
    free(node);
    return t;
}

// Worker thread
static void *worker_main(void *arg) {
    parallel_ctx_t *ctx = arg;
    while (1) {
        pthread_mutex_lock(&ctx->mu);
        // wait for work or completion
        while (ctx->ready_head == NULL && ctx->remaining > 0) {
            pthread_cond_wait(&ctx->cv, &ctx->mu);
        }
        if (ctx->ready_head == NULL && ctx->remaining == 0) {
            pthread_mutex_unlock(&ctx->mu);
            break; // all done
        }
        Task *t = pop_ready(ctx);
        pthread_mutex_unlock(&ctx->mu);
        if (!t) continue;

        // Mark running
        t->status = STATUS_RUNNING;

        // Compute task hash if needed (dependencies' result_hashes should have been set already via ordering)
        if (!t->task_hash) {
            if (compute_task_hash(t) != 0) {
                fprintf(stderr, "Failed to compute hash for task %s\n", t->name);
                t->status = STATUS_FAILED;
                pthread_mutex_lock(&ctx->mu);
                ctx->failed = 1;
                ctx->remaining--;
                pthread_cond_broadcast(&ctx->cv);
                pthread_mutex_unlock(&ctx->mu);
                continue;
            }
        }

        // Execute (handles cache internally)
        int r = execute_task(t);
        if (r != 0) {
            // failure recorded
            t->status = STATUS_FAILED;
            pthread_mutex_lock(&ctx->mu);
            ctx->failed = 1;
            // continue to propagate dependents so they can see the failed state if desired
        }

        // After completion, update dependents
        pthread_mutex_lock(&ctx->mu);
        ctx->remaining--;
        // Print graph serialized to reduce interleaving
        pthread_mutex_lock(&ctx->print_mu);
        print_task_graph(ctx->tasks, ctx->n);
        pthread_mutex_unlock(&ctx->print_mu);

        for (int i = 0; i < t->n_dependents; ++i) {
            Task *dep = t->dependents[i];
            int idx = find_task_index(ctx->tasks, ctx->n, dep->name);
            if (idx < 0) continue; // not part of current subset
            // decrement pending
            ctx->pending_deps[idx]--;
            if (ctx->pending_deps[idx] == 0) {
                push_ready(ctx, dep);
                pthread_cond_broadcast(&ctx->cv);
            }
        }
        pthread_cond_broadcast(&ctx->cv);
        pthread_mutex_unlock(&ctx->mu);
    }
    return NULL;
}

// Linear search; small n expected.
static int find_task_index(Task **tasks, int n, const char *name) {
    for (int i = 0; i < n; ++i) {
        if (strcmp(tasks[i]->name, name) == 0) return i;
    }
    return -1;
}

/// Public API ///

/// Executes the given subset of tasks in parallel (respecting dependencies).
/// max_workers: number of threads; if <=0, uses heuristic (4).
/// Returns 0 on all-success or cache-skipped, nonzero if any task failed.
int execute_tasks_parallel(Task **subset, int n, int max_workers) {
    if (!subset || n == 0) return 0;
    if (max_workers <= 0) max_workers = 4;

    parallel_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.tasks = subset;
    ctx.n = n;
    ctx.pending_deps = calloc(n, sizeof(int));
    ctx.remaining = n;
    ctx.failed = 0;
    pthread_mutex_init(&ctx.mu, NULL);
    pthread_cond_init(&ctx.cv, NULL);
    pthread_mutex_init(&ctx.print_mu, NULL);
    ctx.ready_head = ctx.ready_tail = NULL;

    // Compute initial pending dependency counts (only dependencies within subset)
    for (int i = 0; i < n; ++i) {
        Task *t = subset[i];
        int cnt = 0;
        for (int d = 0; d < t->n_deps; ++d) {
            if (find_task_index(subset, n, t->deps[d]) >= 0) cnt++;
        }
        ctx.pending_deps[i] = cnt;
        if (cnt == 0) {
            push_ready(&ctx, t);
        }
    }

    // Spawn workers
    pthread_t *workers = malloc(sizeof(pthread_t) * max_workers);
    for (int i = 0; i < max_workers; ++i) {
        pthread_create(&workers[i], NULL, worker_main, &ctx);
    }

    // Wake up workers
    pthread_mutex_lock(&ctx.mu);
    pthread_cond_broadcast(&ctx.cv);
    pthread_mutex_unlock(&ctx.mu);

    // Join
    for (int i = 0; i < max_workers; ++i) {
        pthread_join(workers[i], NULL);
    }

    // Cleanup
    free(workers);
    free(ctx.pending_deps);
    // drain ready queue if any (should be empty)
    ready_node *cur = ctx.ready_head;
    while (cur) {
        ready_node *next = cur->next;
        free(cur);
        cur = next;
    }
    pthread_mutex_destroy(&ctx.mu);
    pthread_cond_destroy(&ctx.cv);
    pthread_mutex_destroy(&ctx.print_mu);

    return ctx.failed ? 1 : 0;
}
