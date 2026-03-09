#include <algorithm>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

template <typename T>
void order(std::vector<T> &v, std::size_t left, std::size_t right) {
    for (auto i = left + 1; i < right; ++i) {
        for (auto j = i; j > left; --j) {
            if (v[j - 1] > v[j]) {
                std::swap(v[j], v[j - 1]);
            }
        }
    }
}

template <typename T>
T median_of_three(std::vector<T> &v,
                  std::ptrdiff_t l, std::ptrdiff_t m, std::ptrdiff_t r) {
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
std::ptrdiff_t hoare_partition(std::vector<T> &v, std::size_t left, std::size_t right) {
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
void qsplit(std::vector<T> &v, std::size_t left, std::size_t right) {
    while (right - left > 16) {
        std::ptrdiff_t p = hoare_partition(v, left, right);

        std::size_t l1 = left;
        std::size_t r1 = static_cast<std::size_t>(p + 1);
        std::size_t l2 = static_cast<std::size_t>(p + 1);
        std::size_t r2 = right;

        if (r1 - l1 < r2 - l2) {
            if (r1 - l1 > 16) {
                qsplit(v, l1, r1);
            }
            left = l2;
            right = r2;
        } else {
            if (r2 - l2 > 16) {
                qsplit(v, l2, r2);
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
void sort(std::vector<T> &v) {
    qsplit(v, 0, std::size(v));
}

int main() {
    auto size = 1'000uz;

    std::vector<int> v(size, 0);
    for (auto i = 0uz; i < size; ++i) {
        v[i] = size - i;
    }

    sort(v);

    assert(std::ranges::is_sorted(v));
}