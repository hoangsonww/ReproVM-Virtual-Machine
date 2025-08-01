#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdbool.h>

char *read_entire_file(const char *path, size_t *out_size);
char *strdup_safe(const char *s);
void trim(char *s);
char **split_csv_array(const char *line, int *out_n); // comma-separated, returns malloc'd array; caller frees
void free_string_array(char **arr, int n);
int ensure_dir_recursive(const char *path);
int file_exists(const char *path);
int copy_file(const char *src, const char *dst);
char *join_strings(const char **parts, int n, const char *sep); // new string
char *hex_encode(const unsigned char *data, size_t len); // returns malloc'd hex string lowercase
void hexdump(const unsigned char *data, size_t len);
#endif
