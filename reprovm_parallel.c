// Alternate entry point that enables parallel execution if requested.
// Compatible with existing code; does not require modifying original files.

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include "task.h"
#include "cas.h"
#include "util.h"

// Declaration from parallel_executor.c
int execute_tasks_parallel(Task **subset, int n, int max_workers);

void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s [-j N] <manifest> [target1 target2 ...]\n"
            "  -j N    number of parallel workers (default: autodetect or 4)\n"
            "Example:\n"
            "  %s -j 8 manifest.txt build test\n",
            prog, prog);
}

static int get_cpu_count(void) {
#if defined(_SC_NPROCESSORS_ONLN)
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    if (n > 0) return (int)n;
#endif
    return 4;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    int max_workers = 0;
    int argi = 1;
    // parse -j
    if (argc >= 3 && (strcmp(argv[1], "-j") == 0 || strcmp(argv[1], "--jobs") == 0)) {
        max_workers = atoi(argv[2]);
        if (max_workers <= 0) {
            fprintf(stderr, "Invalid worker count '%s'\n", argv[2]);
            return 1;
        }
        argi += 2;
    } else {
        max_workers = get_cpu_count();
    }

    if (argi >= argc) {
        usage(argv[0]);
        return 1;
    }
    const char *manifest = argv[argi++];
    char **targets = NULL;
    int n_targets = 0;
    if (argi < argc) {
        n_targets = argc - argi;
        targets = &argv[argi];
    }

    if (cas_init(".") != 0) {
        fprintf(stderr, "Failed to initialize CAS\n");
        return 1;
    }

    TaskList *list = parse_manifest(manifest);
    if (!list) {
        fprintf(stderr, "Failed to parse manifest '%s'\n", manifest);
        return 1;
    }

    int needed_n = 0;
    Task **needed = collect_needed_tasks(list, targets, n_targets, &needed_n);
    if (needed_n == 0) {
        fprintf(stderr, "No tasks to run.\n");
        free_tasklist(list);
        free(needed);
        return 0;
    }

    printf("Will execute %d tasks (parallel workers: %d)\n", needed_n, max_workers);

    int result = execute_tasks_parallel(needed, needed_n, max_workers);
    if (result != 0) {
        fprintf(stderr, "One or more tasks failed.\n");
    } else {
        printf("All tasks completed (some may have been cached). Final graph:\n");
        print_task_graph(needed, needed_n);
    }

    free_tasklist(list);
    free(needed);
    return result;
}
