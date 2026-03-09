#include <algorithm>
#include <benchmark/benchmark.h>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

template <typename T>
void order(std::vector<T>& v, std::size_t left, std::size_t right) {
    for (auto i = left + 1; i < right; ++i) {
        for (auto j = i; j > left; --j) {
            if (v[j - 1] > v[j]) {
                std::swap(v[j], v[j - 1]);
            }
        }
    }
}

template <typename T>
T median_of_three(std::vector<T>& v,
                  std::ptrdiff_t l,
                  std::ptrdiff_t m,
                  std::ptrdiff_t r) {
    if (v[l] > v[m]) {
        std::swap(v[l], v[m]);
    }
    if (v[m] > v[r]) {
        std::swap(v[m], v[r]);
    }
    if (v[l] > v[m]) {
        std::swap(v[l], v[m]);
    }
    return v[m];
}

template <typename T>
std::ptrdiff_t hoare_partition(std::vector<T>& v,
                               std::size_t left,
                               std::size_t right) {
    using idx = std::ptrdiff_t;

    idx l = static_cast<idx>(left);
    idx r = static_cast<idx>(right) - 1;
    idx m = l + ((r - l) >> 1);

    const T pivot = median_of_three(v, l, m, r);

    idx i = l - 1;
    idx j = r + 1;

    for (;;) {
        do {
            ++i;
        } while (v[i] < pivot);

        do {
            --j;
        } while (v[j] > pivot);

        if (i >= j) {
            return j;
        }

        std::swap(v[i], v[j]);
    }
}

template <typename T>
void qsplit(std::vector<T>& v,
            std::size_t left,
            std::size_t right,
            std::size_t threshold) {
    while (right - left > threshold) {
        std::ptrdiff_t p = hoare_partition(v, left, right);

        std::size_t l1 = left;
        std::size_t r1 = static_cast<std::size_t>(p + 1);
        std::size_t l2 = static_cast<std::size_t>(p + 1);
        std::size_t r2 = right;

        if (r1 - l1 < r2 - l2) {
            if (r1 - l1 > threshold) {
                qsplit(v, l1, r1, threshold);
            }
            left = l2;
            right = r2;
        } else {
            if (r2 - l2 > threshold) {
                qsplit(v, l2, r2, threshold);
            }
            left = l1;
            right = r1;
        }
    }

    if (right - left > 1) {
        order(v, left, right);
    }
}

template <typename T>
void sort(std::vector<T>& v, std::size_t threshold = 16) {
    qsplit(v, 0, v.size(), threshold);
}

static std::vector<double> make_reverse_sorted_input() {
    constexpr std::size_t size = 10'000;
    std::vector<double> v(size);

    for (std::size_t i = 0; i < size; ++i) {
        v[i] = static_cast<double>(size - i);
    }

    return v;
}

static void BM_HybridSortThreshold(benchmark::State& state) {
    const std::size_t threshold = static_cast<std::size_t>(state.range(0));
    const std::vector<double> source = make_reverse_sorted_input();
    std::vector<double> work;

    for (auto _ : state) {
        state.PauseTiming();
        work = source;
        state.ResumeTiming();

        sort(work, threshold);

        auto* data = work.data();
        benchmark::DoNotOptimize(data);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(
        static_cast<int64_t>(state.iterations()) *
        static_cast<int64_t>(source.size()));
}

BENCHMARK(BM_HybridSortThreshold)
    ->RangeMultiplier(2)
    ->Range(2, 64)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();