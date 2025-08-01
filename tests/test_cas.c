#include "../cas.h"
#include "../util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    if (cas_init(".") != 0) { fprintf(stderr, "cas_init failed\n"); return 1; }

    const char *infile = "tests_cas_input.txt";
    FILE *f = fopen(infile, "w");
    if (!f) { perror("fopen"); return 1; }
    fputs("foo\n", f);
    fclose(f);

    char hash1[65] = {0};
    if (cas_store(infile, hash1) != 0) { fprintf(stderr, "cas_store failed\n"); return 1; }

    // remove original and fetch back
    remove(infile);
    const char *out = "tests_cas_output.txt";
    if (cas_fetch(hash1, out) != 0) { fprintf(stderr, "cas_fetch failed\n"); return 1; }

    char hash2[65] = {0};
    if (cas_hash_of_file(out, hash2) != 0) { fprintf(stderr, "hash_of_file failed\n"); return 1; }

    if (strcmp(hash1, hash2) != 0) {
        fprintf(stderr, "hash mismatch: %s vs %s\n", hash1, hash2);
        return 1;
    }

    // verify CAS blob exists
    char caspath[1024];
    char prefix[3] = { hash1[0], hash1[1], 0 };
    snprintf(caspath, sizeof(caspath), ".reprovm/cas/objects/%s/%s", prefix, hash1 + 2);
    FILE *g = fopen(caspath, "rb");
    if (!g) { fprintf(stderr, "CAS object missing at %s\n", caspath); return 1; }
    fclose(g);

    // cleanup
    remove(out);
    puts("OK");
    return 0;
}
