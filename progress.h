#ifndef PROGRESS_H
#define PROGRESS_H

#include <stddef.h>
#include <time.h>

// Progress bar configuration
typedef struct {
    size_t total;
    size_t current;
    size_t width;
    int show_percentage;
    int show_count;
    int show_time;
    int show_rate;
    char *prefix;
    char *suffix;
    time_t start_time;
    int enabled;
} ProgressBar;

// Initialize progress bar
ProgressBar* progress_create(size_t total, const char *prefix);

// Update progress
void progress_update(ProgressBar *bar, size_t current);

// Increment progress
void progress_increment(ProgressBar *bar);

// Finish progress (100%)
void progress_finish(ProgressBar *bar);

// Free progress bar
void progress_free(ProgressBar *bar);

// Set prefix/suffix
void progress_set_prefix(ProgressBar *bar, const char *prefix);
void progress_set_suffix(ProgressBar *bar, const char *suffix);

// Enable/disable
void progress_set_enabled(ProgressBar *bar, int enabled);

// Spinner for indeterminate progress
typedef struct {
    int frame;
    char *message;
    int enabled;
    time_t start_time;
} Spinner;

// Spinner functions
Spinner* spinner_create(const char *message);
void spinner_update(Spinner *spinner);
void spinner_finish(Spinner *spinner, const char *final_message);
void spinner_free(Spinner *spinner);

#endif // PROGRESS_H
