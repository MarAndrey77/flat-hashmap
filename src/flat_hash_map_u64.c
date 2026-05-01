#include "flat_hash_map_u64.h"
#include <stdlib.h>
#include <string.h>
#include <arm_neon.h>

#define GROUP_SIZE 16
#define EMPTY 0x80
#define MASK 0x7F

static inline uint64_t hash_u64(uint64_t x) {
    x += 0x9e3779b97f4a7c15;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    return x ^ (x >> 31);
}

static inline uint8_t h2(uint64_t h) {
    return h & MASK;
}

fhm_u64 *fhm_u64_create(size_t cap) {
    fhm_u64 *m = malloc(sizeof(*m));

    size_t capacity = 1;
    while (capacity < cap) capacity <<= 1;

    m->capacity = capacity;
    m->size = 0;

    m->ctrl = aligned_alloc(16, capacity + GROUP_SIZE);
    m->keys = aligned_alloc(16, capacity * sizeof(uint64_t));
    m->values = aligned_alloc(16, capacity * sizeof(uint64_t));

    memset(m->ctrl, EMPTY, capacity + GROUP_SIZE);

    return m;
}

void fhm_u64_free(fhm_u64 *m) {
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

int fhm_u64_get(fhm_u64 *m, uint64_t key, uint64_t *out) {
    uint64_t h = hash_u64(key);
    uint8_t fp = h2(h);

    size_t mask = m->capacity - 1;
    size_t i = h & mask;

    while (1) {
        uint8_t *group = m->ctrl + i;
        uint16_t match = match_group(group, fp);

        while (match) {
            int bit = __builtin_ctz(match);
            size_t idx = (i + bit) & mask;

            if (m->keys[idx] == key) {
                *out = m->values[idx];
                return 1;
            }

            match &= match - 1;
        }

        if (match_empty(group)) return 0;

        i = (i + GROUP_SIZE) & mask;
    }
}

void fhm_u64_put(fhm_u64 *m, uint64_t key, uint64_t value) {
    uint64_t h = hash_u64(key);
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
            m->keys[idx] = key;
            m->values[idx] = value;
            m->size++;
            return;
        }

        i = (i + GROUP_SIZE) & mask;
    }
}