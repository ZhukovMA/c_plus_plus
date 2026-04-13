#include <benchmark/benchmark.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <new>
#include <random>
#include <utility>
#include <vector>

class Allocator
{
public:
    enum class SearchPolicy
    {
        first_fit,
        best_fit
    };

    explicit Allocator(std::size_t bytes,
                       SearchPolicy policy = SearchPolicy::first_fit)
        : m_capacity(bytes),
          m_policy(policy),
          m_find(policy == SearchPolicy::first_fit
                     ? &Allocator::find_first
                     : &Allocator::find_best)
    {
        assert(m_capacity >= sizeof(Node) + 1);

        m_begin = ::operator new(m_capacity, std::align_val_t{kAlignment});

        m_head = as_node(m_begin);
        m_head->size = m_capacity - sizeof(Header);
        m_head->next = nullptr;
    }

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;

    ~Allocator()
    {
        ::operator delete(m_begin, std::align_val_t{kAlignment});
    }

    [[nodiscard]] void* allocate(std::size_t bytes)
    {
        if (bytes == 0)
        {
            bytes = 1;
        }

        const std::size_t required = round_up(bytes, kAlignment);
        const auto [block, previous] = (this->*m_find)(required);

        if (!block)
        {
            return nullptr;
        }

        const std::size_t step = sizeof(Header) + required;
        const bool can_split =
            block->size >= required + sizeof(Header) + min_payload_for_free_block();

        std::size_t stored_size = required;

        if (can_split)
        {
            auto* tail = as_node(as_bytes(block) + step);
            tail->size = block->size - step;
            tail->next = block->next;

            if (previous)
            {
                previous->next = tail;
            }
            else
            {
                m_head = tail;
            }
        }
        else
        {
            stored_size = block->size;

            if (previous)
            {
                previous->next = block->next;
            }
            else
            {
                m_head = block->next;
            }
        }

        auto* header = as_header(block);
        header->size = stored_size;

        return as_bytes(block) + sizeof(Header);
    }

    void deallocate(void* ptr)
    {
        if (!ptr)
        {
            return;
        }

        auto* node = as_node(as_bytes(ptr) - sizeof(Header));

        Node* previous = nullptr;
        Node* current = m_head;

        while (current && current < node)
        {
            previous = current;
            current = current->next;
        }

        node->next = current;

        if (previous)
        {
            previous->next = node;
        }
        else
        {
            m_head = node;
        }

        merge_adjacent(previous, node);
    }

    void dump_free_list() const
    {
        std::cout << "policy = "
                  << (m_policy == SearchPolicy::first_fit ? "first_fit" : "best_fit")
                  << ", free blocks: ";

        const Node* current = m_head;
        while (current)
        {
            std::cout << '[' << static_cast<const void*>(current)
                      << ", size = " << current->size << "] ";
            current = current->next;
        }

        std::cout << '\n';
    }

private:
    struct Node
    {
        std::size_t size{};
        Node* next{};
    };

    struct alignas(std::max_align_t) Header
    {
        std::size_t size{};
    };

    struct FindResult
    {
        Node* current{};
        Node* previous{};
    };

    using FindFn = FindResult (Allocator::*)(std::size_t) const;

    static constexpr std::size_t kAlignment = alignof(std::max_align_t);

    static constexpr std::size_t round_up(std::size_t value, std::size_t alignment)
    {
        const std::size_t remainder = value % alignment;
        return remainder == 0 ? value : value + (alignment - remainder);
    }

    static constexpr std::size_t min_payload_for_free_block()
    {
        if constexpr (sizeof(Node) > sizeof(Header))
        {
            return sizeof(Node) - sizeof(Header) + 1;
        }
        else
        {
            return 1;
        }
    }

    static std::byte* as_bytes(void* ptr)
    {
        return static_cast<std::byte*>(ptr);
    }

    static const std::byte* as_bytes(const void* ptr)
    {
        return static_cast<const std::byte*>(ptr);
    }

    static Node* as_node(void* ptr)
    {
        return static_cast<Node*>(ptr);
    }

    static Header* as_header(void* ptr)
    {
        return static_cast<Header*>(ptr);
    }

    static const std::byte* end_of(const Node* node)
    {
        return as_bytes(node) + sizeof(Header) + node->size;
    }

    FindResult find_first(std::size_t required) const
    {
        Node* previous = nullptr;
        Node* current = m_head;

        while (current && current->size < required)
        {
            previous = current;
            current = current->next;
        }

        return {current, previous};
    }

    FindResult find_best(std::size_t required) const
    {
        Node* best = nullptr;
        Node* best_previous = nullptr;

        Node* previous = nullptr;
        Node* current = m_head;

        while (current)
        {
            if (current->size >= required &&
                (!best || current->size < best->size))
            {
                best = current;
                best_previous = previous;

                if (current->size == required)
                {
                    break;
                }
            }

            previous = current;
            current = current->next;
        }

        return {best, best_previous};
    }

    void merge_adjacent(Node* previous, Node* node)
    {
        if (node->next && end_of(node) == as_bytes(node->next))
        {
            node->size += sizeof(Header) + node->next->size;
            node->next = node->next->next;
        }

        if (previous && end_of(previous) == as_bytes(node))
        {
            previous->size += sizeof(Header) + node->size;
            previous->next = node->next;
        }
    }

    std::size_t m_capacity{};
    void* m_begin{};
    Node* m_head{};
    SearchPolicy m_policy{};
    FindFn m_find{};
};

namespace
{
constexpr std::size_t KiB = 1024;
constexpr std::size_t MiB = 1024 * KiB;

constexpr std::size_t kAllocCount = 1024;
constexpr std::size_t kFreeStep = 32;
constexpr std::size_t kPoolSize = 96 * MiB;

struct Scenario
{
    std::vector<std::size_t> first;
    std::vector<std::size_t> second;

    Scenario()
    {
        first.resize(kAllocCount);
        second.resize(kAllocCount / kFreeStep);

        std::mt19937 gen(123456);
        std::uniform_int_distribution<std::size_t> dist(1, 64);

        for (auto& value : first)
        {
            value = dist(gen) * KiB;
        }

        for (auto& value : second)
        {
            value = dist(gen) * KiB;
        }
    }
};

const Scenario g_scenario{};

void run_allocator_benchmark(benchmark::State& state, Allocator::SearchPolicy policy)
{
    std::vector<void*> pointers(kAllocCount, nullptr);

    for (auto _ : state) {
        Allocator allocator(kPoolSize, policy);

        for (std::size_t i = 0; i < kAllocCount; ++i) {
            pointers[i] = allocator.allocate(g_scenario.first[i]);


            benchmark::DoNotOptimize(pointers[i]);
            assert(pointers[i] != nullptr);
        }

        for (std::size_t i = 0; i < kAllocCount; i += kFreeStep) {
            allocator.deallocate(pointers[i]);
            pointers[i] = nullptr;
        }

        std::size_t refill_index = 0;
        for (std::size_t i = 0; i < kAllocCount; i += kFreeStep)
        {
            pointers[i] = allocator.allocate(g_scenario.second[refill_index++]);
            benchmark::DoNotOptimize(pointers[i]);
            assert(pointers[i] != nullptr);
        }

        for (void*& ptr : pointers)
        {
            allocator.deallocate(ptr);
            ptr = nullptr;
        }

        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(kAllocCount));
}

void BM_FirstFit(benchmark::State& state)
{
    run_allocator_benchmark(state, Allocator::SearchPolicy::first_fit);
}

void BM_BestFit(benchmark::State& state) {
    run_allocator_benchmark(state, Allocator::SearchPolicy::best_fit);
}

void self_test()
{
    Allocator allocator(1024, Allocator::SearchPolicy::first_fit);

    allocator.allocate(16);
    void* x = allocator.allocate(16);
    void* y = allocator.allocate(16);


    allocator.allocate(16);

    allocator.deallocate(y);
    allocator.deallocate(x);

    void* z = allocator.allocate(32);
    assert(z == x);
}
} // namespace

BENCHMARK(BM_FirstFit)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_BestFit)->Unit(benchmark::kMicrosecond);

int main(int argc, char** argv)
{
    self_test();

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }

    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
}