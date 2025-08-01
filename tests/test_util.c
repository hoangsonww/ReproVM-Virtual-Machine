#include "../util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    char out[65];

    // test sha256_string on "abc"
    sha256_string((unsigned char*)"abc", 3, out);
    if (strcmp(out, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") != 0) {
        fprintf(stderr, "sha256_string failed for 'abc': got %s\n", out);
        return 1;
    }

    // write temp file with known content "hello\n"
    const char *fn = "tests_tmp_hello.txt";
    FILE *f = fopen(fn, "w");
    if (!f) { perror("fopen"); return 1; }
    fputs("hello\n", f);
    fclose(f);

    if (sha256_file(fn, out) != 0) {
        fprintf(stderr, "sha256_file failed\n");
        return 1;
    }
    if (strcmp(out, "5891b5b522d5df086d0ff0b110fbd9d21bb4fc7163af34d08286a2e846f6be03") != 0) {
        fprintf(stderr, "sha256_file mismatch: expected 5891b5...be03 got %s\n", out);
        return 1;
    }

    remove(fn);
    puts("OK");
    return 0;
}
