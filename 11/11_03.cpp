#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

template <std::random_access_iterator RandomIt, class Comp>
requires std::indirect_strict_weak_order<Comp, RandomIt>
void order(RandomIt first, RandomIt last, Comp& comp) {
    if (first == last) {
        return;
    }

    for (auto it = std::next(first); it != last; ++it) {
        for (auto jt = it; jt != first && std::invoke(comp, *jt, *std::prev(jt)); --jt) {
            std::iter_swap(std::prev(jt), jt);
        }
    }
}

template <std::random_access_iterator RandomIt, class Comp>
requires std::indirect_strict_weak_order<Comp, RandomIt>
auto median_of_three(RandomIt left, RandomIt middle, RandomIt right, Comp& comp)
    -> std::iter_value_t<RandomIt>
{
    if (std::invoke(comp, *middle, *left)) {
        std::iter_swap(left, middle);
    }
    if (std::invoke(comp, *right, *middle)) {
        std::iter_swap(middle, right);
    }
    if (std::invoke(comp, *middle, *left)) {
        std::iter_swap(left, middle);
    }

    return *middle;
}

template <std::random_access_iterator RandomIt, class Comp>
requires std::indirect_strict_weak_order<Comp, RandomIt>
RandomIt hoare_partition(RandomIt first, RandomIt last, Comp& comp) {
    auto middle = first;
    std::advance(middle, std::distance(first, last) / 2);

    const auto pivot = median_of_three(first, middle, std::prev(last), comp);

    auto left = first;
    auto right = std::prev(last);

    for (;;) {
        while (std::invoke(comp, *left, pivot)) {
            ++left;
        }

        while (std::invoke(comp, pivot, *right)) {
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

template <std::random_access_iterator RandomIt, class Comp>
requires std::indirect_strict_weak_order<Comp, RandomIt>
void qsplit(RandomIt first, RandomIt last, Comp& comp) {
    while (std::distance(first, last) > 16) {
        auto cut = hoare_partition(first, last, comp);

        const auto left_size = std::distance(first, cut);
        const auto right_size = std::distance(cut, last);

        if (left_size < right_size) {
            if (left_size > 1) {
                qsplit(first, cut, comp);
            }
            first = cut;
        } else {
            if (right_size > 1) {
                qsplit(cut, last, comp);
            }
            last = cut;
        }
    }

    if (std::distance(first, last) > 1) {
        order(first, last, comp);
    }
}

template <std::random_access_iterator RandomIt, class Comp = std::less<>>
requires std::indirect_strict_weak_order<Comp, RandomIt>
void sort(RandomIt first, RandomIt last, Comp comp = {}) {
    if (std::distance(first, last) > 1) {
        qsplit(first, last, comp);
    }
}

bool greater_free(int left, int right) {
    return left > right;
}

int main() {
    {
        std::vector<int> v{9, 2, 7, 1, 5, 3, 8, 4, 6};
        sort(v.begin(), v.end(), greater_free);
        assert(std::ranges::is_sorted(v, greater_free));
    }

    {
        std::vector<int> v{9, 2, 7, 1, 5, 3, 8, 4, 6};
        sort(v.begin(), v.end(), std::less<>{});
        assert(std::ranges::is_sorted(v, std::less<>{}));
    }

    {
        std::vector<int> v{9, 2, 7, 1, 5, 3, 8, 4, 6};

        auto even_first = [](int left, int right) {
            const bool left_even = (left % 2 == 0);
            const bool right_even = (right % 2 == 0);

            if (left_even != right_even) {
                return left_even > right_even;
            }

            return left < right;
        };

        sort(v.begin(), v.end(), even_first);
        assert(std::ranges::is_sorted(v, even_first));
    }
}