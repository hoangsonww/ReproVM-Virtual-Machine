#ifndef CAS_H
#define CAS_H

#include <stddef.h>

// Initialize the CAS and cache directories under base_dir (e.g., ".reprovm")
int cas_init(const char *base_dir);

// Store a blob from memory; returns newly allocated hash string (hex), or NULL on error.
// The blob is written into CAS if not already present.
char *cas_store_blob_from_memory(const unsigned char *data, size_t len);

// Store a blob from an existing file; returns hash string (caller must free)
char *cas_store_blob_from_file(const char *path);

// Check if a blob exists already
int cas_blob_exists(const char *hash);

// Restore blob to a destination file (overwrites)
int cas_restore_blob_to_file(const char *hash, const char *dest);

// Get path to the object (internal use)
int cas_get_object_path(const char *hash, char *out_path, size_t sz);

// Base paths (you can read these if needed)
const char *cas_get_objects_root();
const char *cas_get_cache_root();
#endif
