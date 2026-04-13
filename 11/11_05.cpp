#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

template <std::ranges::input_range R>
void print_range(std::string_view title, R&& range) {
    std::cout << title << ": [";
    bool first = true;
    for (auto&& value : range) {
        if (!first) {
            std::cout << ", ";
        }
        first = false;
        std::cout << value;
    }
    std::cout << "]\n";
}

void print_title(std::string_view title) {
    std::cout << "\n==== " << title << " ====\n";
}

template <std::ranges::input_range R, class OutputIt, class Predicate, class UnaryOperation>
OutputIt transform_if(R&& input, OutputIt out, Predicate predicate, UnaryOperation operation) {
    using stored_type = std::ranges::range_value_t<R>;

    std::vector<stored_type> filtered;
    if constexpr (std::ranges::sized_range<R>) {
        filtered.reserve(static_cast<std::size_t>(std::ranges::size(input)));
    }

    std::ranges::copy_if(input, std::back_inserter(filtered), predicate);
    return std::ranges::transform(filtered, out, operation).out;
}

template <std::ranges::input_range R1, std::ranges::input_range R2>
requires std::ranges::sized_range<R1> && std::ranges::sized_range<R2>
double mae(const R1& actual, const R2& predicted) {
    if (std::ranges::size(actual) != std::ranges::size(predicted)) {
        throw std::invalid_argument("MAE requires ranges of equal length");
    }

    if (std::ranges::empty(actual)) {
        throw std::invalid_argument("MAE requires non-empty ranges");
    }

    const double total_error = std::transform_reduce(
        std::ranges::begin(actual), std::ranges::end(actual),
        std::ranges::begin(predicted),
        0.0,
        std::plus<>{},
        [](const auto& left, const auto& right) {
            return std::abs(static_cast<double>(left) - static_cast<double>(right));
        }
    );

    return total_error / static_cast<double>(std::ranges::size(actual));
}

template <std::ranges::input_range R1, std::ranges::input_range R2>
requires std::ranges::sized_range<R1> && std::ranges::sized_range<R2>
double mse(const R1& actual, const R2& predicted) {
    if (std::ranges::size(actual) != std::ranges::size(predicted)) {
        throw std::invalid_argument("MSE requires ranges of equal length");
    }

    if (std::ranges::empty(actual)) {
        throw std::invalid_argument("MSE requires non-empty ranges");
    }

    const double total_squared_error = std::transform_reduce(
        std::ranges::begin(actual), std::ranges::end(actual),
        std::ranges::begin(predicted),
        0.0,
        std::plus<>{},
        [](const auto& left, const auto& right) {
            const double diff = static_cast<double>(left) - static_cast<double>(right);
            return diff * diff;
        }
    );

    return total_squared_error / static_cast<double>(std::ranges::size(actual));
}

class Fibonacci : public std::ranges::view_interface<Fibonacci> {
public:
    explicit Fibonacci(std::size_t count) : count_(count) {}

    auto begin() const {
        return Iterator{0, count_, 0, 1};
    }

    auto end() const {
        return std::default_sentinel;
    }

    std::size_t size() const noexcept {
        return count_;
    }

private:
    class Iterator {
    public:
        using iterator_concept = std::input_iterator_tag;
        using iterator_category = std::input_iterator_tag;
        using value_type = std::uint64_t;
        using difference_type = std::ptrdiff_t;

        value_type operator*() const noexcept {
            return current_;
        }

        Iterator& operator++() noexcept {
            const value_type next_value = current_ + next_;
            current_ = next_;
            next_ = next_value;
            ++index_;
            return *this;
        }

        void operator++(int) noexcept {
            ++(*this);
        }

        bool operator==(std::default_sentinel_t) const noexcept {
            return index_ >= count_;
        }

        friend bool operator==(std::default_sentinel_t sentinel, const Iterator& it) noexcept {
            return it == sentinel;
        }

    private:
        friend class Fibonacci;

        Iterator(std::size_t index, std::size_t count, value_type current, value_type next) noexcept
            : index_(index), count_(count), current_(current), next_(next) {}

        std::size_t index_{0};
        std::size_t count_{0};
        value_type current_{0};
        value_type next_{1};
    };

    std::size_t count_{0};
};

void demo_ranges_algorithms() {
    print_title("ranges::replace, ranges::fill, ranges::unique, ranges::rotate, ranges::sample");

    std::vector<int> replaced{1, 2, 3, 2, 4, 2, 5};
    std::ranges::replace(replaced, 2, 20);
    print_range("replace 2 -> 20", replaced);

    std::array<int, 6> filled{};
    std::ranges::fill(filled, 7);
    print_range("fill by 7", filled);

    std::vector<int> unique_data{1, 1, 2, 2, 2, 3, 3, 4, 4, 5};
    auto unique_tail = std::ranges::unique(unique_data);
    unique_data.erase(unique_tail.begin(), unique_tail.end());
    print_range("unique compacted", unique_data);

    std::vector<int> rotated{10, 20, 30, 40, 50, 60};
    std::ranges::rotate(rotated, rotated.begin() + 2);
    print_range("rotate left by 2", rotated);

    std::vector<int> population{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> sampled;
    std::mt19937 generator(20260413);
    std::ranges::sample(population, std::back_inserter(sampled), 4, generator);
    print_range("sample 4 values", sampled);
}

void demo_transform_if() {
    print_title("custom transform_if via ranges::copy_if + ranges::transform");

    std::vector<int> source{1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> result;

    transform_if(
        source,
        std::back_inserter(result),
        [](int value) { return value % 2 == 0; },
        [](int value) { return value * value; }
    );

    print_range("source", source);
    print_range("transform_if -> square only even values", result);
}

void demo_metrics() {
    print_title("MAE and MSE");

    std::vector<double> actual{3.0, -0.5, 2.0, 7.0};
    std::vector<double> predicted{2.5, 0.0, 2.0, 8.0};

    print_range("actual", actual);
    print_range("predicted", predicted);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "MAE = " << mae(actual, predicted) << '\n';
    std::cout << "MSE = " << mse(actual, predicted) << '\n';
}

void demo_views() {
    print_title("views::filter, views::drop, views::join, views::zip, views::stride");

    std::vector<int> values{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto even_values = values | std::views::filter([](int x) { return x % 2 == 0; });
    print_range("filter even", even_values);

    auto tail = values | std::views::drop(3);
    print_range("drop first 3", tail);

    std::vector<std::vector<int>> blocks{{1, 2}, {3, 4, 5}, {6, 7}};
    auto flattened = blocks | std::views::join;
    print_range("join nested vectors", flattened);

    std::vector<std::string> names{"Ann", "Bob", "Chris"};
    std::vector<int> scores{91, 85, 99};

    std::cout << "zip names and scores: [";
    bool first = true;
    for (auto&& [name, score] : std::views::zip(names, scores)) {
        if (!first) {
            std::cout << ", ";
        }
        first = false;
        std::cout << name << ":" << score;
    }
    std::cout << "]\n";

    auto every_third = values | std::views::stride(3);
    print_range("stride by 3", every_third);
}

void demo_fibonacci_view() {
    print_title("custom Fibonacci view");

    Fibonacci fib{15};
    print_range("first 15 Fibonacci numbers", fib);

    auto after_five = fib | std::views::drop(5);
    print_range("drop first 5 Fibonacci numbers", after_five);

    auto sparse_fib = fib | std::views::stride(2);
    print_range("every second Fibonacci number", sparse_fib);

    std::vector<std::uint64_t> doubled_even_fib;
    transform_if(
        fib,
        std::back_inserter(doubled_even_fib),
        [](std::uint64_t value) { return value % 2 == 0; },
        [](std::uint64_t value) { return value * 2; }
    );
    print_range("double only even Fibonacci numbers", doubled_even_fib);
}

int main() {
    try {
        demo_ranges_algorithms();
        demo_transform_if();
        demo_metrics();
        demo_views();
        demo_fibonacci_view();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}