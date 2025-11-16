#include "cli.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

static int colors_enabled = 1;

void cli_enable_colors(int enable) {
    colors_enabled = enable && isatty(STDOUT_FILENO);
}

int cli_colors_enabled(void) {
    return colors_enabled;
}

void cli_print_colored(const char *color, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (colors_enabled) {
        printf("%s", color);
    }
    vprintf(fmt, args);
    if (colors_enabled) {
        printf("%s", CLI_COLOR_RESET);
    }

    va_end(args);
}

void cli_print_banner(void) {
    if (!colors_enabled) {
        printf("===========================================\n");
        printf("    ReproVM - Reproducible Task VM\n");
        printf("    Production-Ready Edition v2.0\n");
        printf("===========================================\n\n");
        return;
    }

    printf("\n");
    cli_print_colored(CLI_COLOR_CYAN CLI_COLOR_BOLD,
        "╔═══════════════════════════════════════════════════════════╗\n");
    cli_print_colored(CLI_COLOR_CYAN CLI_COLOR_BOLD,
        "║                                                           ║\n");
    cli_print_colored(CLI_COLOR_CYAN CLI_COLOR_BOLD, "║  ");
    cli_print_colored(CLI_COLOR_GREEN CLI_COLOR_BOLD, "ReproVM");
    cli_print_colored(CLI_COLOR_WHITE, " - Reproducible Task Virtual Machine      ");
    cli_print_colored(CLI_COLOR_CYAN CLI_COLOR_BOLD, "║\n");
    cli_print_colored(CLI_COLOR_CYAN CLI_COLOR_BOLD, "║  ");
    cli_print_colored(CLI_COLOR_YELLOW, "Production-Ready Edition v2.0");
    cli_print_colored(CLI_COLOR_CYAN, "                     ");
    cli_print_colored(CLI_COLOR_CYAN CLI_COLOR_BOLD, "║\n");
    cli_print_colored(CLI_COLOR_CYAN CLI_COLOR_BOLD,
        "║                                                           ║\n");
    cli_print_colored(CLI_COLOR_CYAN CLI_COLOR_BOLD,
        "╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void cli_print_version(void) {
    cli_print_colored(CLI_COLOR_BOLD, "ReproVM version 2.0.0-production\n");
    printf("Built with production-ready features:\n");
    printf("  - Comprehensive logging\n");
    printf("  - Metrics & monitoring\n");
    printf("  - Health checks\n");
    printf("  - Security hardening\n");
    printf("  - CI/CD integration\n");
    printf("  - Enhanced CLI\n");
    printf("\n");
    printf("License: MIT\n");
    printf("Repository: https://github.com/hoangsonww/ReproVM-Virtual-Machine\n");
}

void cli_print_help(const char *program_name) {
    cli_print_colored(CLI_COLOR_BOLD, "Usage: ");
    printf("%s [OPTIONS] <manifest> [targets...]\n\n", program_name);

    cli_print_colored(CLI_COLOR_BOLD, "Arguments:\n");
    printf("  <manifest>              Path to the task manifest file\n");
    printf("  [targets...]            Specific tasks to execute (default: all)\n\n");

    cli_print_colored(CLI_COLOR_BOLD, "General Options:\n");
    printf("  -h, --help              Show this help message\n");
    printf("  -v, --version           Show version information\n");
    printf("  -V, --verbose           Enable verbose output\n");
    printf("  -q, --quiet             Suppress non-error output\n");
    printf("  -c, --config FILE       Use custom configuration file\n");
    printf("      --no-color          Disable colored output\n\n");

    cli_print_colored(CLI_COLOR_BOLD, "Execution Options:\n");
    printf("  -j, --jobs N            Number of parallel jobs (default: CPU count)\n");
    printf("  -f, --force             Force rebuild, ignore cache\n");
    printf("  -n, --dry-run           Show what would be executed without running\n");
    printf("  -t, --timeout SEC       Set task timeout in seconds\n");
    printf("      --no-progress       Disable progress bar\n\n");

    cli_print_colored(CLI_COLOR_BOLD, "Cache Options:\n");
    printf("      --cache-dir DIR     Override cache directory\n");
    printf("      --clean-cache       Clean the cache before running\n");
    printf("      --verify-cache      Verify cache integrity\n");
    printf("      --compress          Enable compression for CAS\n\n");

    cli_print_colored(CLI_COLOR_BOLD, "Output Options:\n");
    printf("  -l, --log-file FILE     Write logs to file\n");
    printf("  -m, --metrics FILE      Write metrics to file\n");
    printf("  -o, --output FORMAT     Output format (text, json)\n");
    printf("      --stats             Show statistics after execution\n\n");

    cli_print_colored(CLI_COLOR_BOLD, "Diagnostic Options:\n");
    printf("      --health-check      Run system health check\n");
    printf("      --list-tasks        List all tasks in manifest\n");
    printf("      --profile           Enable performance profiling\n\n");

    cli_print_colored(CLI_COLOR_BOLD, "Examples:\n");
    printf("  # Run all tasks in manifest\n");
    printf("  %s manifest.txt\n\n", program_name);
    printf("  # Run specific tasks with 8 parallel jobs\n");
    printf("  %s -j 8 manifest.txt build test\n\n", program_name);
    printf("  # Force rebuild with verbose output\n");
    printf("  %s -f -V manifest.txt\n\n", program_name);
    printf("  # Health check only\n");
    printf("  %s --health-check\n\n", program_name);
}

int cli_parse_args(int argc, char **argv, CLIOptions *opts) {
    // Initialize options with defaults
    memset(opts, 0, sizeof(CLIOptions));
    opts->verbose = 0;
    opts->quiet = 0;
    opts->force_rebuild = 0;
    opts->dry_run = 0;
    opts->enable_colors = 1;
    opts->enable_progress = 1;
    opts->parallel_jobs = -1; // Auto-detect
    opts->timeout = 3600;
    opts->output_format = strdup("text");

    static struct option long_options[] = {
        {"help",         no_argument,       0, 'h'},
        {"version",      no_argument,       0, 'v'},
        {"verbose",      no_argument,       0, 'V'},
        {"quiet",        no_argument,       0, 'q'},
        {"config",       required_argument, 0, 'c'},
        {"jobs",         required_argument, 0, 'j'},
        {"force",        no_argument,       0, 'f'},
        {"dry-run",      no_argument,       0, 'n'},
        {"timeout",      required_argument, 0, 't'},
        {"log-file",     required_argument, 0, 'l'},
        {"metrics",      required_argument, 0, 'm'},
        {"output",       required_argument, 0, 'o'},
        {"no-color",     no_argument,       0,  1 },
        {"no-progress",  no_argument,       0,  2 },
        {"cache-dir",    required_argument, 0,  3 },
        {"clean-cache",  no_argument,       0,  4 },
        {"verify-cache", no_argument,       0,  5 },
        {"compress",     no_argument,       0,  6 },
        {"stats",        no_argument,       0,  7 },
        {"health-check", no_argument,       0,  8 },
        {"list-tasks",   no_argument,       0,  9 },
        {"profile",      no_argument,       0, 10 },
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "hvVqc:j:fnt:l:m:o:",
                              long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                opts->show_help = 1;
                return 0;
            case 'v':
                opts->show_version = 1;
                return 0;
            case 'V':
                opts->verbose = 1;
                break;
            case 'q':
                opts->quiet = 1;
                break;
            case 'c':
                opts->config_file = strdup(optarg);
                break;
            case 'j':
                opts->parallel_jobs = atoi(optarg);
                break;
            case 'f':
                opts->force_rebuild = 1;
                break;
            case 'n':
                opts->dry_run = 1;
                break;
            case 't':
                opts->timeout = atoi(optarg);
                break;
            case 'l':
                opts->log_file = strdup(optarg);
                break;
            case 'm':
                opts->metrics_file = strdup(optarg);
                break;
            case 'o':
                free(opts->output_format);
                opts->output_format = strdup(optarg);
                break;
            case 1: // --no-color
                opts->enable_colors = 0;
                break;
            case 2: // --no-progress
                opts->enable_progress = 0;
                break;
            case 3: // --cache-dir
                opts->cache_dir = strdup(optarg);
                break;
            case 4: // --clean-cache
                opts->clean_cache = 1;
                break;
            case 5: // --verify-cache
                opts->verify_cache = 1;
                break;
            case 6: // --compress
                opts->enable_compression = 1;
                break;
            case 7: // --stats
                opts->show_stats = 1;
                break;
            case 8: // --health-check
                opts->health_check = 1;
                return 0;
            case 9: // --list-tasks
                opts->list_tasks = 1;
                break;
            case 10: // --profile
                opts->enable_profiling = 1;
                break;
            default:
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                return -1;
        }
    }

    // Get manifest file and targets
    if (optind < argc && !opts->health_check) {
        opts->manifest_file = strdup(argv[optind++]);

        // Collect targets
        opts->target_count = argc - optind;
        if (opts->target_count > 0) {
            opts->targets = malloc(sizeof(char*) * opts->target_count);
            for (int i = 0; i < opts->target_count; i++) {
                opts->targets[i] = strdup(argv[optind++]);
            }
        }
    }

    return 0;
}

int cli_validate_options(CLIOptions *opts) {
    if (!opts->manifest_file && !opts->health_check) {
        fprintf(stderr, "Error: Manifest file required\n");
        return -1;
    }

    if (opts->parallel_jobs < -1 || opts->parallel_jobs > 256) {
        fprintf(stderr, "Error: Invalid job count (must be 1-256 or -1 for auto)\n");
        return -1;
    }

    if (opts->timeout < 0) {
        fprintf(stderr, "Error: Invalid timeout (must be >= 0)\n");
        return -1;
    }

    if (opts->verbose && opts->quiet) {
        fprintf(stderr, "Warning: Both --verbose and --quiet specified, using verbose\n");
        opts->quiet = 0;
    }

    return 0;
}

void cli_free_options(CLIOptions *opts) {
    free(opts->manifest_file);
    free(opts->config_file);
    free(opts->log_file);
    free(opts->metrics_file);
    free(opts->output_format);
    free(opts->cache_dir);

    if (opts->targets) {
        for (int i = 0; i < opts->target_count; i++) {
            free(opts->targets[i]);
        }
        free(opts->targets);
    }
}
