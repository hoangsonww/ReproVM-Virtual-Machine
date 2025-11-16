#include "compression.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Note: This is a stub implementation. Full implementation would use zlib, lz4, etc.

CompressionConfig g_compression_config = {
    .algorithm = COMPRESS_NONE,
    .level = 6,
    .enabled = 0
};

void compression_init(CompressionAlgorithm algorithm, int level) {
    g_compression_config.algorithm = algorithm;
    g_compression_config.level = level;
    g_compression_config.enabled = (algorithm != COMPRESS_NONE);

    LOG_INFO("Compression initialized: algorithm=%s, level=%d",
             compression_algorithm_name(algorithm), level);
}

const char* compression_algorithm_name(CompressionAlgorithm algorithm) {
    switch (algorithm) {
        case COMPRESS_NONE: return "none";
        case COMPRESS_ZLIB: return "zlib";
        case COMPRESS_GZIP: return "gzip";
        case COMPRESS_LZ4:  return "lz4";
        case COMPRESS_ZSTD: return "zstd";
        default:            return "unknown";
    }
}

// Stub implementation - would use actual compression libraries
int compression_compress(const unsigned char *input, size_t input_len,
                        unsigned char **output, size_t *output_len) {
    if (!g_compression_config.enabled || g_compression_config.algorithm == COMPRESS_NONE) {
        // No compression - just copy
        *output = malloc(input_len);
        if (!*output) return -1;
        memcpy(*output, input, input_len);
        *output_len = input_len;
        return 0;
    }

    // Stub: In production, use actual compression libraries
    // For zlib: use compress2()
    // For lz4: use LZ4_compress_default()
    // For zstd: use ZSTD_compress()

    LOG_DEBUG("Compression stub called (algorithm=%s, input_len=%zu)",
              compression_algorithm_name(g_compression_config.algorithm), input_len);

    // For now, just copy the data (no actual compression)
    *output = malloc(input_len);
    if (!*output) return -1;
    memcpy(*output, input, input_len);
    *output_len = input_len;

    return 0;
}

int compression_decompress(const unsigned char *input, size_t input_len,
                          unsigned char **output, size_t *output_len) {
    if (!g_compression_config.enabled || g_compression_config.algorithm == COMPRESS_NONE) {
        *output = malloc(input_len);
        if (!*output) return -1;
        memcpy(*output, input, input_len);
        *output_len = input_len;
        return 0;
    }

    // Stub: In production, use actual decompression libraries
    LOG_DEBUG("Decompression stub called (algorithm=%s, input_len=%zu)",
              compression_algorithm_name(g_compression_config.algorithm), input_len);

    *output = malloc(input_len);
    if (!*output) return -1;
    memcpy(*output, input, input_len);
    *output_len = input_len;

    return 0;
}

int compression_compress_file(const char *input_path, const char *output_path) {
    FILE *in = fopen(input_path, "rb");
    if (!in) {
        LOG_ERROR("Failed to open input file: %s", input_path);
        return -1;
    }

    // Get file size
    fseek(in, 0, SEEK_END);
    size_t file_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    // Read file
    unsigned char *input_data = malloc(file_size);
    if (!input_data) {
        fclose(in);
        return -1;
    }
    fread(input_data, 1, file_size, in);
    fclose(in);

    // Compress
    unsigned char *output_data;
    size_t output_len;
    if (compression_compress(input_data, file_size, &output_data, &output_len) != 0) {
        free(input_data);
        return -1;
    }
    free(input_data);

    // Write compressed data
    FILE *out = fopen(output_path, "wb");
    if (!out) {
        LOG_ERROR("Failed to open output file: %s", output_path);
        free(output_data);
        return -1;
    }
    fwrite(output_data, 1, output_len, out);
    fclose(out);
    free(output_data);

    LOG_DEBUG("File compressed: %s -> %s (%.2f%% ratio)",
              input_path, output_path,
              file_size > 0 ? (100.0 * output_len / file_size) : 100.0);

    return 0;
}

int compression_decompress_file(const char *input_path, const char *output_path) {
    FILE *in = fopen(input_path, "rb");
    if (!in) {
        LOG_ERROR("Failed to open input file: %s", input_path);
        return -1;
    }

    fseek(in, 0, SEEK_END);
    size_t file_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    unsigned char *input_data = malloc(file_size);
    if (!input_data) {
        fclose(in);
        return -1;
    }
    fread(input_data, 1, file_size, in);
    fclose(in);

    unsigned char *output_data;
    size_t output_len;
    if (compression_decompress(input_data, file_size, &output_data, &output_len) != 0) {
        free(input_data);
        return -1;
    }
    free(input_data);

    FILE *out = fopen(output_path, "wb");
    if (!out) {
        LOG_ERROR("Failed to open output file: %s", output_path);
        free(output_data);
        return -1;
    }
    fwrite(output_data, 1, output_len, out);
    fclose(out);
    free(output_data);

    LOG_DEBUG("File decompressed: %s -> %s", input_path, output_path);

    return 0;
}

double compression_estimate_ratio(const unsigned char *data, size_t len) {
    if (len == 0) return 1.0;

    // Simple entropy estimation
    int byte_count[256] = {0};
    for (size_t i = 0; i < len; i++) {
        byte_count[data[i]]++;
    }

    double entropy = 0.0;
    for (int i = 0; i < 256; i++) {
        if (byte_count[i] > 0) {
            double p = (double)byte_count[i] / len;
            entropy -= p * (log(p) / log(2.0));
        }
    }

    // Estimate compression ratio based on entropy
    // Low entropy = high compressibility
    double max_entropy = 8.0; // bits per byte
    double estimated_ratio = entropy / max_entropy;

    return estimated_ratio < 0.3 ? 0.3 : estimated_ratio;
}
