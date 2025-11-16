#include "progress.h"
#include "cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

static int get_terminal_width(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    return 80; // Default
}

ProgressBar* progress_create(size_t total, const char *prefix) {
    ProgressBar *bar = calloc(1, sizeof(ProgressBar));
    bar->total = total;
    bar->current = 0;
    bar->width = 40;
    bar->show_percentage = 1;
    bar->show_count = 1;
    bar->show_time = 1;
    bar->show_rate = 1;
    bar->prefix = prefix ? strdup(prefix) : NULL;
    bar->suffix = NULL;
    bar->start_time = time(NULL);
    bar->enabled = isatty(STDOUT_FILENO);
    return bar;
}

void progress_set_enabled(ProgressBar *bar, int enabled) {
    bar->enabled = enabled && isatty(STDOUT_FILENO);
}

void progress_set_prefix(ProgressBar *bar, const char *prefix) {
    free(bar->prefix);
    bar->prefix = prefix ? strdup(prefix) : NULL;
}

void progress_set_suffix(ProgressBar *bar, const char *suffix) {
    free(bar->suffix);
    bar->suffix = suffix ? strdup(suffix) : NULL;
}

void progress_update(ProgressBar *bar, size_t current) {
    if (!bar->enabled) return;

    bar->current = current;
    if (bar->current > bar->total) bar->current = bar->total;

    double progress = bar->total > 0 ? (double)bar->current / bar->total : 0.0;
    int filled = (int)(progress * bar->width);

    // Clear line
    printf("\r\033[K");

    // Prefix
    if (bar->prefix) {
        if (cli_colors_enabled()) {
            printf("%s%s%s ", CLI_COLOR_CYAN, bar->prefix, CLI_COLOR_RESET);
        } else {
            printf("%s ", bar->prefix);
        }
    }

    // Progress bar
    if (cli_colors_enabled()) {
        printf("[");
        for (int i = 0; i < bar->width; i++) {
            if (i < filled) {
                printf("%s█%s", CLI_COLOR_GREEN, CLI_COLOR_RESET);
            } else {
                printf("░");
            }
        }
        printf("]");
    } else {
        printf("[");
        for (int i = 0; i < bar->width; i++) {
            printf(i < filled ? "=" : " ");
        }
        printf("]");
    }

    // Percentage
    if (bar->show_percentage) {
        printf(" %.1f%%", progress * 100.0);
    }

    // Count
    if (bar->show_count) {
        printf(" (%zu/%zu)", bar->current, bar->total);
    }

    // Time and rate
    if (bar->show_time || bar->show_rate) {
        time_t elapsed = time(NULL) - bar->start_time;
        if (elapsed > 0 && bar->current > 0) {
            double rate = (double)bar->current / elapsed;

            if (bar->show_rate) {
                printf(" %.1f tasks/s", rate);
            }

            if (bar->show_time && bar->current < bar->total) {
                size_t remaining = bar->total - bar->current;
                time_t eta = (time_t)(remaining / rate);
                printf(" ETA: %lum%lus", eta / 60, eta % 60);
            }
        }
    }

    // Suffix
    if (bar->suffix) {
        printf(" %s", bar->suffix);
    }

    fflush(stdout);
}

void progress_increment(ProgressBar *bar) {
    progress_update(bar, bar->current + 1);
}

void progress_finish(ProgressBar *bar) {
    progress_update(bar, bar->total);
    if (bar->enabled) {
        printf("\n");
    }
}

void progress_free(ProgressBar *bar) {
    if (!bar) return;
    free(bar->prefix);
    free(bar->suffix);
    free(bar);
}

// Spinner implementation
static const char *spinner_frames[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
static const int spinner_frame_count = 10;

Spinner* spinner_create(const char *message) {
    Spinner *spinner = calloc(1, sizeof(Spinner));
    spinner->frame = 0;
    spinner->message = message ? strdup(message) : NULL;
    spinner->enabled = isatty(STDOUT_FILENO);
    spinner->start_time = time(NULL);
    return spinner;
}

void spinner_update(Spinner *spinner) {
    if (!spinner->enabled) return;

    printf("\r\033[K");

    if (cli_colors_enabled()) {
        printf("%s%s%s ", CLI_COLOR_CYAN,
               spinner_frames[spinner->frame], CLI_COLOR_RESET);
    } else {
        const char *simple_frames[] = {"|", "/", "-", "\\"};
        printf("%s ", simple_frames[spinner->frame % 4]);
    }

    if (spinner->message) {
        printf("%s", spinner->message);
    }

    time_t elapsed = time(NULL) - spinner->start_time;
    printf(" (%lus)", elapsed);

    fflush(stdout);

    spinner->frame = (spinner->frame + 1) % spinner_frame_count;
}

void spinner_finish(Spinner *spinner, const char *final_message) {
    if (!spinner->enabled) {
        if (final_message) printf("%s\n", final_message);
        return;
    }

    printf("\r\033[K");

    if (cli_colors_enabled()) {
        printf("%s✓%s ", CLI_COLOR_GREEN, CLI_COLOR_RESET);
    } else {
        printf("[DONE] ");
    }

    if (final_message) {
        printf("%s", final_message);
    } else if (spinner->message) {
        printf("%s", spinner->message);
    }

    printf("\n");
}

void spinner_free(Spinner *spinner) {
    if (!spinner) return;
    free(spinner->message);
    free(spinner);
}
