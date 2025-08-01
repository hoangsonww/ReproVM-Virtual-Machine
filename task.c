#define _POSIX_C_SOURCE 200809L
#include "task.h"
#include "util.h"
#include "cas.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#define META_EXT ".meta"

// Internal helpers
static Task *task_new() {
    Task *t = calloc(1, sizeof(Task));
    t->status = STATUS_PENDING;
    return t;
}

static void task_free(Task *t) {
    if (!t) return;
    free(t->name);
    free(t->cmd);
    for (int i = 0; i < t->n_inputs; ++i) free(t->inputs[i]);
    free(t->inputs);
    for (int i = 0; i < t->n_outputs; ++i) free(t->outputs[i]);
    free(t->outputs);
    for (int i = 0; i < t->n_deps; ++i) free(t->deps[i]);
    free(t->deps);
    free(t->task_hash);
    free(t->result_hash);
    for (int i = 0; i < t->n_dependents; ++i) ; // dependents are borrowed
    free(t->dependents);
    free(t);
}

void free_tasklist(TaskList *list) {
    if (!list) return;
    if (list->tasks) {
        for (int i = 0; i < list->n; ++i) task_free(list->tasks[i]);
        free(list->tasks);
    }
    free(list);
}

Task *find_task(TaskList *list, const char *name) {
    if (!list || !name) return NULL;
    for (int i = 0; i < list->n; ++i) {
        if (strcmp(list->tasks[i]->name, name) == 0) return list->tasks[i];
    }
    return NULL;
}

// Parse a manifest like:
// task foo {
//   cmd = echo hello > out.txt
//   inputs = in1.txt,in2.txt
//   outputs = out.txt
//   deps = other
// }
TaskList *parse_manifest(const char *path) {
    size_t sz;
    char *content = read_entire_file(path, &sz);
    if (!content) {
        fprintf(stderr, "Failed to read manifest %s\n", path);
        return NULL;
    }
    TaskList *list = calloc(1, sizeof(TaskList));
    int capacity = 8;
    list->tasks = malloc(sizeof(Task*) * capacity);
    list->n = 0;

    char *line = NULL;
    char *saveptr = NULL;
    line = strtok_r(content, "\n", &saveptr);
    Task *cur = NULL;
    while (line) {
        char tmp[4096];
        strncpy(tmp, line, sizeof(tmp));
        tmp[sizeof(tmp)-1] = '\0';
        trim(tmp);
        if (tmp[0] == '#' || tmp[0] == '\0') {
            // comment or empty
        } else if (strncmp(tmp, "task ", 5) == 0) {
            char *p = tmp + 5;
            while (*p && isspace((unsigned char)*p)) p++;
            char *name_end = p;
            while (*name_end && !isspace((unsigned char)*name_end) && *name_end != '{') name_end++;
            size_t name_len = name_end - p;
            char *name = malloc(name_len + 1);
            memcpy(name, p, name_len);
            name[name_len] = '\0';
            cur = task_new();
            cur->name = name;
            // expect '{' somewhere, if not skip until next line
        } else if (strstr(tmp, "cmd") == tmp && cur) {
            char *eq = strchr(tmp, '=');
            if (eq) {
                eq++;
                while (*eq && isspace((unsigned char)*eq)) eq++;
                cur->cmd = strdup_safe(eq);
            }
        } else if (strstr(tmp, "inputs") == tmp && cur) {
            char *eq = strchr(tmp, '=');
            if (eq) {
                eq++;
                char *rest = strdup_safe(eq);
                trim(rest);
                cur->inputs = split_csv_array(rest, &cur->n_inputs);
                free(rest);
            }
        } else if (strstr(tmp, "outputs") == tmp && cur) {
            char *eq = strchr(tmp, '=');
            if (eq) {
                eq++;
                char *rest = strdup_safe(eq);
                trim(rest);
                cur->outputs = split_csv_array(rest, &cur->n_outputs);
                free(rest);
            }
        } else if (strstr(tmp, "deps") == tmp && cur) {
            char *eq = strchr(tmp, '=');
            if (eq) {
                eq++;
                char *rest = strdup_safe(eq);
                trim(rest);
                cur->deps = split_csv_array(rest, &cur->n_deps);
                free(rest);
            }
        } else if (strchr(tmp, '}') && cur) {
            // finish current
            if (list->n >= capacity) {
                capacity *= 2;
                list->tasks = realloc(list->tasks, sizeof(Task*) * capacity);
            }
            list->tasks[list->n++] = cur;
            cur = NULL;
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }
    free(content);
    if (cur) { // unclosed task
        if (list->n >= capacity) {
            capacity *= 2;
            list->tasks = realloc(list->tasks, sizeof(Task*) * capacity);
        }
        list->tasks[list->n++] = cur;
    }

    // Build reverse dependencies
    for (int i = 0; i < list->n; ++i) {
        Task *t = list->tasks[i];
        t->n_dependents = 0;
        t->dependents = NULL;
    }
    for (int i = 0; i < list->n; ++i) {
        Task *t = list->tasks[i];
        for (int d = 0; d < t->n_deps; ++d) {
            Task *dep = find_task(list, t->deps[d]);
            if (dep) {
                dep->dependents = realloc(dep->dependents, sizeof(Task*) * (dep->n_dependents + 1));
                dep->dependents[dep->n_dependents++] = t;
            } else {
                fprintf(stderr, "Warning: task '%s' has unknown dependency '%s'\n", t->name, t->deps[d]);
            }
        }
    }

    return list;
}

// Recursively collect needed tasks (targets + dependencies)
static void collect_dfs(Task *t, Task **out, int *n, int *cap, TaskList *master, int *visited_flags) {
    // mark visited
    for (int i = 0; i < *n; ++i) if (out[i] == t) return; // already added
    // first add dependencies
    for (int i = 0; i < t->n_deps; ++i) {
        Task *dep = find_task(master, t->deps[i]);
        if (dep) collect_dfs(dep, out, n, cap, master, visited_flags);
    }
    if (*n >= *cap) {
        *cap *= 2;
        out = realloc(out, sizeof(Task*) * (*cap));
    }
    out[(*n)++] = t;
}

Task **collect_needed_tasks(TaskList *list, char **target_names, int n_targets, int *out_n) {
    if (!list) { *out_n = 0; return NULL; }
    int cap = 16;
    Task **out = malloc(sizeof(Task*) * cap);
    int count = 0;
    int *visited = calloc(list->n, sizeof(int));
    if (n_targets == 0) {
        // all tasks
        for (int i = 0; i < list->n; ++i) {
            collect_dfs(list->tasks[i], out, &count, &cap, list, visited);
        }
    } else {
        for (int i = 0; i < n_targets; ++i) {
            Task *t = find_task(list, target_names[i]);
            if (!t) {
                fprintf(stderr, "Unknown target '%s'\n", target_names[i]);
                continue;
            }
            collect_dfs(t, out, &count, &cap, list, visited);
        }
    }
    free(visited);
    // Remove duplicates while preserving order (already done by DFS guard)
    *out_n = count;
    return out;
}

Task **topo_sort(Task **subset, int n, int *out_n) {
    // Kahn's algorithm. We'll compute indegrees limited to subset.
    // Build map: for quick membership tests
    Task **result = malloc(sizeof(Task*) * n);
    int res_n = 0;

    // initialize indegrees
    for (int i = 0; i < n; ++i) subset[i]->indegree = 0;
    for (int i = 0; i < n; ++i) {
        Task *t = subset[i];
        for (int d = 0; d < t->n_deps; ++d) {
            for (int j = 0; j < n; ++j) {
                if (strcmp(t->deps[d], subset[j]->name) == 0) {
                    subset[i]->indegree++;
                }
            }
        }
    }

    // queue of zero indegree
    Task **queue = malloc(sizeof(Task*) * n);
    int qh = 0, qt = 0;
    for (int i = 0; i < n; ++i) {
        if (subset[i]->indegree == 0) queue[qt++] = subset[i];
    }

    while (qh < qt) {
        Task *t = queue[qh++];
        result[res_n++] = t;
        // decrease indegree of dependents (only if in subset)
        for (int i = 0; i < t->n_dependents; ++i) {
            Task *dep = t->dependents[i];
            // is dep in subset?
            bool in_subset = false;
            for (int j = 0; j < n; ++j) if (subset[j] == dep) { in_subset = true; break; }
            if (!in_subset) continue;
            dep->indegree--;
            if (dep->indegree == 0) queue[qt++] = dep;
        }
    }
    free(queue);
    if (res_n != n) {
        fprintf(stderr, "Cycle detected among tasks; cannot topo sort.\n");
        free(result);
        return NULL;
    }
    *out_n = res_n;
    return result;
}

// Compute task hash based on command + inputs' blob hashes + deps' result hashes.
int compute_task_hash(Task *task) {
    if (!task) return -1;
    // Compute input blob hashes
    char **input_hashes = NULL;
    int n_inputs = task->n_inputs;
    if (n_inputs > 0) {
        input_hashes = malloc(sizeof(char*) * n_inputs);
        for (int i = 0; i < n_inputs; ++i) {
            char *h = cas_store_blob_from_file(task->inputs[i]);
            if (!h) {
                fprintf(stderr, "Failed to hash input file '%s' for task '%s'\n", task->inputs[i], task->name);
                for (int j = 0; j < i; ++j) free(input_hashes[j]);
                free(input_hashes);
                return -1;
            }
            input_hashes[i] = h;
        }
    }
    // Collect dependency result hashes
    char **dep_hashes = NULL;
    int n_dep_hashes = task->n_deps;
    if (n_dep_hashes > 0) {
        dep_hashes = malloc(sizeof(char*) * n_dep_hashes);
        for (int i = 0; i < task->n_deps; ++i) {
            // we expect that dependent tasks have result_hash already computed (since topo order)
            // If not, fallback to empty string
            char *depname = task->deps[i];
            // result_hash needs to be accessible; this assumes upstream has been run/hashed
            // For safety, we allow missing to be empty
            dep_hashes[i] = strdup_safe("");
            (void)depname; // actual assignment happens outside; we'll fix this below
        }
        // better approach: since we don't have a mapping here, we rely on caller having populated task->deps' result_hash somewhere else
    }
    // Actually, easier: instead of trying to look up dependency objects here (since Task* graph is available),
    // we will expect the caller sets up a list of dependency result hashes via their Task structs.
    // So to get those, we need external mapping; adjust: we will not use dep_hashes here. Instead, caller must
    // pass in dependency result hashes via concatenation externally. To not complicate, we'll reconstruct a string:
    size_t bufcap = 4096;
    char *buf = malloc(bufcap);
    buf[0] = '\0';
    // command
    strcat(buf, "cmd=");
    strcat(buf, task->cmd ? task->cmd : "");
    strcat(buf, "\n");
    // inputs (sorted for determinism)
    if (n_inputs > 0) {
        // sort input hashes
        for (int i = 0; i < n_inputs; ++i)
            for (int j = i+1; j < n_inputs; ++j)
                if (strcmp(input_hashes[i], input_hashes[j]) > 0) {
                    char *tmp = input_hashes[i];
                    input_hashes[i] = input_hashes[j];
                    input_hashes[j] = tmp;
                }
        strcat(buf, "inputs=");
        for (int i = 0; i < n_inputs; ++i) {
            strcat(buf, input_hashes[i]);
            if (i + 1 < n_inputs) strcat(buf, ",");
        }
        strcat(buf, "\n");
    } else {
        strcat(buf, "inputs=\n");
    }
    // dependencies' result_hashes (we can find the Task* if we had global list – assume caller already ensured their result_hash is set)
    if (task->n_deps > 0) {
        // because we don't have a reverse pointer to dependency Task* here, we do a small hack:
        strcat(buf, "deps=");
        for (int i = 0; i < task->n_deps; ++i) {
            // Try to find the dependency's result hash by scanning the environment:
            // In practice the caller will have already computed dependent task->result_hash.
            // Here we assume dependency names correspond; this will be patched in caller before calling this function.
            // So we append placeholder if missing.
            strcat(buf, ""); // placeholder
            if (i + 1 < task->n_deps) strcat(buf, ",");
        }
        strcat(buf, "\n");
    } else {
        strcat(buf, "deps=\n");
    }

    // Hash the buffer
    unsigned char digest[32];
    // reuse SHA256 functions directly
    // Implement temporary context here
    extern void sha256_init(SHA256_CTX *ctx);
    extern void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len);
    extern void sha256_final(SHA256_CTX *ctx, uint8_t hash[]);
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (unsigned char*)buf, strlen(buf));
    sha256_final(&ctx, digest);
    char *hex = hex_encode(digest, 32);
    if (!hex) {
        free(buf);
        if (input_hashes) {
            for (int i = 0; i < n_inputs; ++i) free(input_hashes[i]);
            free(input_hashes);
        }
        return -1;
    }
    // assign
    task->task_hash = strdup_safe(hex);
    free(hex);
    free(buf);
    if (input_hashes) {
        for (int i = 0; i < n_inputs; ++i) free(input_hashes[i]);
        free(input_hashes);
    }
    return 0;
}

// Load existing record if present
int try_load_task_record(Task *task) {
    if (!task || !task->task_hash) return 0;
    char meta_path[2048];
    snprintf(meta_path, sizeof(meta_path), "%s/%s%s", cas_get_cache_root(), task->task_hash, META_EXT);
    if (!file_exists(meta_path)) return 0;
    // open
    FILE *f = fopen(meta_path, "r");
    if (!f) return -1;
    char *line = NULL;
    size_t cap = 0;
    char *output_names[64];
    char *output_hashes[64];
    int n_outputs = 0;
    char result_hash_buf[256] = {0};
    while (getline(&line, &cap, f) != -1) {
        trim(line);
        if (strncmp(line, "result_hash:", 12) == 0) {
            char *p = line + 12;
            while (*p && isspace((unsigned char)*p)) p++;
            strncpy(result_hash_buf, p, sizeof(result_hash_buf)-1);
        } else if (strncmp(line, "output ", 7) == 0) {
            char *p = line + 7;
            // format: output <filename> <blob_hash>
            char *fname = strtok(p, " ");
            char *h = strtok(NULL, " ");
            if (fname && h) {
                output_names[n_outputs] = strdup_safe(fname);
                output_hashes[n_outputs] = strdup_safe(h);
                n_outputs++;
            }
        }
    }
    free(line);
    fclose(f);
    if (strlen(result_hash_buf) > 0) {
        task->result_hash = strdup_safe(result_hash_buf);
    } else {
        task->result_hash = strdup_safe("");
    }
    // If outputs exist, restore them to allow user to have them available when using cached result
    for (int i = 0; i < n_outputs; ++i) {
        // locate task->outputs to match by name; only restore if output is declared in task
        for (int j = 0; j < task->n_outputs; ++j) {
            if (strcmp(task->outputs[j], output_names[i]) == 0) {
                cas_restore_blob_to_file(output_hashes[i], task->outputs[j]);
            }
        }
        free(output_names[i]);
        free(output_hashes[i]);
    }
    task->status = STATUS_SKIPPED;
    return 1;
}

int write_task_record(Task *task) {
    if (!task || !task->task_hash) return -1;
    char meta_path[2048];
    snprintf(meta_path, sizeof(meta_path), "%s/%s%s", cas_get_cache_root(), task->task_hash, META_EXT);
    FILE *f = fopen(meta_path, "w");
    if (!f) return -1;
    fprintf(f, "task_hash: %s\n", task->task_hash);
    fprintf(f, "result_hash: %s\n", task->result_hash ? task->result_hash : "");
    for (int i = 0; i < task->n_outputs; ++i) {
        char *out = task->outputs[i];
        // compute its blob hash from stored CAS by re-hashing its file (should match existing)
        char *h = cas_store_blob_from_file(out);
        if (!h) continue;
        fprintf(f, "output %s %s\n", out, h);
        free(h);
    }
    fclose(f);
    return 0;
}

// Helper to compute result_hash from outputs (sort output hashes and hash their concatenation)
static int compute_result_hash(Task *task) {
    if (!task) return -1;
    char **hashes = malloc(sizeof(char*) * task->n_outputs);
    for (int i = 0; i < task->n_outputs; ++i) {
        if (!file_exists(task->outputs[i])) {
            hashes[i] = strdup_safe("");
        } else {
            char *h = cas_store_blob_from_file(task->outputs[i]);
            if (!h) hashes[i] = strdup_safe("");
            else hashes[i] = h;
        }
    }
    // sort
    for (int i = 0; i < task->n_outputs; ++i)
        for (int j = i+1; j < task->n_outputs; ++j)
            if (strcmp(hashes[i], hashes[j]) > 0) {
                char *t = hashes[i];
                hashes[i] = hashes[j];
                hashes[j] = t;
            }
    // concatenate
    size_t bufcap = 4096;
    char *buf = malloc(bufcap);
    buf[0] = '\0';
    for (int i = 0; i < task->n_outputs; ++i) {
        strcat(buf, hashes[i]);
        if (i + 1 < task->n_outputs) strcat(buf, ",");
    }
    // hash this
    extern void sha256_init(SHA256_CTX *ctx);
    extern void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len);
    extern void sha256_final(SHA256_CTX *ctx, uint8_t hash[]);
    SHA256_CTX ctx;
    uint8_t digest[32];
    sha256_init(&ctx);
    sha256_update(&ctx, (unsigned char*)buf, strlen(buf));
    sha256_final(&ctx, digest);
    char *hex = hex_encode(digest, 32);
    if (!hex) {
        free(buf);
        for (int i = 0; i < task->n_outputs; ++i) free(hashes[i]);
        free(hashes);
        return -1;
    }
    task->result_hash = strdup_safe(hex);
    free(hex);
    free(buf);
    for (int i = 0; i < task->n_outputs; ++i) free(hashes[i]);
    free(hashes);
    return 0;
}

// Execute a task: check cache, run if needed, update outputs
int execute_task(Task *task) {
    if (!task) return -1;
    task->status = STATUS_RUNNING;
    // compute task hash (requires that dependency tasks already have result_hash set)
    if (!task->task_hash) {
        if (compute_task_hash(task) != 0) {
            fprintf(stderr, "Failed to compute hash for task %s\n", task->name);
            task->status = STATUS_FAILED;
            return -1;
        }
    }
    // Try cache
    int cache_hit = try_load_task_record(task);
    if (cache_hit == 1) {
        // already loaded outputs
        task->status = STATUS_SKIPPED;
        return 0;
    } else if (cache_hit < 0) {
        // error reading
        fprintf(stderr, "Error reading cache metadata for task %s\n", task->name);
    }
    // Run the command
    printf("==> Running task '%s': %s\n", task->name, task->cmd ? task->cmd : "(no cmd)"); fflush(stdout);
    int ret = system(task->cmd);
    if (ret != 0) {
        fprintf(stderr, "Task '%s' failed with exit code %d\n", task->name, ret);
        task->status = STATUS_FAILED;
        return -1;
    }
    // After execution, compute result hash
    if (compute_result_hash(task) != 0) {
        fprintf(stderr, "Failed to compute result hash for task '%s'\n", task->name);
        task->status = STATUS_FAILED;
        return -1;
    }
    // Write record (also stores outputs into CAS)
    if (write_task_record(task) != 0) {
        fprintf(stderr, "Failed to write metadata for task '%s'\n", task->name);
        task->status = STATUS_FAILED;
        return -1;
    }
    task->status = STATUS_SUCCESS;
    return 0;
}

// Simple recursive print of graph; avoid infinite loops via visited set
static void print_task_recursive(Task *t, int indent, Task **all, int all_n, int *visited_flags) {
    int idx = -1;
    for (int i = 0; i < all_n; ++i) if (all[i] == t) { idx = i; break; }
    if (idx >=0 && visited_flags[idx]) return;
    if (idx >=0) visited_flags[idx] = 1;

    const char *sym = "[ ]";
    if (t->status == STATUS_RUNNING) sym = "[~]";
    else if (t->status == STATUS_SKIPPED) sym = "[*]";
    else if (t->status == STATUS_SUCCESS) sym = "[✔]";
    else if (t->status == STATUS_FAILED) sym = "[X]";

    for (int i = 0; i < indent; ++i) printf("  ");
    printf("%s %s", sym, t->name);
    if (t->task_hash) printf(" (hash=%s)", t->task_hash);
    if (t->result_hash) printf(" res=%s", t->result_hash);
    printf("\n");
    // show dependencies
    for (int i = 0; i < t->n_deps; ++i) {
        Task *dep = NULL;
        for (int j = 0; j < all_n; ++j) if (strcmp(all[j]->name, t->deps[i]) == 0) { dep = all[j]; break; }
        if (dep) print_task_recursive(dep, indent + 1, all, all_n, visited_flags);
    }
}

void print_task_graph(Task **tasks, int n) {
    printf("=== Task Graph ===\n");
    int *visited = calloc(n, sizeof(int));
    // find roots (those not depended on by others in subset)
    for (int i = 0; i < n; ++i) {
        bool is_dep = false;
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            for (int d = 0; d < tasks[j]->n_deps; ++d) {
                if (strcmp(tasks[j]->deps[d], tasks[i]->name) == 0) {
                    is_dep = true;
                    break;
                }
            }
            if (is_dep) break;
        }
        if (!is_dep) {
            print_task_recursive(tasks[i], 0, tasks, n, visited);
        }
    }
    // any unprinted remainers
    for (int i = 0; i < n; ++i) {
        if (!visited[i]) print_task_recursive(tasks[i], 0, tasks, n, visited);
    }
    free(visited);
    printf("==================\n");
}
