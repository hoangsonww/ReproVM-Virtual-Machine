// cas.c
#define _POSIX_C_SOURCE 200809L
#include "cas.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

/*
 * Minimal SHA-256 implementation (public domain / simplified)
 * Based on the pseudocode from FIPS PUB 180-4.
 */
typedef struct {
    uint64_t bitlen;
    uint32_t state[8];
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

static const uint32_t k[64] = {
    0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
    0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
    0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
    0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
    0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
    0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
    0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
    0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i = 0; i < 16; ++i)
        m[i] = (data[i*4] << 24) | (data[i*4+1] << 16) | (data[i*4+2] << 8) | (data[i*4+3]);
    for (int i = 16; i < 64; ++i) {
        uint32_t s0 = rotr(m[i-15], 7) ^ rotr(m[i-15], 18) ^ (m[i-15] >> 3);
        uint32_t s1 = rotr(m[i-2], 17) ^ rotr(m[i-2], 19) ^ (m[i-2] >> 10);
        m[i] = m[i-16] + s0 + m[i-7] + s1;
    }
    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];
    uint32_t e = ctx->state[4];
    uint32_t f = ctx->state[5];
    uint32_t g = ctx->state[6];
    uint32_t h = ctx->state[7];
    for (int i = 0; i < 64; ++i) {
        uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t temp1 = h + S1 + ch + k[i] + m[i];
        uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667ul;
    ctx->state[1] = 0xbb67ae85ul;
    ctx->state[2] = 0x3c6ef372ul;
    ctx->state[3] = 0xa54ff53aul;
    ctx->state[4] = 0x510e527ful;
    ctx->state[5] = 0x9b05688cul;
    ctx->state[6] = 0x1f83d9abul;
    ctx->state[7] = 0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[]) {
    size_t i = ctx->datalen;

    // Pad whatever data is left in the buffer.
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);

    // Since this implementation uses little endian byte ordering and SHA uses big endian,
    // reverse all the bytes when copying the final state.
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 8; ++i) {
            hash[j + i*4] = (ctx->state[i] >> (24 - j * 8)) & 0x000000ff;
        }
    }
}

/* global root */
static char objects_root[1024] = {0};
static char cache_root[1024] = {0};

const char *cas_get_objects_root() { return objects_root; }
const char *cas_get_cache_root() { return cache_root; }

int cas_init(const char *base_dir) {
    if (!base_dir) return -1;
    snprintf(objects_root, sizeof(objects_root), "%s/.reprovm/cas/objects", base_dir);
    snprintf(cache_root, sizeof(cache_root), "%s/.reprovm/cache", base_dir);
    if (ensure_dir_recursive(objects_root) != 0) return -1;
    if (ensure_dir_recursive(cache_root) != 0) return -1;
    return 0;
}

static int make_object_path(const char *hash, char *out, size_t sz) {
    if (strlen(hash) < 3) return -1;
    // use two-level: first two chars as dir
    char dir[1024];
    snprintf(dir, sizeof(dir), "%s/%c%c", objects_root, hash[0], hash[1]);
    if (ensure_dir_recursive(dir) != 0) return -1;
    // object file name is remainder
    snprintf(out, sz, "%s/%s", dir, hash + 2);
    return 0;
}

int cas_get_object_path(const char *hash, char *out_path, size_t sz) {
    return make_object_path(hash, out_path, sz);
}

int cas_blob_exists(const char *hash) {
    char path[2048];
    if (make_object_path(hash, path, sizeof(path)) != 0) return 0;
    return file_exists(path);
}

char *cas_store_blob_from_memory(const unsigned char *data, size_t len) {
    SHA256_CTX ctx;
    uint8_t hash_raw[32];
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, hash_raw);
    char *hex = hex_encode(hash_raw, 32);
    if (!hex) return NULL;
    char obj_path[2048];
    if (make_object_path(hex, obj_path, sizeof(obj_path)) != 0) {
        free(hex);
        return NULL;
    }
    if (!file_exists(obj_path)) {
        // write it (atomic)
        char tmp[4096];
        snprintf(tmp, sizeof(tmp), "%s.tmp", obj_path);
        FILE *f = fopen(tmp, "wb");
        if (!f) {
            free(hex);
            return NULL;
        }
        if (fwrite(data, 1, len, f) != len) {
            fclose(f);
            unlink(tmp);
            free(hex);
            return NULL;
        }
        fclose(f);
        if (rename(tmp, obj_path) != 0) {
            unlink(tmp);
            free(hex);
            return NULL;
        }
    }
    return hex;
}

char *cas_store_blob_from_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    SHA256_CTX ctx;
    uint8_t hash_raw[32];
    sha256_init(&ctx);
    char buf[8192];
    size_t r;
    while ((r = fread(buf, 1, sizeof(buf), f)) > 0) {
        sha256_update(&ctx, (unsigned char*)buf, r);
    }
    fclose(f);
    sha256_final(&ctx, hash_raw);
    char *hex = hex_encode(hash_raw, 32);
    if (!hex) return NULL;
    char obj_path[2048];
    if (make_object_path(hex, obj_path, sizeof(obj_path)) != 0) {
        free(hex);
        return NULL;
    }
    if (!file_exists(obj_path)) {
        // copy over
        if (copy_file(path, obj_path) != 0) {
            free(hex);
            return NULL;
        }
    }
    return hex;
}

int cas_restore_blob_to_file(const char *hash, const char *dest) {
    char obj_path[2048];
    if (make_object_path(hash, obj_path, sizeof(obj_path)) != 0) return -1;
    if (!file_exists(obj_path)) return -1;
    return copy_file(obj_path, dest);
}
