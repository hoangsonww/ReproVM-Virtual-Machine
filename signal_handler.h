#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>

// Signal handler function type
typedef void (*SignalCallback)(int signum);

// Setup signal handlers
void setup_signal_handlers(void);

// Cleanup and shutdown
void graceful_shutdown(int signum);

// Check if shutdown was requested
int is_shutdown_requested(void);

// Set shutdown flag
void request_shutdown(void);

// Register custom signal callback
void register_signal_callback(int signum, SignalCallback callback);

// Default handlers
void handle_sigint(int signum);
void handle_sigterm(int signum);
void handle_sigsegv(int signum);
void handle_sigabrt(int signum);

#endif // SIGNAL_HANDLER_H
