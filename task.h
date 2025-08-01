// task.h
#ifndef TASK_H
#define TASK_H

#include <stdbool.h>

typedef enum {
    STATUS_PENDING,
    STATUS_RUNNING,
    STATUS_SKIPPED, // cache hit
    STATUS_SUCCESS,
    STATUS_FAILED
} task_status_t;

typedef struct Task {
    char *name;
    char *cmd;
    char **inputs;
    int n_inputs;
    char **outputs;
    int n_outputs;
    char **deps;
    int n_deps;

    // content hashes
    char *task_hash;       // computed from cmd + input hashes + deps' result hashes
    char *result_hash;     // derived from outputs (after run)

    task_status_t status;

    // internal adjacency bookkeeping for topo sort
    int indegree;
    struct Task **dependents; // reverse edges
    int n_dependents;
} Task;

typedef struct {
    Task **tasks;
    int n;
} TaskList;

// Parse manifest file into TaskList. Returns NULL on failure.
TaskList *parse_manifest(const char *path);

// Free TaskList
void free_tasklist(TaskList *list);

// Find task by name (NULL if not found)
Task *find_task(TaskList *list, const char *name);

// Compute closure of named targets including dependencies. Returns array of Task* (malloced), count via out_n.
// Caller must free the returned array (not the tasks).
Task **collect_needed_tasks(TaskList *list, char **target_names, int n_targets, int *out_n);

// Topologically sort given subset of tasks (array of Task*, count). Returns array in-order (new array), or NULL on cycle error.
Task **topo_sort(Task **subset, int n, int *out_n);

// Compute the task hash for a task (fills task_hash). Returns 0 on success.
int compute_task_hash(Task *task);

// Try loading a cached record for the task. If present, populates task->result_hash and leaves outputs mapping accessible via metadata.
// Returns 1 if cache hit loaded, 0 if no cache, -1 on error.
int try_load_task_record(Task *task);

// Write task record after successful run (stores outputs and result_hash). Returns 0 on success.
int write_task_record(Task *task);

// Execute a task, respecting cache. Returns 0 on success, nonzero on failure.
int execute_task(Task *task);

// Print dependency/status diagram for a set of tasks (roots inferred).
void print_task_graph(Task **tasks, int n);

#endif
