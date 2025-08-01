// main.c
#define _POSIX_C_SOURCE 200809L
#include "task.h"
#include "cas.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <manifest> [target1 target2 ...]\n", prog);
    fprintf(stderr, "Example manifest format:\n");
    fprintf(stderr, "task build {\n");
    fprintf(stderr, "  cmd = gcc -o hello hello.c\n");
    fprintf(stderr, "  inputs = hello.c\n");
    fprintf(stderr, "  outputs = hello\n");
    fprintf(stderr, "  deps =\n");
    fprintf(stderr, "}\n");
    fprintf(stderr, "task test {\n");
    fprintf(stderr, "  cmd = ./hello > result.txt\n");
    fprintf(stderr, "  inputs = hello\n");
    fprintf(stderr, "  outputs = result.txt\n");
    fprintf(stderr, "  deps = build\n");
    fprintf(stderr, "}\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }
    const char *manifest = argv[1];
    char **targets = NULL;
    int n_targets = 0;
    if (argc > 2) {
        n_targets = argc - 2;
        targets = &argv[2];
    }

    // Initialize CAS under current directory
    if (cas_init(".") != 0) {
        fprintf(stderr, "Failed to initialize CAS\n");
        return 1;
    }

    TaskList *list = parse_manifest(manifest);
    if (!list) {
        fprintf(stderr, "Failed to parse manifest\n");
        return 1;
    }
    // collect needed tasks
    int needed_n = 0;
    Task **needed = collect_needed_tasks(list, targets, n_targets, &needed_n);
    if (needed_n == 0) {
        fprintf(stderr, "No tasks to run.\n");
        free_tasklist(list);
        free(needed);
        return 0;
    }
    // topo sort
    int sorted_n = 0;
    Task **sorted = topo_sort(needed, needed_n, &sorted_n);
    if (!sorted) {
        fprintf(stderr, "Dependency cycle or topo sort failed.\n");
        free_tasklist(list);
        free(needed);
        return 1;
    }

    printf("Will execute %d tasks in order:\n", sorted_n);
    for (int i = 0; i < sorted_n; ++i) {
        printf("  %s\n", sorted[i]->name);
    }

    // Execute in order
    int overall_failed = 0;
    for (int i = 0; i < sorted_n; ++i) {
        Task *t = sorted[i];
        // Before computing its hash, ensure that its dependencies have their result_hash filled.
        // They should if they ran successfully or were cached.
        // Compute task hash
        if (!t->task_hash) {
            if (compute_task_hash(t) != 0) {
                t->status = STATUS_FAILED;
                overall_failed = 1;
                break;
            }
        }
        // Attempt execution
        int r = execute_task(t);
        print_task_graph(sorted, sorted_n);
        if (r != 0) {
            overall_failed = 1;
            break;
        }
    }

    if (overall_failed) {
        fprintf(stderr, "One or more tasks failed.\n");
        free_tasklist(list);
        free(needed);
        free(sorted);
        return 1;
    }

    printf("All tasks completed (some may have been cached). Final graph:\n");
    print_task_graph(sorted, sorted_n);

    free_tasklist(list);
    free(needed);
    free(sorted);
    return 0;
}
