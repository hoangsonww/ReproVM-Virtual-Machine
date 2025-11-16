#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <stddef.h>

// Compression algorithms
typedef enum {
    COMPRESS_NONE = 0,
    COMPRESS_ZLIB = 1,
    COMPRESS_GZIP = 2,
    COMPRESS_LZ4 = 3,
    COMPRESS_ZSTD = 4
} CompressionAlgorithm;

// Compression configuration
typedef struct {
    CompressionAlgorithm algorithm;
    int level; // 1-9 for zlib/gzip, 1-12 for zstd
    int enabled;
} CompressionConfig;

// Global compression config
extern CompressionConfig g_compression_config;

// Initialize compression
void compression_init(CompressionAlgorithm algorithm, int level);

// Compress data
int compression_compress(const unsigned char *input, size_t input_len,
                        unsigned char **output, size_t *output_len);

// Decompress data
int compression_decompress(const unsigned char *input, size_t input_len,
                          unsigned char **output, size_t *output_len);

// Compress file
int compression_compress_file(const char *input_path, const char *output_path);

// Decompress file
int compression_decompress_file(const char *input_path, const char *output_path);

// Get algorithm name
const char* compression_algorithm_name(CompressionAlgorithm algorithm);

// Estimate compression ratio
double compression_estimate_ratio(const unsigned char *data, size_t len);

#endif // COMPRESSION_H
