#ifndef FLAT_HASH_MAP_STR_H
#define FLAT_HASH_MAP_STR_H

#include <stdint.h>
#include <stddef.h>

#define KEY_SIZE 32

typedef struct {
    uint8_t *ctrl;
    char (*keys)[KEY_SIZE];
    uint64_t *values;
    size_t capacity;
    size_t size;
} fhm_str;

fhm_str *fhm_str_create(size_t cap);
void fhm_str_free(fhm_str *m);

void fhm_str_put(fhm_str *m, const char *key, uint64_t value);
int fhm_str_get(fhm_str *m, const char *key, uint64_t *out);

#endif