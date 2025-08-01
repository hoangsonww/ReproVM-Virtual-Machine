# ReproVM - Reproducible Task Virtual Machine with Content-Addressed Caching

<p>
<!-- Core implementation -->
<img alt="C99" src="https://img.shields.io/badge/C99-language-blue?style=for-the-badge&logo=c&logoColor=white" />
<img alt="GCC" src="https://img.shields.io/badge/GCC-compiler-yellow?style=for-the-badge&logo=gnu&logoColor=white" />
<img alt="Make" src="https://img.shields.io/badge/Make-build-green?style=for-the-badge&logo=gnu-make&logoColor=white" />
<img alt="Bash" src="https://img.shields.io/badge/Shell-Bash-black?style=for-the-badge&logo=gnu-bash&logoColor=white" />
<img alt="POSIX" src="https://img.shields.io/badge/POSIX-compatible-lightgrey?style=for-the-badge&logo=unix&logoColor=white" />
<img alt="pthreads" src="https://img.shields.io/badge/pthreads-parallelism-purple?style=for-the-badge" />

<!-- Storage / hashing / semantics -->
<img alt="Content Addressed Storage" src="https://img.shields.io/badge/CAS-content--addressed_teal?style=for-the-badge&logo=database&logoColor=white" />
<img alt="SHA-256" src="https://img.shields.io/badge/SHA--256-hashing-brightgreen?style=for-the-badge" />
<img alt="CRC32" src="https://img.shields.io/badge/CRC32-checksum-blue?style=for-the-badge" />

<!-- Tooling & ecosystem -->
<img alt="Git" src="https://img.shields.io/badge/Git-version_control-orange?style=for-the-badge&logo=git&logoColor=white" />
<img alt="CLI" src="https://img.shields.io/badge/CLI-tooling-darkblue?style=for-the-badge&logo=console&logoColor=white" />
<img alt="Coreutils" src="https://img.shields.io/badge/Coreutils-UNIX_tools-grey?style=for-the-badge" />
<img alt="curl" src="https://img.shields.io/badge/curl-HTTP_client-blue?style=for-the-badge&logo=curl&logoColor=white" />
<img alt="tar" src="https://img.shields.io/badge/tar-archiving-brown?style=for-the-badge" />
<img alt="AWS CLI" src="https://img.shields.io/badge/AWS_CLI-automation-orange?style=for-the-badge&logo=amazonaws&logoColor=white" />
<img alt="S3" src="https://img.shields.io/badge/AWS_S3-storage-yellow?style=for-the-badge&logo=amazon-aws&logoColor=white" />

<!-- Polyglot extensions -->
<img alt="Go" src="https://img.shields.io/badge/Go-language-cyan?style=for-the-badge&logo=go&logoColor=white" />
<img alt="Rust" src="https://img.shields.io/badge/Rust-language-black?style=for-the-badge&logo=rust&logoColor=white" />
<img alt="Ruby" src="https://img.shields.io/badge/Ruby-language-red?style=for-the-badge&logo=ruby&logoColor=white" />
<img alt="Assembly (NASM)" src="https://img.shields.io/badge/Assembly-NASM-darkgrey?style=for-the-badge&logo=assemblyscript&logoColor=white" />

<!-- Data & services -->
<img alt="JSON" src="https://img.shields.io/badge/JSON-data-yellow?style=for-the-badge" />
<img alt="HTTP Server" src="https://img.shields.io/badge/http--server-static-blue?style=for-the-badge&logo=cloudflare&logoColor=white" />

<!-- Containerization -->
<img alt="Docker" src="https://img.shields.io/badge/Docker-container-blue?style=for-the-badge&logo=docker&logoColor=white" />
<img alt="Docker Compose" src="https://img.shields.io/badge/Docker_Compose-orchestration-236adb?style=for-the-badge&logo=docker&logoColor=white" />

<!-- UI / output -->
<img alt="ASCII UI" src="https://img.shields.io/badge/ASCII_UI-visualization-orange?style=for-the-badge" />

<!-- Platform & license -->
<img alt="Unix" src="https://img.shields.io/badge/Platform-Unix-black?style=for-the-badge&logo=linux&logoColor=white" />
<img alt="MIT License" src="https://img.shields.io/badge/License-MIT-blue?style=for-the-badge&logo=opensource&logoColor=white" />
</p>

## Table of Contents

1. [Overview](#overview)  
2. [Why ReproVM?](#why-reprovm)  
3. [Key Concepts](#key-concepts)  
   - [Key Technologies](#key-technologies)
4. [Quickstart](#quickstart)  
5. [Installation & Build](#installation--build)  
6. [Manifest Specification](#manifest-specification)  
7. [Task Lifecycle](#task-lifecycle)  
8. [Cache & CAS Internals](#cache--cas-internals)  
9. [CLI Usage Reference](#cli-usage-reference)  
10. [Environment & Configuration](#environment--configuration)  
11. [Simulated Realistic Sessions & Logs](#simulated-realistic-sessions--logs)  
12. [Advanced Usage Patterns](#advanced-usage-patterns)  
13. [Extension Points / Developer Notes](#extension-points--developer-notes)  
14. [Performance Tuning](#performance-tuning)  
15. [Security Considerations](#security-considerations)  
16. [Troubleshooting & Error Codes](#troubleshooting--error-codes)  
17. [FAQ](#faq)  
18. [Contributing](#contributing)  
19. [Glossary](#glossary)  
20. [Parallel Execution](#parallel-execution)
    - [What's Different](#whats-different)
    - [Building the Parallel Binary](#building-the-parallel-binary)
    - [Usage](#usage)
    - [Examples](#examples)
    - [Failure Behavior](#failure-behavior)
    - [Integration Notes](#integration-notes)
    - [Simulated Parallel Run Output (Realistic)](#simulated-parallel-run-output-realistic)
    - [Tuning](#tuning)
21. [Docker](#docker)
22. [Example File Layout After Successful Run](#example-file-layout-after-successful-run)
23. [License](#license)

## Overview

ReproVM is a lightweight yet powerful task execution virtual machine written entirely in C (C99). It is designed to make complex workflows — builds, data pipelines, test orchestrations, analyses — **reproducible**, **incremental**, and **efficient** through content-addressed caching and explicit dependency tracking. Think of it as a miniaturized, transparent cousin of modern build systems and pipeline orchestrators (e.g., parts of Docker, Bazel, or data lineage tooling) without hidden state or heavyweight dependencies.

It operates on a declarative manifest of tasks, each describing what to run, what files it consumes and produces, and what it depends on. ReproVM automatically computes identities (hashes), reuses work when possible, restores outputs, and visualizes the dependency graph with status in the terminal.

## Why ReproVM?

Real-world workflows suffer from repeated, wasted computation, hidden dependencies, and brittle pipelines. ReproVM addresses these problems:

- **Reproducibility**: Every piece of work is content-addressed; if inputs and commands are identical, outputs are reused deterministically.
- **Incrementality**: Only tasks whose inputs, commands, or upstream results have changed are rerun.
- **Transparency**: ASCII diagrams show exactly what ran, what was skipped, and why.
- **Portability**: No external runtime; pure C implementation, minimal assumptions.
- **Extensibility**: Clear separation of CAS, task graph, hashing, and execution logic makes it a foundation for more advanced systems.

## Key Concepts

- **Task**: Named unit of work with `cmd`, inputs, outputs, and declared dependencies.
- **Task Hash**: Identity of a task: derived from its command string, input blob hashes, and dependencies’ result hashes.
- **Result Hash**: Hash representing the outputs of a task; feeds into dependents for invalidation logic.
- **CAS (Content-Addressed Storage)**: Stores file blobs (inputs/outputs) keyed by their SHA-256 hash.
- **Cache Record**: Metadata per task hash stored under `.reprovm/cache/` that records what was produced, allowing skip on repeat.
- **Dependency Graph**: Directed acyclic graph (DAG) of tasks; ReproVM topologically orders and executes it.

### Key Technologies

- **C99 / GCC**: Core VM and logic implemented in portable, optimized C.
- **Make**: Build orchestration for building ReproVM itself.
- **Bash / POSIX shell**: Task commands run via shell; manifest authoring assumes standard shell semantics.
- **pthreads**: Optional parallel executor for dependency-aware concurrent task execution.
- **CAS + SHA-256**: Content-addressed storage and hashing form the backbone of caching and reproducibility.
- **Git**: Recommended for versioning manifests/source.
- **Coreutils (curl, tar, sha256sum)**: Used in example pipelines for data fetching, packaging, checksum generation.
- **Python**: Example extension for richer pipeline steps (e.g., data preprocessing).
- **ASCII UI / CLI**: Terminal-based visualization and control.
- **Unix compatibility**: Targeted at Unix-like environments.
- **MIT License**: Permissive open-source licensing.

## Quickstart

1. Write a manifest describing your tasks (see [Manifest Specification](#manifest-specification)).  
2. Build ReproVM: `make`  
3. Run the VM:  
   ```sh
   ./reprovm manifest.txt
   ```

If nothing has changed, subsequent runs are near-instant due to cache hits.

## Installation & Build

### Requirements

* POSIX-compatible OS (Linux, macOS, BSD)
* `gcc` or other C99-compatible compiler
* Standard UNIX shell available for task commands (`sh`, `bash` etc.)

### Build

```sh
git clone <your-repo> reprovm
cd reprovm
make
```

Expected output:

```
$ make
gcc -std=c99 -O2 -Wall -Wextra -g -c main.c -o main.o
gcc -std=c99 -O2 -Wall -Wextra -g -c task.c -o task.o
gcc -std=c99 -O2 -Wall -Wextra -g -c cas.c -o cas.o
gcc -std=c99 -O2 -Wall -Wextra -g -c util.c -o util.o
gcc -std=c99 -O2 -Wall -Wextra -g -o reprovm main.o task.o cas.o util.o
```

This produces the `reprovm` binary.

## Manifest Specification

ReproVM manifest is a simple, custom DSL. Whitespaces are flexible; comments begin with `#`.

### Grammar Summary

```
manifest      := { task_block }+
task_block   := "task" <name> "{" { field_line } "}"
field_line   := <key> "=" <value>
key          := "cmd" | "inputs" | "outputs" | "deps"
value        := arbitrary string (for cmd), or comma-separated list (for others)
```

### Valid Fields

* `cmd` — Shell command to execute. Should produce the declared outputs and respect inputs.
* `inputs` — Comma-separated list of file paths consumed by the task.
* `outputs` — Comma-separated list of file paths produced by the task.
* `deps` — Comma-separated list of other task names that must run before this one.

### Example

```txt
task build {
  cmd = gcc -o hello hello.c
  inputs = hello.c
  outputs = hello
  deps =
}

task test {
  cmd = ./hello > result.txt
  inputs = hello
  outputs = result.txt
  deps = build
}

task checksum {
  cmd = sha256sum result.txt > result.sha
  inputs = result.txt
  outputs = result.sha
  deps = test
}
```

### Notes

* Dependencies (`deps`) are used to control ordering beyond just file-based inference.
* Tasks with no dependencies can be listed with `deps =` or omitted after the equals (empty).

## Task Lifecycle

For each task in topological order:

1. **Compute Task Hash**

  * Concatenate its command, sorted input blob hashes, and dependencies’ result hashes.
  * SHA-256 of that describes the task identity.

2. **Cache Lookup**

  * Look for `.reprovm/cache/<task_hash>.meta`.
  * If present, restore outputs from CAS, mark as **skipped** (`[*]`).

3. **Execution** (if no cache hit)

  * Run the `cmd` via `system()`.
  * After success, compute result hash from its outputs and store each output into CAS.
  * Write `.meta` file containing task\_hash, result\_hash, and output-to-hash mapping.

4. **Graph Update & Display**

  * Update status and reprint ASCII dependency graph.

Statuses:

* `[ ]` pending
* `[~]` running
* `[*]` skipped (cache hit)
* `[✔]` success
* `[X]` failed

## Cache & CAS Internals

### CAS Layout

Blobs are stored under:

```
.reprovm/cas/objects/<first-two-hex>/<remaining-hash>
```

Example:

```
.reprovm/cas/objects/a3/f5d9c0e1b2...  # blob for a file or output
```

This two-level split avoids directory explosion.

### Metadata Record

Each task produces a metadata file:

```
.reprovm/cache/<task_hash>.meta
```

Example contents:

```
task_hash: a3f5d9c0e1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7
result_hash: 7d9a4f2e5b3c1a0d6e8f9b7c4d3e2f1a6b5c4d3e2f1a0b9c8d7e6f5a4b3c2
output hello 5e2f1a3b4c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1
output result.txt 9c4d7a1e2f3b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9
```

### Identity Propagation

* Downstream tasks include upstream result hashes in their own task hash, so any change propagates invalidation automatically.

## CLI Usage Reference

```
./reprovm <manifest> [target1 target2 ...]
```

* `<manifest>`: path to manifest file.
* `[target...]`: optional list of task names to build. If omitted, all tasks are considered targets.

### Examples

* Run full pipeline:

  ```sh
  ./reprovm manifest.txt
  ```

* Run specific target (and its dependencies):

  ```sh
  ./reprovm manifest.txt checksum
  ```

* Force re-computation of everything:

  ```sh
  rm -rf .reprovm
  ./reprovm manifest.txt
  ```

## Environment & Configuration

ReproVM does not require environment variables to function, but the following are recognized/usable for extension or debugging if you choose to augment it:

* `REPROVM_CACHE_DIR` (future extension) — override `.reprovm/cache` path.
* `REPROVM_VERBOSE=1` — hypothetical future verbose mode to log internal decisions.
* `REPROVM_FORCE=<task>` — (not implemented in base) could be used to bypass cache for a particular task.

The current implementation uses the project root (`.`) as base; you can change this by modifying `cas_init` parameter in `main.c`.

## Simulated Realistic Sessions & Logs

**Note:** The following outputs are realistic simulations, with plausible hash values and formatting.

### 1. First Full Run (cold cache)

Manifest: as above (`build`, `test`, `checksum`). `hello.c` prints “Hello, ReproVM!”.

```sh
$ ./reprovm manifest.txt
Will execute 3 tasks in order:
  build
  test
  checksum
==> Running task 'build': gcc -o hello hello.c
==> Task 'build' completed.
=== Task Graph ===
[✔] build (hash=a3f5d9c0e1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7) res=1f2e3d4c5b6a7980a9b8c7d6e5f4a3b2c1d0e9f8a7b6c5d4e3f2a1b0c9d8e7)
[ ] test (hash=5f6e7d8c9b0a1f2e3d4c5b6a7d8e9f0a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d) res=
[ ] checksum (hash=9a8b7c6d5e4f3a2b1c0d9e8f7a6b5c4d3e2f1a0b9c8d7e6f5a4b3c2d1e0f9a) res=
==================
==> Running task 'test': ./hello > result.txt
==> Task 'test' completed.
=== Task Graph ===
[✔] build (hash=...) res=...
[✔] test (hash=5f6e7d8c9b0a1f2e3d4c5b6a7d8e9f0a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d) res=3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c)
[ ] checksum (hash=9a8b7c6d5e4f3a2b1c0d9e8f7a6b5c4d3e2f1a0b9c8d7e6f5a4b3c2d1e0f9a) res=
==================
==> Running task 'checksum': sha256sum result.txt > result.sha
==> Task 'checksum' completed.
=== Task Graph ===
[✔] build (hash=...) res=...
[✔] test (hash=...) res=...
[✔] checksum (hash=9a8b7c6d5e4f3a2b1c0d9e8f7a6b5c4d3e2f1a0b9c8d7e6f5a4b3c2d1e0f9a) res=7f8e9d0c1b2a3e4f5d6c7b8a9f0e1d2c3b4a5f6e7d8c9b0a1c2d3e4f5a6b7c8)
==================
All tasks completed (some may have been cached). Final graph:
=== Task Graph ===
[✔] build ... 
[✔] test ...
[✔] checksum ...
==================
```

### 2. Repeat Run (warm cache)

```sh
$ ./reprovm manifest.txt
Will execute 3 tasks in order:
  build
  test
  checksum
[*] build (cache hit) (hash=a3f5d9c0e1b2c3d4...) res=1f2e3d4c...
[*] test (cache hit) (hash=5f6e7d8c9b0a1f2...) res=3c4d5e6f...
[*] checksum (cache hit) (hash=9a8b7c6d5e4f3a2b...) res=7f8e9d0c...
All tasks completed (some may have been cached). Final graph:
=== Task Graph ===
[*] build ...
[*] test ...
[*] checksum ...
==================
```

### 3. Change in Upstream Input (`hello.c` modified)

Modify `hello.c` to print “Hello, Updated ReproVM!” then:

```sh
$ ./reprovm manifest.txt
Will execute 3 tasks in order:
  build
  test
  checksum
==> Running task 'build': gcc -o hello hello.c
==> Task 'build' completed.
=== Task Graph ===
[✔] build (hash=de4f1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0) res=2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0e1f2a3)
[ ] test ...
[ ] checksum ...
==================
==> Running task 'test': ./hello > result.txt
==> Task 'test' completed.
=== Task Graph ===
[✔] build ...
[✔] test (hash=8c7d6e5f4a3b2c1d0e9f8a7b6c5d4e3f2a1b0c9d8e7f6a5b4c3d2e1f0a9b8c7) res=4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b3c4d)
[ ] checksum ...
==================
==> Running task 'checksum': sha256sum result.txt > result.sha
==> Task 'checksum' completed.
=== Task Graph ===
[✔] build ...
[✔] test ...
[✔] checksum ...
==================
All tasks completed (some may have been cached). Final graph:
...
```

### 4. Failure Case: Missing Input

Manifest has a typo: `inputs = missing.c` but `missing.c` does not exist.

```sh
$ ./reprovm manifest.txt
Will execute 3 tasks in order:
  build
  test
  checksum
==> Running task 'build': gcc -o hello missing.c
Task 'build' failed with exit code 1
=== Task Graph ===
[X] build (hash=...) res=
[ ] test ...
[ ] checksum ...
==================
One or more tasks failed.
```

## Advanced Usage Patterns

### Partial Target Execution

Run only a high-level result:

```sh
$ ./reprovm manifest.txt checksum
Will execute 3 tasks in order:
  build
  test
  checksum
[*] build (cached)
[*] test (cached)
[*] checksum (cached)
Final graph:
=== Task Graph ===
[*] build ...
[*] test ...
[*] checksum ...
==================
```

### Forcing a Rebuild

To ignore the cached result of a task:

```sh
$ rm .reprovm/cache/a3f5d9c0e1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7.meta
$ ./reprovm manifest.txt build
```

### Inspect Cache / Provenance

View metadata manually:

```sh
$ cat .reprovm/cache/a3f5d9c0e1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7.meta
task_hash: a3f5d9c0e1b2...
result_hash: 1f2e3d4c5b6a...
output hello 5e2f1a3b...
```

Restore any blob:

```sh
$ ./reprovm  # normal run will auto-restore, or use cas APIs to manually extract
```

---

## Example: More Complex Pipeline Manifest

```txt
task fetch_data {
  cmd = curl -s -o raw.csv https://example.com/data.csv
  inputs =
  outputs = raw.csv
  deps =
}

task clean {
  cmd = python3 scripts/clean.py raw.csv > cleaned.csv
  inputs = raw.csv
  outputs = cleaned.csv
  deps = fetch_data
}

task train {
  cmd = python3 scripts/train_model.py cleaned.csv model.bin
  inputs = cleaned.csv
  outputs = model.bin
  deps = clean
}

task evaluate {
  cmd = python3 scripts/evaluate.py model.bin cleaned.csv > metrics.json
  inputs = model.bin, cleaned.csv
  outputs = metrics.json
  deps = train
}

task bundle {
  cmd = tar -czf package.tar.gz model.bin metrics.json
  inputs = model.bin, metrics.json
  outputs = package.tar.gz
  deps = evaluate
}
```

Run the final bundle:

```sh
./reprovm pipeline_manifest.txt bundle
```

Re-running after only editing `clean.py` results in re-running `clean`, `train`, `evaluate`, and `bundle` — earlier cached steps (`fetch_data`) are reused.

## Extension Points / Developer Notes

* **Parallelism**: Current ordering is serial. Hooks exist for executing independent zero-indegree tasks concurrently.
* **Remote CAS**: The local CAS abstraction could be extended to push/pull blobs from remote storage (HTTP, S3, peer-to-peer).
* **Manifest Parsers**: Replace the ad-hoc parser with YAML/JSON frontends while emitting the same internal Task structures.
* **Custom Cache Policies**: Add TTL, manual invalidation, or signature-based validation.
* **Remote Execution**: Swap `system()` with RPC to remote workers.
* **Result Verification**: Integrate signature checking of cached outputs for tamper detection.

## Performance Tuning

* Use `-O2` or higher for building ReproVM itself (`Makefile` already uses `-O2`).
* Keep tasks fine-grained to maximize cache reuse.
* Avoid unnecessary outputs: declaring only real outputs prevents wasted hashing overhead.
* Batch small files if desired (could be an extension) to reduce CAS fragmentation.
* Consider adding in-memory memoization layer for extremely hot repeated task hash computations in long sessions.

## Security Considerations

* **Command Execution**: Tasks execute arbitrary shell commands. Only run manifests from trusted sources.
* **Cache Poisoning**: If multiple users share the `.reprovm` directory, ensure trust boundaries; consider signing metadata in a hardened variant.
* **Path Safety**: Manifest authors should avoid tasks that write outside expected output declarations to prevent confusion or overwrites.
* **Replay Attacks**: Without cryptographic signing, someone could replace CAS blob contents on disk. A secure variant would enforce integrity verification with signatures beyond hash existence.

## Troubleshooting & Error Codes

| Symptom / Error                       | Meaning                                         | Action                                                            |
| ------------------------------------- | ----------------------------------------------- | ----------------------------------------------------------------- |
| `Unknown target 'foo'`                | Specified target not defined                    | Verify spelling in manifest                                       |
| `Cycle detected among tasks`          | Dependency loop                                 | Break cycle by reordering or removing dependency                  |
| `Task 'X' failed with exit code N`    | Command returned nonzero                        | Inspect task `cmd`, check inputs, run manually for detailed error |
| Missing output but cache hit reported | Downstream expected file not declared as output | Ensure outputs reflect actual produced files                      |
| Corrupted `.meta` file or CAS object  | Cache metadata unreadable                       | Delete relevant `.meta` to force recompute                        |
| `Failed to hash input file`           | Input file missing or unreadable                | Confirm file exists and permissions are correct                   |

Exit codes:

* `0` — All tasks succeeded (possibly with skips).
* `1` — One or more errors (parsing, execution failure, cycle, internal error).

## FAQ

**Q: Can I use ReproVM inside CI to speed up repeated runs?**
A: Yes. Persist the `.reprovm/cas` and `.reprovm/cache` directories between pipeline runs to reuse cached steps and drastically reduce execution time.

**Q: What happens if I rename a file but its content is same?**
A: File identity in CAS is content-based, but the manifest refers to file *paths*. Renaming changes input/output names in the manifest, so tasks will rerun unless you create symlinks or adapt the manifest.

**Q: Can one task produce outputs not declared?**
A: The system won’t cache undeclared outputs properly. Always declare all side-effect files for reproducibility.

**Q: Is the cache sharable across machines?**
A: Out of the box, the CAS is local. You can copy `.reprovm/cas/objects` and `.reprovm/cache/` to another machine as long as relative paths and environment are compatible. For robust sharing, one could extend to a remote CAS backend.

## Contributing

Contributions are welcome. Suggested ways to help:

* Add parallel execution of independent tasks.
* Implement remote CAS backend (e.g., via HTTP/S3).
* Add manifest validation and richer syntax.
* Introduce a GUI or web visualization of task DAGs.
* Wrap ReproVM in other language bindings (Python, Rust) while reusing core logic.

Guidelines:

1. Fork the repository.
2. Create a feature branch.
3. Write tests or example manifests demonstrating your addition.
4. Submit a pull request with a clear description and any limitations.

## Glossary

* **Blob**: Arbitrary file contents stored in CAS, addressed by SHA-256 hash.
* **Task Hash**: Identity of a task; changes if its command, inputs, or upstream dependencies change.
* **Result Hash**: Digest representing the outputs of a task.
* **Cache Hit**: When a task's metadata already exists, and its outputs are restored instead of re-running.
* **DAG**: Directed Acyclic Graph of task dependencies.
* **CAS**: Content-Addressed Storage, deduplicating by content.

## Parallel Execution

ReproVM can be optionally extended to execute independent tasks in parallel while still respecting explicit dependencies. This is provided via a companion parallel executor and alternate entry point; it is **non-invasive** and does **not** require changes to the original codebase. If you don’t opt into parallel mode, the original serial behavior remains exactly the same.

### What's Different

- Tasks with no mutual dependency (i.e., whose prerequisites are satisfied and that don’t depend on each other) can run concurrently.
- The dependency DAG is observed: a task only becomes eligible when all of its upstream dependencies (within the current target set) have finished (successfully or been skipped).
- Failures are propagated: if any task fails, the overall run reports failure, but other currently runnable tasks may still finish to give more visibility before termination.
- ASCII graph updates still happen after each task completion; the output is serialized to reduce interleaving so you can watch the evolving state even under concurrency.

### Building the Parallel Binary

The parallel version lives alongside the regular `reprovm` binary. Compile it by adding the new sources and linking pthreads:

```sh
gcc -std=c99 -O2 -Wall -Wextra -g reprovm_parallel.c parallel_executor.c task.c cas.c util.c -lpthread -o reprovm_parallel
````

You can keep both versions:

```sh
gcc -std=c99 -O2 -Wall -Wextra -g main.c task.c cas.c util.c -o reprovm
gcc -std=c99 -O2 -Wall -Wextra -g reprovm_parallel.c parallel_executor.c task.c cas.c util.c -lpthread -o reprovm_parallel
```

### Usage

```
./reprovm_parallel [-j N] <manifest> [target1 target2 ...]
```

* `-j N` / `--jobs N`: number of worker threads to use. If omitted, it defaults to the number of online CPUs (fallbacking to 4).
* Manifest and target semantics are identical to the serial version; dependencies are resolved automatically.

You can also influence parallelism via environment variable (future extension support):

```sh
export REPROVM_JOBS=8   # if implemented, can be read as default parallelism
```

### Examples

#### Full Parallel Run (auto worker count)

```sh
$ ./reprovm_parallel manifest.txt
Will execute 3 tasks (parallel workers: 8)
==> Scheduling...
[~] build (hash=...) res=
[~] test  (waiting on build)
[~] checksum (waiting on test)

==> Running task 'build': gcc -o hello hello.c
==> Running task 'test': ./hello > result.txt       # if build finished early enough and test became eligible
[✔] build ...
[*] test  (cache hit or skipped if unchanged)
[ ] checksum ...
...
```

#### Specifying 4 Workers

```sh
$ ./reprovm_parallel -j 4 manifest.txt
Will execute 3 tasks (parallel workers: 4)
==> Running task 'build': gcc -o hello hello.c
==> Task 'build' completed.
=== Task Graph ===
[✔] build ...
[ ] test ...
[ ] checksum ...
==================
==> Running task 'test': ./hello > result.txt
==> Task 'test' completed.
=== Task Graph ===
[✔] build ...
[✔] test ...
[ ] checksum ...
==================
==> Running task 'checksum': sha256sum result.txt > result.sha
==> Task 'checksum' completed.
=== Task Graph ===
[✔] build ...
[✔] test ...
[✔] checksum ...
==================
All tasks completed (some may have been cached). Final graph:
=== Task Graph ===
[✔] build ...
[✔] test ...
[✔] checksum ...
==================
```

#### Mixed Parallelism with Independent Tasks

Given a manifest with two independent tasks `A` and `B` that both feed into `C`, they can run concurrently:

```
Will execute 3 tasks (parallel workers: 4)
==> Running task 'A': ...
==> Running task 'B': ...
[✔] A ...
[✔] B ...
==> Running task 'C': ...
[✔] C ...
Final graph:
[✔] A ...
[✔] B ...
[✔] C ...
```

### Failure Behavior

If one worker encounters a failure (non-zero exit), the failure is recorded but other in-flight eligible tasks are allowed to finish so you get a full snapshot. The final exit code is non-zero, and the ASCII graph will show `[X]` for failed tasks.

### Integration Notes

* The parallel executor uses the same task hashing, cache lookup, execution, and graph-printing logic.
* No modifications are required in existing source files—just use `reprovm_parallel` when you want concurrency.
* You can mix: use the serial `./reprovm` for debugging or simple runs, and the parallel `./reprovm_parallel -j N` for performance on larger DAGs.

### Simulated Parallel Run Output (Realistic)

```sh
$ ./reprovm_parallel -j 3 manifest.txt
Will execute 3 tasks (parallel workers: 3)
==> Scheduling and starting workers...
[~] build (running)         [ ] test (waiting)        [ ] checksum (waiting)
==> Running task 'build': gcc -o hello hello.c
[✔] build (completed)
=== Task Graph ===
[✔] build (hash=...) res=...
[ ] test  (ready) res=
[ ] checksum (waiting on test)
==================
==> Running task 'test': ./hello > result.txt
[~] test (running)
[ ] checksum (waiting on test)
[✔] test (completed)
=== Task Graph ===
[✔] build ...
[✔] test ...
[ ] checksum ...
==================
==> Running task 'checksum': sha256sum result.txt > result.sha
[✔] checksum (completed)
=== Task Graph ===
[✔] build ...
[✔] test ...
[✔] checksum ...
==================
All tasks completed (some may have been cached). Final graph:
=== Task Graph ===
[✔] build ...
[✔] test ...
[✔] checksum ...
==================
```

If a task is a cache hit and skipped, it shows as `[*]` and its dependents may immediately become eligible without delay.

### Tuning

* Use `-j` to control throughput; too many threads on small DAGs may add scheduling overhead, so match worker count to workload size.
* Because output rendering is serialized, you get consistent task graph snapshots even under concurrency.

Here’s a **Docker section** you can insert into the README (e.g., right after **Installation & Build** or before **Manifest Specification**):

## Docker

ReproVM can be run inside a container for reproducible, isolated execution environments. The provided `Dockerfile` and `docker-compose.yml` (see repo) bundle all required tools (GCC, make, Python, Node.js, Ruby, etc.), build both serial and parallel binaries, and give you a consistent runtime.

### Overview

- Container encapsulates build/runtime dependencies: C toolchain, shell, optional polyglot helpers (Go, Rust, Node.js, Ruby), and ReproVM itself.
- Workspace is mounted from the host so manifests, source files, and the `.reprovm` cache/CAS are shared (and persist across container invocations).
- Parallelism is exposed via the `reprovm_parallel` binary and optionally controlled with `-j` or environment variable `REPROVM_JOBS`.

### Building the Image

From the repo root:

```sh
docker build -t reprovm:latest .
````

This builds the image, compiling ReproVM (serial + parallel) and preparing ancillary scripts.

### Running ReproVM with Docker

Run the pipeline against your manifest with:

```sh
docker run --rm -v "$(pwd)":/workspace -w /workspace reprovm:latest ./run_pipeline.sh manifest.txt
```

Or invoke the parallel binary directly (auto-detects CPU count unless overridden):

```sh
docker run --rm -v "$(pwd)":/workspace -w /workspace reprovm:latest ./reprovm_parallel -j 4 manifest.txt
```

To pass parallelism via environment variable:

```sh
docker run --rm -v "$(pwd)":/workspace -w /workspace -e REPROVM_JOBS=8 reprovm:latest ./run_pipeline.sh manifest.txt
```

### Using docker-compose

With the included `docker-compose.yml`, you can spin up the ReproVM service easily:

```sh
docker compose up reprovm
```

This mounts the current directory into `/workspace`, builds/uses the image, and runs `./run_pipeline.sh` by default. To target a specific manifest or override:

```sh
docker compose run --rm reprovm ./reprovm_parallel -j 2 pipeline_manifest.txt bundle
```

### Persisting Cache

Since `.reprovm` lives in the mounted workspace, cache and CAS entries survive between container runs. To maximize CI or repeated-run performance, persist the host-side `.reprovm` directory (e.g., in CI artifacts or volume-backed storage).

### Examples

Cold build:

```sh
docker run --rm -v "$(pwd)":/workspace -w /workspace reprovm:latest ./reprovm_parallel manifest.txt
```

Warm rebuild (uses cache):

```sh
docker run --rm -v "$(pwd)":/workspace -w /workspace reprovm:latest ./reprovm_parallel manifest.txt
```

Run a specific target:

```sh
docker run --rm -v "$(pwd)":/workspace -w /workspace reprovm:latest ./reprovm_parallel manifest.txt checksum
```

Shell into container for inspection/debugging:

```sh
docker run --rm -it -v "$(pwd)":/workspace -w /workspace reprovm:latest bash
# then inside:
./reprovm_parallel -j 4 manifest.txt
```

### Cleanup

To remove the built image when you no longer need it:

```sh
docker image rm reprovm:latest
```

If using docker-compose, tear down (and optionally remove volumes):

```sh
docker compose down
```

### Tips

* **Bind mount the workspace** so intermediate outputs, manifests, and the cache are visible/editable from the host.
* **Use the same `.reprovm` directory** across container invocations for maximum cache reuse.
* **Override commands** by replacing the entrypoint/command in `docker run` or `docker-compose` if you need to run custom diagnostics (e.g., inspect `.reprovm/cache` contents).
* **CI integration:** Build the image once, mount the repo, run the manifest, and persist `.reprovm` between jobs to drastically cut repeat run time.

## Example File Layout After Successful Run

```
.
├── hello.c
├── manifest.txt
├── hello                    # compiled binary
├── result.txt              # program output
├── result.sha             # checksum
├── reprovm                # compiled VM executable
└── .reprovm
    ├── cache
    │   ├── a3f5d9c0e1b2...meta
    │   ├── 5f6e7d8c9b0a...meta
    │   └── 9a8b7c6d5e4f...meta
    └── cas
        └── objects
            ├── a3/
            │   └── f5d9c0e1b2c3d4...  # blobs: e.g., input hello.c
            ├── 5f/
            │   └── 6e7d8c9b0a1f2e...  # more blobs
            └── 9a/
                └── 8b7c6d5e4f3a2b...  # output blobs
```

## License

MIT License. See the [LICENSE](LICENSE) file for details.

---

Thank you for using ReproVM! We hope it helps you build reproducible, efficient workflows with ease. If you have any questions or suggestions, feel free to open an issue or contribute to the project.
