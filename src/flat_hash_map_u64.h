#ifndef FLAT_HASH_MAP_U64_H
#define FLAT_HASH_MAP_U64_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t *ctrl;
    uint64_t *keys;
    uint64_t *values;
    size_t capacity;
    size_t size;
} fhm_u64;

fhm_u64 *fhm_u64_create(size_t cap);
void fhm_u64_free(fhm_u64 *m);

void fhm_u64_put(fhm_u64 *m, uint64_t key, uint64_t value);
int fhm_u64_get(fhm_u64 *m, uint64_t key, uint64_t *out);

#endif