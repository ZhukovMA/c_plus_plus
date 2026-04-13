#include <benchmark/benchmark.h>

#include <cstdint>
#include <functional>
#include <utility>

#if defined(__GNUC__) || defined(__clang__)
    #define NOINLINE [[gnu::noinline]]
    #define LAMBDA_NOINLINE [[gnu::noinline]]
#else
    #error "This code is intended to be compiled with GCC or Clang."
#endif

using value_type = std::uint64_t;

[[nodiscard]] inline value_type payload(value_type x) noexcept {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

NOINLINE value_type free_function(value_type x) noexcept {
    return payload(x);
}

class MemberCall final {
public:
    NOINLINE value_type call(value_type x) const noexcept {
        return payload(x);
    }
};

class VirtualBase {
public:
    virtual ~VirtualBase() = default;
    virtual value_type call(value_type x) const noexcept = 0;
};

class VirtualDerived final : public VirtualBase {
public:
    NOINLINE value_type call(value_type x) const noexcept override {
        return payload(x);
    }
};

struct FunctionObject final {
    NOINLINE value_type operator()(value_type x) const noexcept {
        return payload(x);
    }
};

NOINLINE std::function<value_type(value_type)> make_std_function() {
    return [] LAMBDA_NOINLINE (value_type x) noexcept -> value_type {
        return payload(x);
    };
}

template <class Invoker>
void run_case(benchmark::State& state, const Invoker& invoker) {
    value_type x = 0x1234567890ABCDEFULL;

    for (auto _ : state) {
        benchmark::DoNotOptimize(x);
        x = invoker(x);
        benchmark::DoNotOptimize(x);
    }

    benchmark::ClobberMemory();
    state.SetItemsProcessed(state.iterations());
}

template <class Invoker>
void add_case(const char* name, Invoker invoker) {
    benchmark::RegisterBenchmark(
        name,
        [invoker = std::move(invoker)](benchmark::State& state) {
            run_case(state, invoker);
        })
        ->Unit(benchmark::kNanosecond)
        ->Repetitions(10)
        ->ReportAggregatesOnly(true);
}

int main(int argc, char** argv) {
    const MemberCall member_object{};
    const VirtualDerived derived_object{};
    const VirtualBase& virtual_object = derived_object;
    const FunctionObject function_object{};

    const auto lambda_auto = [] LAMBDA_NOINLINE (value_type x) noexcept -> value_type {
        return payload(x);
    };

    const std::function<value_type(value_type)> lambda_std_function = make_std_function();

    add_case("free_function", [](value_type x) noexcept {
        return free_function(x);
    });

    add_case("member_function", [&member_object](value_type x) noexcept {
        return member_object.call(x);
    });

    add_case("virtual_function", [&virtual_object](value_type x) noexcept {
        return virtual_object.call(x);
    });

    add_case("function_object", [function_object](value_type x) noexcept {
        return function_object(x);
    });

    add_case("lambda_auto", [lambda_auto](value_type x) noexcept {
        return lambda_auto(x);
    });

    add_case("lambda_std_function", [&lambda_std_function](value_type x) {
        return lambda_std_function(x);
    });

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}