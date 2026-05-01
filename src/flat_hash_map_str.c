#include "flat_hash_map_str.h"
#include <stdlib.h>
#include <string.h>
#include <arm_neon.h>

#define GROUP_SIZE 16
#define EMPTY 0x80
#define MASK 0x7F

static inline uint64_t hash_str(const char *s) {
    uint64_t h = 1469598103934665603ULL;

    for (int i = 0; i < KEY_SIZE && s[i]; i++) {
        h ^= (uint8_t)s[i];
        h *= 1099511628211ULL;
    }

    return h;
}

static inline uint8_t h2(uint64_t h) {
    return h & MASK;
}

fhm_str *fhm_str_create(size_t cap) {
    fhm_str *m = malloc(sizeof(*m));

    size_t capacity = 1;
    while (capacity < cap) capacity <<= 1;

    m->capacity = capacity;
    m->size = 0;

    m->ctrl = aligned_alloc(16, capacity + GROUP_SIZE);
    m->keys = aligned_alloc(16, capacity * sizeof(char[KEY_SIZE]));
    m->values = aligned_alloc(16, capacity * sizeof(uint64_t));

    memset(m->ctrl, EMPTY, capacity + GROUP_SIZE);

    return m;
}

void fhm_str_free(fhm_str *m) {
    free(m->ctrl);
    free(m->keys);
    free(m->values);
    free(m);
}

static inline uint16_t match_group(uint8_t *ctrl, uint8_t fp) {
    uint8x16_t v = vld1q_u8(ctrl);
    uint8x16_t f = vdupq_n_u8(fp);
    uint8x16_t cmp = vceqq_u8(v, f);

    uint64x2_t r = vreinterpretq_u64_u8(cmp);
    return (uint16_t)((vgetq_lane_u64(r, 0) | vgetq_lane_u64(r, 1)) & 0xFFFF);
}

static inline uint16_t match_empty(uint8_t *ctrl) {
    uint8x16_t v = vld1q_u8(ctrl);
    uint8x16_t e = vdupq_n_u8(EMPTY);
    uint8x16_t cmp = vceqq_u8(v, e);

    uint64x2_t r = vreinterpretq_u64_u8(cmp);
    return (uint16_t)((vgetq_lane_u64(r, 0) | vgetq_lane_u64(r, 1)) & 0xFFFF);
}

int fhm_str_get(fhm_str *m, const char *key, uint64_t *out) {
    uint64_t h = hash_str(key);
    uint8_t fp = h2(h);

    size_t mask = m->capacity - 1;
    size_t i = h & mask;

    while (1) {
        uint8_t *group = m->ctrl + i;
        uint16_t match = match_group(group, fp);

        while (match) {
            int bit = __builtin_ctz(match);
            size_t idx = (i + bit) & mask;

            if (strncmp(m->keys[idx], key, KEY_SIZE) == 0) {
                *out = m->values[idx];
                return 1;
            }

            match &= match - 1;
        }

        if (match_empty(group)) return 0;

        i = (i + GROUP_SIZE) & mask;
    }
}

void fhm_str_put(fhm_str *m, const char *key, uint64_t value) {
    uint64_t h = hash_str(key);
    uint8_t fp = h2(h);

    size_t mask = m->capacity - 1;
    size_t i = h & mask;

    while (1) {
        uint8_t *group = m->ctrl + i;
        uint16_t empty = match_empty(group);

        if (empty) {
            int bit = __builtin_ctz(empty);
            size_t idx = (i + bit) & mask;

            m->ctrl[idx] = fp;
            strncpy(m->keys[idx], key, KEY_SIZE);
            m->values[idx] = value;
            m->size++;
            return;
        }

        i = (i + GROUP_SIZE) & mask;
    }
}