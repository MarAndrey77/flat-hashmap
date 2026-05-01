#include <iostream>
#include <vector>
#include <random>
#include <unordered_map>
#include <chrono>

extern "C" {
#include "flat_hash_map_u64.h"
#include "flat_hash_map_str.h"
}

using Clock = std::chrono::high_resolution_clock;

static std::vector<uint64_t> gen_u64(size_t n) {
    std::vector<uint64_t> v(n);
    std::mt19937_64 rng(42);
    for (auto &x : v) x = rng();
    return v;
}

static std::vector<std::string> gen_str(size_t n) {
    std::vector<std::string> v(n);
    std::mt19937_64 rng(42);

    for (size_t i = 0; i < n; i++) {
        uint64_t x = rng();
        v[i] = std::to_string(x).substr(0, 31);
    }
    return v;
}

void bench_u64(const std::vector<uint64_t>& keys) {
    std::cout << "\n==== U64 BENCH ====\n";

    fhm_u64* fhm = fhm_u64_create(keys.size());
    std::unordered_map<uint64_t,uint64_t> um;
    um.reserve(keys.size());

    auto t1 = Clock::now();
    for (auto k : keys) fhm_u64_put(fhm, k, k);
    auto t2 = Clock::now();

    for (size_t i = 0; i < keys.size(); i++)
        um.emplace(keys[i], i);

    auto t3 = Clock::now();

    std::cout << "flat insert: "
              << std::chrono::duration<double>(t2 - t1).count() << "\n";

    std::cout << "std insert: "
              << std::chrono::duration<double>(t3 - t2).count() << "\n";

    volatile uint64_t sink = 0;

    t1 = Clock::now();
    for (auto k : keys) {
        uint64_t v;
        fhm_u64_get(fhm, k, &v);
        sink ^= v;
    }
    t2 = Clock::now();

    for (auto k : keys)
        sink ^= um[k];

    t3 = Clock::now();

    std::cout << "flat lookup: "
              << std::chrono::duration<double>(t2 - t1).count() << "\n";

    std::cout << "std lookup: "
              << std::chrono::duration<double>(t3 - t2).count() << "\n";

    fhm_u64_free(fhm);
}

void bench_str(const std::vector<std::string>& keys) {
    std::cout << "\n==== STRING BENCH ====\n";

    fhm_str* fhm = fhm_str_create(keys.size());
    std::unordered_map<std::string,uint64_t> um;
    um.reserve(keys.size());

    auto t1 = Clock::now();
    for (auto &k : keys) fhm_str_put(fhm, k.c_str(), 1);
    auto t2 = Clock::now();

    for (size_t i = 0; i < keys.size(); i++)
        um.emplace(keys[i], i);

    auto t3 = Clock::now();

    std::cout << "flat insert: "
              << std::chrono::duration<double>(t2 - t1).count() << "\n";

    std::cout << "std insert: "
              << std::chrono::duration<double>(t3 - t2).count() << "\n";

    volatile uint64_t sink = 0;

    t1 = Clock::now();
    for (auto &k : keys) {
        uint64_t v;
        fhm_str_get(fhm, k.c_str(), &v);
        sink ^= v;
    }
    t2 = Clock::now();

    for (auto &k : keys)
        sink ^= um[k];

    t3 = Clock::now();

    std::cout << "flat lookup: "
              << std::chrono::duration<double>(t2 - t1).count() << "\n";

    std::cout << "std lookup: "
              << std::chrono::duration<double>(t3 - t2).count() << "\n";

    fhm_str_free(fhm);
}

int main() {
    const size_t N = 5000000;

    auto u64 = gen_u64(N);
    auto str = gen_str(N);

    bench_u64(u64);
    bench_str(str);
}