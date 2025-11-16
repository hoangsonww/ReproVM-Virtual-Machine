# ReproVM Production-Ready Features

This document describes the production-ready enhancements added to ReproVM.

## Table of Contents

1. [Overview](#overview)
2. [Logging System](#logging-system)
3. [Configuration Management](#configuration-management)
4. [Metrics & Monitoring](#metrics--monitoring)
5. [Error Handling](#error-handling)
6. [Security Features](#security-features)
7. [Signal Handling](#signal-handling)
8. [Health Checks](#health-checks)
9. [CI/CD Pipeline](#cicd-pipeline)
10. [Testing](#testing)
11. [Deployment](#deployment)

## Overview

ReproVM has been enhanced with enterprise-grade features to make it truly production-ready:

- **Comprehensive Logging**: Multi-level logging with file and console output
- **Configuration Management**: File-based and environment variable configuration
- **Metrics Collection**: Detailed performance and resource usage tracking
- **Enhanced Error Handling**: Proper error codes, context, and recovery mechanisms
- **Security Hardening**: Input validation, path safety, and permission checks
- **Signal Handling**: Graceful shutdown and crash reporting
- **Health Checks**: System health monitoring and diagnostics
- **CI/CD Integration**: Automated testing, building, and deployment
- **Comprehensive Testing**: Unit tests, integration tests, and memory leak detection

## Logging System

### Features

- Multiple log levels: DEBUG, INFO, WARN, ERROR, FATAL
- Colored console output (can be disabled)
- File logging with timestamps
- Thread-safe logging
- Location tracking (file, line, function)

### Usage

```c
#include "logger.h"

// Initialize logger
logger_init(".reprovm/reprovm.log", LOG_INFO, 1);

// Log messages
LOG_DEBUG("Debug information: %d", value);
LOG_INFO("Operation completed successfully");
LOG_WARN("Resource usage is high: %.2f%%", usage);
LOG_ERROR("Failed to open file: %s", filename);
LOG_FATAL("Critical error, exiting");  // Exits program

// Cleanup
logger_close();
```

### Configuration

Set log level via environment variable:
```bash
export REPROVM_LOG_LEVEL=DEBUG
export REPROVM_LOG_FILE=/var/log/reprovm.log
export REPROVM_NO_COLOR=1  # Disable colored output
```

## Configuration Management

### Features

- Default configuration with sensible values
- File-based configuration (.conf format)
- Environment variable overrides
- Configuration validation
- Runtime configuration inspection

### Configuration File

Create `.reprovm/reprovm.conf`:

```ini
# Logging
log_file=.reprovm/reprovm.log
log_level=INFO

# Cache
cache_dir=.reprovm
max_cache_size_mb=10240

# Execution
parallel_jobs=4
retry_attempts=3
timeout_seconds=3600

# Performance
enable_metrics=1

# Remote CAS (optional)
remote_cas_url=https://your-cas-server.com
```

### Environment Variables

```bash
# Logging
REPROVM_LOG_LEVEL=DEBUG
REPROVM_LOG_FILE=/var/log/reprovm.log

# Execution
REPROVM_JOBS=8
REPROVM_RETRY_ATTEMPTS=5
REPROVM_TIMEOUT=7200

# Cache
REPROVM_CACHE_DIR=/tmp/reprovm-cache

# Remote CAS
REPROVM_REMOTE_CAS_URL=https://cas.example.com
REPROVM_REMOTE_CAS_ACCESS_KEY=your_access_key
REPROVM_REMOTE_CAS_SECRET_KEY=your_secret_key

# Features
REPROVM_VERBOSE=1
REPROVM_FORCE=1
REPROVM_NO_COLOR=1
REPROVM_NO_PROGRESS=1
```

## Metrics & Monitoring

### Features

- Task execution metrics (count, timing, cache hits)
- Resource usage tracking (CPU, memory)
- Cache performance statistics
- Error and retry counting
- JSON export for external monitoring systems

### Metrics Collection

Metrics are automatically collected during execution:

```c
#include "metrics.h"

// Initialize metrics
metrics_init();

// Record task execution
metrics_task_start("compile");
// ... execute task ...
metrics_task_end("compile", cached, failed, duration_ms);

// Record events
metrics_record_cache_hit();
metrics_record_error();
metrics_record_retry();

// Generate report
metrics_calculate_stats();
metrics_print_summary();
metrics_write_to_file(".reprovm/metrics.log");
metrics_write_json(".reprovm/metrics.json");
```

### Metrics Output

Console summary:
```
╔════════════════════════════════════════════════════════════════╗
║                    REPROVM METRICS SUMMARY                     ║
╠════════════════════════════════════════════════════════════════╣
║ Execution Summary:                                             ║
║   Total Tasks:        15                                       ║
║   Tasks Executed:     8                                        ║
║   Tasks Cached:       7                                        ║
║   Tasks Failed:       0                                        ║
║   Elapsed Time:       12.34 s                                  ║
╠════════════════════════════════════════════════════════════════╣
║ Cache Performance:                                             ║
║   Cache Hits:         7                                        ║
║   Cache Misses:       8                                        ║
║   Cache Hit Rate:     46.67%                                   ║
╚════════════════════════════════════════════════════════════════╝
```

JSON export (.reprovm/metrics.json):
```json
{
  "timestamp": 1699564800,
  "elapsed_seconds": 12.34,
  "tasks": {
    "total": 15,
    "executed": 8,
    "cached": 7,
    "failed": 0
  },
  "cache": {
    "hits": 7,
    "misses": 8,
    "hit_rate": 0.4667
  },
  "timing_ms": {
    "total": 12340.0,
    "average": 822.67,
    "min": 45.23,
    "max": 3456.78
  }
}
```

## Error Handling

### Error Codes

```c
typedef enum {
    ERR_SUCCESS = 0,
    ERR_INVALID_ARG = 1,
    ERR_FILE_NOT_FOUND = 2,
    ERR_FILE_IO = 3,
    ERR_MEMORY_ALLOC = 4,
    ERR_PARSE_ERROR = 5,
    ERR_TASK_FAILED = 6,
    ERR_CYCLE_DETECTED = 7,
    ERR_CACHE_ERROR = 8,
    ERR_CAS_ERROR = 9,
    ERR_NETWORK_ERROR = 10,
    ERR_TIMEOUT = 11,
    ERR_PERMISSION_DENIED = 12,
    ERR_INVALID_HASH = 13,
    ERR_CONFIG_ERROR = 14,
    ERR_SYSTEM_ERROR = 15
} ErrorCode;
```

### Usage

```c
#include "error_handling.h"

// Set error with context
SET_ERROR(ERR_FILE_NOT_FOUND, "Cannot find manifest: %s", filename);

// Get last error
ErrorContext *err = get_last_error();
if (err->code != ERR_SUCCESS) {
    print_error(err);
}

// Retry mechanism
int result = retry_operation(my_function, context, max_attempts, delay_ms);

// Check macros
CHECK_NULL(ptr, "Failed to allocate memory for %s", name);
CHECK_ERROR(fd >= 0, ERR_FILE_IO, "Cannot open file: %s", path);
```

## Security Features

### Input Validation

```c
#include "security.h"

// Validate inputs
if (validate_file_path(path) != 0) {
    // Invalid path
}

if (validate_task_name(name) != 0) {
    // Invalid task name
}

if (validate_command(cmd) != 0) {
    // Invalid or dangerous command
}

if (validate_hash(hash) != 0) {
    // Invalid hash format
}
```

### Path Safety

```c
// Sanitize and check paths
char *safe_path = sanitize_path(user_input);
if (is_path_safe(safe_path)) {
    // Path is safe to use
}

// Check if path is within allowed directory
if (is_within_directory(path, base_dir)) {
    // Path is confined to base directory
}
```

### Permission Checks

```c
// Check file permissions
if (check_file_permissions(path, 1) == 0) {
    // File is readable and writable
}

// Check directory permissions
if (check_directory_permissions(path) == 0) {
    // Directory is accessible
}

// Check file size limit
if (check_file_size_limit(path, max_size) == 0) {
    // File is within size limit
}
```

## Signal Handling

### Features

- Graceful shutdown on SIGINT/SIGTERM
- Crash reporting on SIGSEGV/SIGABRT
- Stack trace printing
- Automatic metrics saving on exit

### Usage

```c
#include "signal_handler.h"

// Setup signal handlers
setup_signal_handlers();

// Check if shutdown was requested
if (is_shutdown_requested()) {
    // Cleanup and exit
    graceful_shutdown(0);
}

// Register custom callback
void my_handler(int signum) {
    printf("Custom handler for signal %d\n", signum);
}
register_signal_callback(SIGUSR1, my_handler);
```

## Health Checks

### Features

- System health monitoring
- Cache health checks
- CAS health checks
- Disk usage monitoring
- Memory usage tracking
- JSON health status export

### Usage

```c
#include "health_check.h"

// Initialize health check system
health_init();

// Perform health checks
health_check_all();

// Print health report
health_print_report();

// Write to file
health_write_to_file(".reprovm/health.log");
health_write_json(".reprovm/health.json");

// Get overall status
HealthStatus status = health_get_overall_status();
```

### Health Report

```
╔════════════════════════════════════════════════════════════════╗
║                     SYSTEM HEALTH REPORT                       ║
╠════════════════════════════════════════════════════════════════╣
║ Overall Status: OK                                             ║
║ Last Check: Fri Nov 10 14:30:00 2023                          ║
╠════════════════════════════════════════════════════════════════╣
║ Cache:     [OK      ] Cache operational with 42 entries       ║
║ CAS:       [OK      ] CAS operational with 128 objects        ║
║ Disk:      [OK      ] Disk usage normal: 45.2% used          ║
║ Memory:    [OK      ] Memory usage normal: 234.56 MB         ║
╚════════════════════════════════════════════════════════════════╝
```

## CI/CD Pipeline

### GitHub Actions Workflows

The project includes comprehensive CI/CD workflows:

1. **Linting**: Code quality checks with clang-format, clang-tidy, cppcheck
2. **Build**: Multi-platform builds (Ubuntu, macOS) with multiple compilers
3. **Test**: Automated test suite execution
4. **Code Coverage**: Coverage analysis with Codecov integration
5. **Security Scan**: CodeQL analysis for security vulnerabilities
6. **Docker**: Automated Docker image builds and publishing
7. **Performance**: Benchmark tests
8. **Release**: Automated release creation for tagged versions

### Running CI Locally

```bash
# Run linting
make clean
clang-format -i *.c *.h
clang-tidy *.c

# Run tests
make test

# Run with coverage
make coverage
gcovr --root . --html --html-details -o coverage.html

# Build Docker image
docker build -t reprovm:latest .
```

## Testing

### Test Suite

```bash
# Run all tests
make test

# Run specific test
cd tests
bash test_manifest.sh
bash test_parallel.sh
bash test_cas.sh

# Run memory leak detection
bash test_valgrind.sh

# Run performance benchmarks
bash benchmark.sh
```

### Writing Tests

Create test scripts in `tests/` directory:

```bash
#!/bin/bash
set -e

echo "=== Test: My Feature ==="

# Create test manifest
cat > test_my_feature.txt <<EOF
task test {
  cmd = echo "test"
  inputs =
  outputs = output.txt
  deps =
}
EOF

# Run test
../reprovm test_my_feature.txt

# Verify results
if [ -f output.txt ]; then
    echo "✓ Test passed"
else
    echo "✗ Test failed"
    exit 1
fi
```

## Deployment

### Installation

```bash
# Build and install
make all
sudo make install

# Verify installation
which reprovm
reprovm --help
```

### Docker Deployment

```bash
# Build image
docker build -t reprovm:latest .

# Run container
docker run --rm -v $(pwd):/workspace -w /workspace reprovm:latest ./reprovm manifest.txt

# Docker Compose
docker-compose up
```

### System Service

Create `/etc/systemd/system/reprovm.service`:

```ini
[Unit]
Description=ReproVM Service
After=network.target

[Service]
Type=simple
User=reprovm
WorkingDirectory=/opt/reprovm
ExecStart=/usr/local/bin/reprovm_parallel /opt/reprovm/manifest.txt
Restart=on-failure
Environment=REPROVM_LOG_LEVEL=INFO
Environment=REPROVM_JOBS=4

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl enable reprovm
sudo systemctl start reprovm
sudo systemctl status reprovm
```

### Monitoring Integration

#### Prometheus

Export metrics in Prometheus format:
```bash
# Add to your metrics collection
curl http://localhost:9090/metrics
```

#### Grafana

Import the provided dashboard template for visualization.

#### ELK Stack

Forward logs to Elasticsearch:
```bash
filebeat -c /etc/filebeat/filebeat.yml
```

## Best Practices

1. **Logging**: Use appropriate log levels; DEBUG for development, INFO for production
2. **Configuration**: Store sensitive data in environment variables, not config files
3. **Metrics**: Monitor cache hit rates; < 50% may indicate inefficient task granularity
4. **Security**: Always validate user inputs and sanitize paths
5. **Health Checks**: Run periodic health checks in production environments
6. **Backups**: Regularly backup the .reprovm/cache directory for disaster recovery
7. **Updates**: Keep ReproVM updated to receive security patches and improvements

## Troubleshooting

### High Memory Usage

```bash
# Check memory metrics
cat .reprovm/metrics.json | jq '.resources.peak_memory_bytes'

# Reduce parallel jobs
export REPROVM_JOBS=2
```

### Cache Issues

```bash
# Check cache health
./reprovm --health-check

# Clear cache
rm -rf .reprovm/cache/*

# Verify cache
./reprovm --verify-cache
```

### Performance Problems

```bash
# Run performance analysis
./tests/benchmark.sh

# Enable verbose logging
export REPROVM_LOG_LEVEL=DEBUG
export REPROVM_VERBOSE=1
```

## Support

For issues, questions, or contributions:

- GitHub Issues: https://github.com/hoangsonww/ReproVM-Virtual-Machine/issues
- Documentation: See README.md
- Security Issues: See SECURITY.md

## License

MIT License - See LICENSE file for details
