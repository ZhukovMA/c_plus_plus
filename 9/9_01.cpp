#include <cstddef>
#include <iostream>
#include <source_location>
#include <string_view>

class Tracer final {
public:
    Tracer(const std::source_location location = std::source_location::current()) noexcept
        : location_(location), depth_(current_depth_++) {
        print("in");
    }

    ~Tracer() noexcept {
        if (current_depth_ > 0) {
            --current_depth_;
        }
        print("out");
    }

    Tracer(const Tracer&) = delete;
    Tracer& operator=(const Tracer&) = delete;
    Tracer(Tracer&&) = delete;
    Tracer& operator=(Tracer&&) = delete;

private:
    std::source_location location_;
    std::size_t depth_{};

    inline static thread_local std::size_t current_depth_ = 0;

    void print(std::string_view stage) const {
        std::cout << '[' << stage << "] ";

        for (std::size_t i = 0; i < depth_; ++i) {
            std::cout << "  ";
        }

        std::cout << location_.function_name() << " in file: " << location_.file_name() << " in line: " << location_.line() << '\n';
    }
};

#ifndef NDEBUG

#define TRACER_IMPL(x, y) x##y
#define TRACER(x, y) TRACER_IMPL(x, y)

#define trace() [[maybe_unused]] Tracer TRACER(_tracer_guard_, __LINE__)

#else

#define trace() ((void)0)

#endif

void test_func3() {
    trace();
}

void test_func2() {
    trace();
    test_func3();
}

void test_func1() {
    trace();
    test_func2();
}

int main() {
    trace();
    test_func1();
}