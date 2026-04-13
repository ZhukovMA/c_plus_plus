#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <vector>

template <std::random_access_iterator RandomIt>
void order(RandomIt first, RandomIt last) {
    if (first == last) {
        return;
    }

    for (auto it = std::next(first); it != last; ++it) {
        for (auto jt = it; jt != first && *std::prev(jt) > *jt; --jt) {
            std::iter_swap(std::prev(jt), jt);
        }
    }
}

template <std::random_access_iterator RandomIt>
auto median_of_three(RandomIt left, RandomIt middle, RandomIt right) -> std::iter_value_t<RandomIt>
{
    if (*left > *middle) {
        std::iter_swap(left, middle);
    }
    if (*middle > *right) {
        std::iter_swap(middle, right);
    }
    if (*left > *middle) {
        std::iter_swap(left, middle);
    }

    return *middle;
}

template <std::random_access_iterator RandomIt>
RandomIt hoare_partition(RandomIt first, RandomIt last) {
    auto middle = first;
    std::advance(middle, std::distance(first, last) / 2);

    const auto pivot = median_of_three(first, middle, std::prev(last));

    auto left = first;
    auto right = std::prev(last);

    for (;;) {
        while (*left < pivot) {
            ++left;
        }

        while (*right > pivot) {
            --right;
        }

        if (left >= right) {
            return std::next(right);
        }

        std::iter_swap(left, right);
        ++left;
        --right;
    }
}

template <std::random_access_iterator RandomIt>
void qsplit(RandomIt first, RandomIt last) {
    while (std::distance(first, last) > 16) {
        auto cut = hoare_partition(first, last);

        const auto left_size = std::distance(first, cut);
        const auto right_size = std::distance(cut, last);

        if (left_size < right_size) {
            if (left_size > 1) {
                qsplit(first, cut);
            }
            first = cut;
        } else {
            if (right_size > 1) {
                qsplit(cut, last);
            }
            last = cut;
        }
    }

    if (std::distance(first, last) > 1) {
        order(first, last);
    }
}

template <std::random_access_iterator RandomIt>
void sort(RandomIt first, RandomIt last) {
    if (std::distance(first, last) > 1) {
        qsplit(first, last);
    }
}

int main() {
    auto size = 1'000uz;

    std::vector<int> v(size, 0);
    for (auto i = 0uz; i < size; ++i) {
        v[i] = static_cast<int>(size - i);
    }

    sort(v.begin(), v.end());

    assert(std::ranges::is_sorted(v));
}