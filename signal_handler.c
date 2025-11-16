#include "signal_handler.h"
#include "logger.h"
#include "metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <execinfo.h>

static volatile sig_atomic_t shutdown_requested = 0;
static SignalCallback custom_callbacks[NSIG] = {0};

int is_shutdown_requested(void) {
    return shutdown_requested;
}

void request_shutdown(void) {
    shutdown_requested = 1;
}

void print_backtrace(void) {
    void *array[20];
    size_t size;
    char **strings;

    size = backtrace(array, 20);
    strings = backtrace_symbols(array, size);

    LOG_ERROR("Stack trace:");
    for (size_t i = 0; i < size; i++) {
        LOG_ERROR("  [%zu] %s", i, strings[i]);
    }

    free(strings);
}

void handle_sigint(int signum) {
    (void)signum;
    LOG_WARN("Received SIGINT (Ctrl+C), initiating graceful shutdown...");
    request_shutdown();
}

void handle_sigterm(int signum) {
    (void)signum;
    LOG_WARN("Received SIGTERM, initiating graceful shutdown...");
    request_shutdown();
}

void handle_sigsegv(int signum) {
    (void)signum;
    LOG_FATAL("Segmentation fault detected!");
    print_backtrace();

    // Write metrics before crash
    metrics_write_to_file(".reprovm/crash_metrics.log");

    // Re-raise the signal to get core dump
    signal(SIGSEGV, SIG_DFL);
    raise(SIGSEGV);
}

void handle_sigabrt(int signum) {
    (void)signum;
    LOG_FATAL("Abnormal termination (SIGABRT)!");
    print_backtrace();

    // Write metrics before crash
    metrics_write_to_file(".reprovm/crash_metrics.log");

    signal(SIGABRT, SIG_DFL);
    raise(SIGABRT);
}

void graceful_shutdown(int signum) {
    LOG_INFO("Performing graceful shutdown (signal %d)...", signum);

    // Calculate and print metrics
    metrics_calculate_stats();
    metrics_print_summary();
    metrics_write_to_file(".reprovm/metrics.log");
    metrics_write_json(".reprovm/metrics.json");

    // Close logger
    logger_close();

    exit(signum == SIGINT || signum == SIGTERM ? 0 : 1);
}

void register_signal_callback(int signum, SignalCallback callback) {
    if (signum >= 0 && signum < NSIG) {
        custom_callbacks[signum] = callback;
    }
}

void universal_signal_handler(int signum) {
    // Call custom callback if registered
    if (custom_callbacks[signum]) {
        custom_callbacks[signum](signum);
    }

    // Call default handlers
    switch (signum) {
        case SIGINT:
            handle_sigint(signum);
            graceful_shutdown(signum);
            break;
        case SIGTERM:
            handle_sigterm(signum);
            graceful_shutdown(signum);
            break;
        case SIGSEGV:
            handle_sigsegv(signum);
            break;
        case SIGABRT:
            handle_sigabrt(signum);
            break;
        default:
            LOG_WARN("Received signal %d", signum);
            break;
    }
}

void setup_signal_handlers(void) {
    struct sigaction sa;
    sa.sa_handler = universal_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    // Handle graceful shutdown signals
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM");
    }

    // Handle crash signals
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction SIGSEGV");
    }
    if (sigaction(SIGABRT, &sa, NULL) == -1) {
        perror("sigaction SIGABRT");
    }

    // Ignore SIGPIPE (broken pipe)
    signal(SIGPIPE, SIG_IGN);

    LOG_DEBUG("Signal handlers configured");
}
