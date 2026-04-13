#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace lab {

struct Check {
    std::size_t total = 0;
    std::size_t failed = 0;

    void expect(bool condition, std::string_view message) {
        ++total;
        if (!condition) {
            ++failed;
            std::cout << "FAIL " << message << '\n';
        }
    }

    void summary() const {
        if (failed == 0) {
            std::cout << "All checks passed: " << total << " out of " << total << ".\n";
        } else {
            std::cout << "Failed checks: " << failed << " out of " << total << ".\n";
        }
    }
};

std::string hex_addr(const void* p) {
    std::ostringstream out;
    out << "0x" << std::hex << std::uppercase << reinterpret_cast<std::uintptr_t>(p);
    return out.str();
}

template <class T>
struct VectorGrowthStep {
    std::size_t size_after_push{};
    std::size_t old_capacity{};
    std::size_t new_capacity{};
    long double ratio{};
};

template <class T>
std::vector<VectorGrowthStep<T>> trace_vector_growth(std::size_t insertions) {
    std::vector<T> v;
    std::vector<VectorGrowthStep<T>> result;

    std::size_t old_capacity = v.capacity();

    for (std::size_t i = 0; i < insertions; ++i) {
        v.push_back(T{});

        if (v.capacity() != old_capacity) {
            const std::size_t new_capacity = v.capacity();
            const long double ratio = (old_capacity == 0) ? std::numeric_limits<long double>::infinity() : static_cast<long double>(new_capacity) / static_cast<long double>(old_capacity);

            result.push_back(VectorGrowthStep<T>{
                .size_after_push = v.size(),
                .old_capacity = old_capacity,
                .new_capacity = new_capacity,
                .ratio = ratio
            });

            old_capacity = new_capacity;
        }
    }

    return result;
}

long double detect_stable_vector_ratio(const std::vector<VectorGrowthStep<int>>& steps) {
    std::map<long double, int> freq;

    for (const auto& s : steps) {
        if (s.old_capacity == 0) {
            continue;
        }
        long double rounded = std::round(s.ratio * 1000.0L) / 1000.0L;
        ++freq[rounded];
    }

    auto it = std::max_element(
        freq.begin(), freq.end(),
        [](const auto& a, const auto& b) {
            if (a.second != b.second) return a.second < b.second;
            return a.first < b.first;
        }
    );

    return it == freq.end() ? 0.0L : it->first;
}

struct DequeStep {
    std::size_t index{};
    std::uintptr_t address{};
    std::intptr_t delta_from_prev{};
    bool new_block{};
};

std::vector<DequeStep> trace_deque_addresses(std::size_t insertions) {
    std::deque<int> dq;
    std::vector<DequeStep> result;
    result.reserve(insertions);

    std::uintptr_t prev = 0;

    for (std::size_t i = 0; i < insertions; ++i) {
        dq.push_back(static_cast<int>(i));

        const std::uintptr_t current = reinterpret_cast<std::uintptr_t>(&dq.back());

        const std::intptr_t delta = (i == 0) ? 0 : static_cast<std::intptr_t>(current) - static_cast<std::intptr_t>(prev);

        const bool new_block = (i != 0) && (delta != static_cast<std::intptr_t>(sizeof(int)));

        result.push_back(DequeStep{
            .index = i,
            .address = current,
            .delta_from_prev = delta,
            .new_block = new_block
        });

        prev = current;
    }

    return result;
}

std::vector<std::size_t> extract_complete_deque_block_lengths( const std::vector<DequeStep>& trace) {
    std::vector<std::size_t> lengths;

    if (trace.empty()) {
        return lengths;
    }

    std::size_t current_len = 1;

    for (std::size_t i = 1; i < trace.size(); ++i) {
        if (!trace[i].new_block) {
            ++current_len;
        } else {
            lengths.push_back(current_len);
            current_len = 1;
        }
    }

    return lengths;
}

std::optional<std::size_t> detect_common_block_length(const std::vector<std::size_t>& lengths) {
    if (lengths.empty()) {
        return std::nullopt;
    }

    std::map<std::size_t, std::size_t> freq;
    for (auto len : lengths) {
        ++freq[len];
    }

    auto it = std::max_element(freq.begin(), freq.end(), [](const auto& a, const auto& b) {
            if (a.second != b.second) return a.second < b.second;
            return a.first < b.first;
        }
    );

    return it->first;
}

void print_vector_report(const std::vector<VectorGrowthStep<int>>& steps) {

    for (const auto& step : steps) {
        std::cout << "size = " << std::setw(5) << step.size_after_push << "  capacity: " << std::setw(5) << step.old_capacity << " -> " << std::setw(5) << step.new_capacity;

        if (step.old_capacity != 0) {
            std::cout << "  ratio=" << std::fixed << std::setprecision(3) << static_cast<double>(step.ratio) << '\n';
        }
    }

    const auto stable = detect_stable_vector_ratio(steps);

    if (stable > 0.0L) {
        std::cout << "Empirically detected growth factor: approximately x" << std::fixed << std::setprecision(3) << static_cast<double>(stable) << "\n\n";
    } else {
        std::cout << "Unable to determine the growth factor: not enough data.\n";
    }
}

void print_deque_report(const std::vector<DequeStep>& trace, std::size_t common_block_len ) {

    for (const auto& step : trace) {
        if (step.index < 20 || step.new_block || step.index + 5 >= trace.size()) {
            std::cout << "index = " << std::setw(4) << step.index << "  addr=" << hex_addr(reinterpret_cast<const void*>(step.address)) << "  delta=" << std::setw(6) << step.delta_from_prev;

            if (step.new_block) {
                std::cout << "  transition to a new memory page";
            }
            std::cout << '\n';
        }
    }

    std::cout << "\n Empirically detected page size: " << common_block_len << " elements" << " = " << common_block_len * sizeof(int) << " bytes\n\n";
}

void verify_vector(Check& check, const std::vector<VectorGrowthStep<int>>& steps) {
    check.expect(!steps.empty(), "vector must increase capacity at least once");

    for (const auto& step : steps) {
        check.expect(step.new_capacity > step.old_capacity, "after reallocation, new capacity must be greater than old capacity");
    }

    for (std::size_t i = 2; i < steps.size(); ++i) {
        check.expect(steps[i].new_capacity >= steps[i - 1].new_capacity, "vector capacity must not decrease during repeated push_back operations");
    }
}

void verify_deque(Check& check, std::size_t insertions, std::size_t expected_block_len) {
    std::deque<int> dq;
    std::vector<const int*> saved_addresses;
    saved_addresses.reserve(insertions);

    for (std::size_t i = 0; i < insertions; ++i) {
        dq.push_back(static_cast<int>(i));
        saved_addresses.push_back(&dq[i]);

        for (std::size_t watched = 0; watched < saved_addresses.size(); watched += expected_block_len) {
            check.expect(saved_addresses[watched] == &dq[watched], "address of an already inserted deque element must remain unchanged during push_back");
        }
    }
}

} // namespace lab

int main() {
    using namespace lab;

    constexpr std::size_t vector_insertions = 200;
    constexpr std::size_t deque_insertions = 400;

    Check check;

    const auto vector_steps = trace_vector_growth<int>(vector_insertions);
    print_vector_report(vector_steps);
    verify_vector(check, vector_steps);

    const auto deque_trace = trace_deque_addresses(deque_insertions);
    const auto full_blocks = extract_complete_deque_block_lengths(deque_trace);
    const auto common_block = detect_common_block_length(full_blocks);

    if (!common_block) {
        return 1;
    }

    print_deque_report(deque_trace, *common_block);
    check.expect(*common_block > 0, "deque page size must be positive");
    verify_deque(check, deque_insertions, *common_block);

    check.summary();
    return check.failed == 0 ? 0 : 1;
}