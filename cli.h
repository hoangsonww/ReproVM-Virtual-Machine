#ifndef CLI_H
#define CLI_H

#include <getopt.h>

// CLI options structure
typedef struct {
    char *manifest_file;
    char **targets;
    int target_count;

    // Flags
    int verbose;
    int quiet;
    int force_rebuild;
    int dry_run;
    int show_version;
    int show_help;
    int enable_colors;
    int enable_progress;

    // Performance
    int parallel_jobs;
    int timeout;

    // Output
    char *log_file;
    char *metrics_file;
    char *output_format; // json, text, etc.

    // Features
    int health_check;
    int verify_cache;
    int clean_cache;
    int show_stats;
    int list_tasks;

    // Advanced
    char *config_file;
    char *cache_dir;
    int enable_profiling;
    int enable_compression;
} CLIOptions;

// Parse command line arguments
int cli_parse_args(int argc, char **argv, CLIOptions *opts);

// Print help message
void cli_print_help(const char *program_name);

// Print version information
void cli_print_version(void);

// Print banner
void cli_print_banner(void);

// Free CLI options
void cli_free_options(CLIOptions *opts);

// Validate CLI options
int cli_validate_options(CLIOptions *opts);

// Color support
void cli_enable_colors(int enable);
int cli_colors_enabled(void);

// Print colored text
void cli_print_colored(const char *color, const char *fmt, ...);

// ANSI colors
#define CLI_COLOR_RESET   "\033[0m"
#define CLI_COLOR_RED     "\033[31m"
#define CLI_COLOR_GREEN   "\033[32m"
#define CLI_COLOR_YELLOW  "\033[33m"
#define CLI_COLOR_BLUE    "\033[34m"
#define CLI_COLOR_MAGENTA "\033[35m"
#define CLI_COLOR_CYAN    "\033[36m"
#define CLI_COLOR_WHITE   "\033[37m"
#define CLI_COLOR_BOLD    "\033[1m"

#endif // CLI_H
