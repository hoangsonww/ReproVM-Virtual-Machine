#ifndef REMOTE_CAS_H
#define REMOTE_CAS_H

#include <stddef.h>

// Remote CAS backends
typedef enum {
    REMOTE_CAS_NONE = 0,
    REMOTE_CAS_HTTP = 1,
    REMOTE_CAS_S3 = 2,
    REMOTE_CAS_GCS = 3,
    REMOTE_CAS_AZURE = 4,
    REMOTE_CAS_CUSTOM = 99
} RemoteCASBackend;

// Remote CAS configuration
typedef struct {
    RemoteCASBackend backend;
    char endpoint[512];
    char access_key[256];
    char secret_key[256];
    char bucket[256];
    char region[64];
    int use_ssl;
    int enabled;
} RemoteCASConfig;

// Global remote CAS config
extern RemoteCASConfig g_remote_cas_config;

// Initialize remote CAS
int remote_cas_init(RemoteCASBackend backend, const char *endpoint,
                    const char *access_key, const char *secret_key);

// Store blob to remote
int remote_cas_store(const char *hash, const unsigned char *data, size_t len);

// Retrieve blob from remote
int remote_cas_retrieve(const char *hash, unsigned char **data, size_t *len);

// Check if blob exists remotely
int remote_cas_exists(const char *hash);

// Sync local to remote
int remote_cas_sync_to_remote(void);

// Sync remote to local
int remote_cas_sync_from_remote(void);

// Backend-specific implementations
int remote_cas_http_store(const char *url, const char *hash,
                          const unsigned char *data, size_t len);
int remote_cas_http_retrieve(const char *url, const char *hash,
                             unsigned char **data, size_t *len);

int remote_cas_s3_store(const char *bucket, const char *hash,
                        const unsigned char *data, size_t len);
int remote_cas_s3_retrieve(const char *bucket, const char *hash,
                           unsigned char **data, size_t *len);

// Cleanup
void remote_cas_cleanup(void);

#endif // REMOTE_CAS_H
