#define _POSIX_C_SOURCE 200809L
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

char *read_entire_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    rewind(f);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, sz, f) != (size_t)sz) {
        free(buf);
        fclose(f);
        return NULL;
    }
    buf[sz] = '\0';
    fclose(f);
    if (out_size) *out_size = sz;
    return buf;
}

char *strdup_safe(const char *s) {
    if (!s) return NULL;
    size_t l = strlen(s);
    char *d = malloc(l + 1);
    if (!d) return NULL;
    memcpy(d, s, l + 1);
    return d;
}

void trim(char *s) {
    if (!s) return;
    // leading
    char *p = s;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    // trailing
    size_t len = strlen(s);
    while (len && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r' || s[len - 1] == '\n')) {
        s[len - 1] = '\0';
        len--;
    }
}

char **split_csv_array(const char *line, int *out_n) {
    if (!line) { *out_n = 0; return NULL; }
    char *copy = strdup_safe(line);
    if (!copy) { *out_n = 0; return NULL; }
    int capacity = 8;
    char **arr = malloc(sizeof(char*) * capacity);
    int count = 0;
    char *p = copy;
    while (*p) {
        // find comma or end
        char *comma = strchr(p, ',');
        size_t len = comma ? (size_t)(comma - p) : strlen(p);
        char *token = malloc(len + 1);
        memcpy(token, p, len);
        token[len] = '\0';
        trim(token);
        if (strlen(token) > 0) {
            if (count >= capacity) {
                capacity *= 2;
                arr = realloc(arr, sizeof(char*) * capacity);
            }
            arr[count++] = token;
        } else {
            free(token);
        }
        if (!comma) break;
        p = comma + 1;
    }
    free(copy);
    *out_n = count;
    return arr;
}

void free_string_array(char **arr, int n) {
    if (!arr) return;
    for (int i = 0; i < n; ++i) free(arr[i]);
    free(arr);
}

int ensure_dir_recursive(const char *path) {
    if (!path) return -1;
    char tmp[4096];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp)-1] = '\0';
    size_t len = strlen(tmp);
    if (len == 0) return -1;
    if (tmp[len - 1] == '/') tmp[len - 1] = '\0';
    for (char *p = tmp + 1; *p; ++p) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0) {
                if (errno != EEXIST) return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0755) != 0) {
        if (errno != EEXIST) return -1;
    }
    return 0;
}

int file_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0);
}

int copy_file(const char *src, const char *dst) {
    FILE *fsrc = fopen(src, "rb");
    if (!fsrc) return -1;
    FILE *fdst = fopen(dst, "wb");
    if (!fdst) { fclose(fsrc); return -1; }
    char buf[8192];
    size_t r;
    while ((r = fread(buf, 1, sizeof(buf), fsrc)) > 0) {
        if (fwrite(buf, 1, r, fdst) != r) {
            fclose(fsrc); fclose(fdst);
            return -1;
        }
    }
    fclose(fsrc);
    fclose(fdst);
    return 0;
}

char *join_strings(const char **parts, int n, const char *sep) {
    if (n == 0) return strdup_safe("");
    size_t total = 0;
    size_t seplen = strlen(sep);
    for (int i = 0; i < n; ++i) total += strlen(parts[i]);
    total += seplen * (n - 1) + 1;
    char *res = malloc(total);
    if (!res) return NULL;
    res[0] = '\0';
    for (int i = 0; i < n; ++i) {
        strcat(res, parts[i]);
        if (i + 1 < n) strcat(res, sep);
    }
    return res;
}

char *hex_encode(const unsigned char *data, size_t len) {
    static const char hex[] = "0123456789abcdef";
    char *out = malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[2*i]   = hex[(data[i] >> 4) & 0xF];
        out[2*i+1] = hex[data[i] & 0xF];
    }
    out[len*2] = '\0';
    return out;
}

void hexdump(const unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
        if ((i+1)%16==0) printf("\n");
        else if ((i+1)%2==0) printf(" ");
    }
    if (len%16) printf("\n");
}
