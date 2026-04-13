#ifndef UNIQUE_ALLOCATOR_HPP
#define UNIQUE_ALLOCATOR_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <string_view>
#include <utility>
#include <vector>

class Allocator
{
public:
    Allocator() = default;
    Allocator(const Allocator&) = delete;
    auto operator=(const Allocator&) -> Allocator& = delete;
    Allocator(Allocator&&) = delete;
    auto operator=(Allocator&&) -> Allocator& = delete;

    virtual ~Allocator() = default;

    virtual auto allocate(std::size_t bytes,
                          std::size_t alignment = alignof(std::max_align_t)) -> void* = 0;

    virtual void deallocate(void* ptr) = 0;

protected:
    template <class T = std::byte>
    static auto get(void* ptr) noexcept -> T*
    {
        return static_cast<T*>(ptr);
    }

    template <class T = std::byte>
    static auto get(const void* ptr) noexcept -> const T*
    {
        return static_cast<const T*>(ptr);
    }

    static constexpr std::size_t default_alignment = alignof(std::max_align_t);
};

class LinearAllocator final : public Allocator
{
public:
    explicit LinearAllocator(std::size_t capacity)
        : m_capacity(capacity),
          m_storage(get<std::byte>(::operator new(capacity, std::align_val_t(default_alignment))))
    {
    }

    ~LinearAllocator() override
    {
        ::operator delete(m_storage, std::align_val_t(default_alignment));
    }

    auto allocate(std::size_t bytes, std::size_t alignment = default_alignment) -> void* override
    {
        if (bytes == 0 || alignment == 0)
        {
            return nullptr;
        }

        void* current = m_storage + m_offset;
        std::size_t free_space = m_capacity - m_offset;

        if (std::align(alignment, bytes, current, free_space) == nullptr)
        {
            return nullptr;
        }

        m_offset = static_cast<std::size_t>(get<std::byte>(current) - m_storage) + bytes;
        return current;
    }

    void deallocate(void*) override
    {
    }

    void reset() noexcept
    {
        m_offset = 0;
    }

    auto used() const noexcept -> std::size_t
    {
        return m_offset;
    }

private:
    std::size_t m_capacity{};
    std::size_t m_offset{};
    std::byte* m_storage{};
};

class StackAllocator final : public Allocator
{
public:
    explicit StackAllocator(std::size_t capacity)
        : m_capacity(capacity),
          m_storage(get<std::byte>(::operator new(capacity, std::align_val_t(default_alignment))))
    {
    }

    ~StackAllocator() override
    {
        ::operator delete(m_storage, std::align_val_t(default_alignment));
    }

    auto allocate(std::size_t bytes, std::size_t alignment = default_alignment) -> void* override
    {
        if (bytes == 0 || alignment == 0)
        {
            return nullptr;
        }

        alignment = std::max(alignment, alignof(Header));

        void* current = m_storage + m_offset + sizeof(Header);
        std::size_t free_space = m_capacity - m_offset;

        if (free_space <= sizeof(Header))
        {
            return nullptr;
        }

        free_space -= sizeof(Header);

        if (std::align(alignment, bytes, current, free_space) == nullptr)
        {
            return nullptr;
        }

        auto* payload = get<std::byte>(current);
        auto* header = get<Header>(payload - sizeof(Header));
        header->previous_offset = m_offset;

        m_offset = static_cast<std::size_t>(payload - m_storage) + bytes;
        return payload;
    }

    void deallocate(void* ptr) override
    {
        if (ptr == nullptr)
        {
            return;
        }

        auto* payload = get<std::byte>(ptr);
        auto* header = get<Header>(payload - sizeof(Header));
        m_offset = header->previous_offset;
    }

    auto used() const noexcept -> std::size_t
    {
        return m_offset;
    }

private:
    struct alignas(std::max_align_t) Header
    {
        std::size_t previous_offset{};
    };

    std::size_t m_capacity{};
    std::size_t m_offset{};
    std::byte* m_storage{};
};

class PoolAllocator final : public Allocator
{
public:
    PoolAllocator(std::size_t block_size, std::size_t blocks_per_chunk)
        : m_stride(round_up(std::max(block_size, sizeof(Node)), default_alignment)),
          m_blocks_per_chunk(std::max<std::size_t>(1, blocks_per_chunk)),
          m_chunk_size(m_stride * m_blocks_per_chunk)
    {
        refill();
    }

    ~PoolAllocator() override
    {
        for (void* chunk : m_chunks)
        {
            ::operator delete(chunk, std::align_val_t(default_alignment));
        }
    }

    auto allocate(std::size_t bytes, std::size_t alignment = default_alignment) -> void* override
    {
        if (bytes == 0 || bytes > m_stride || alignment > default_alignment)
        {
            return nullptr;
        }

        if (m_free == nullptr)
        {
            refill();
        }

        Node* node = m_free;
        m_free = m_free->next;
        return node;
    }

    void deallocate(void* ptr) override
    {
        if (ptr == nullptr)
        {
            return;
        }

        auto* node = get<Node>(ptr);
        node->next = m_free;
        m_free = node;
    }

    auto chunk_count() const noexcept -> std::size_t
    {
        return m_chunks.size();
    }

private:
    struct Node
    {
        Node* next{};
    };

    static auto round_up(std::size_t value, std::size_t alignment) -> std::size_t
    {
        const std::size_t mask = alignment - 1;
        return (value + mask) & ~mask;
    }

    void refill()
    {
        auto* chunk = get<std::byte>(::operator new(m_chunk_size, std::align_val_t(default_alignment)));
        m_chunks.push_back(chunk);

        for (std::size_t index = 0; index < m_blocks_per_chunk; ++index)
        {
            auto* node = get<Node>(chunk + index * m_stride);
            node->next = m_free;
            m_free = node;
        }
    }

    std::size_t m_stride{};
    std::size_t m_blocks_per_chunk{};
    std::size_t m_chunk_size{};
    Node* m_free{};
    std::vector<void*> m_chunks;
};

class FreeListAllocator final : public Allocator
{
public:
    explicit FreeListAllocator(std::size_t capacity)
        : m_capacity(std::max(capacity, sizeof(Node))),
          m_storage(get<std::byte>(::operator new(m_capacity, std::align_val_t(default_alignment)))),
          m_head(get<Node>(m_storage))
    {
        m_head->size = m_capacity;
        m_head->next = nullptr;
    }

    ~FreeListAllocator() override
    {
        ::operator delete(m_storage, std::align_val_t(default_alignment));
    }

    auto allocate(std::size_t bytes, std::size_t alignment = default_alignment) -> void* override
    {
        if (bytes == 0 || alignment == 0)
        {
            return nullptr;
        }

        alignment = std::max(alignment, alignof(Header));

        Candidate best{};

        for (Node* previous = nullptr, * current = m_head; current != nullptr; previous = current, current = current->next)
        {
            void* candidate = get<std::byte>(current) + sizeof(Header);
            std::size_t free_space = current->size;

            if (free_space <= sizeof(Header))
            {
                continue;
            }

            free_space -= sizeof(Header);

            if (std::align(alignment, bytes, candidate, free_space) == nullptr)
            {
                continue;
            }

            auto* payload = get<std::byte>(candidate);
            std::size_t payload_offset = static_cast<std::size_t>(payload - get<std::byte>(current));
            std::size_t consumed = payload_offset + bytes;

            if (!best.found || current->size < best.block->size)
            {
                best.found = true;
                best.block = current;
                best.previous = previous;
                best.payload_offset = payload_offset;
                best.consumed = consumed;
            }
        }

        if (!best.found)
        {
            return nullptr;
        }

        auto* block_begin = get<std::byte>(best.block);
        std::size_t actual_consumed = best.consumed;
        std::size_t remainder = best.block->size - actual_consumed;

        if (remainder >= sizeof(Node))
        {
            auto* tail = get<Node>(block_begin + actual_consumed);
            tail->size = remainder;
            tail->next = best.block->next;

            if (best.previous == nullptr)
            {
                m_head = tail;
            }
            else
            {
                best.previous->next = tail;
            }
        }
        else
        {
            actual_consumed = best.block->size;

            if (best.previous == nullptr)
            {
                m_head = best.block->next;
            }
            else
            {
                best.previous->next = best.block->next;
            }
        }

        auto* payload = block_begin + best.payload_offset;
        auto* header = get<Header>(payload - sizeof(Header));
        header->block_size = actual_consumed;
        header->payload_offset = best.payload_offset;

        return payload;
    }

    void deallocate(void* ptr) override
    {
        if (ptr == nullptr)
        {
            return;
        }

        auto* payload = get<std::byte>(ptr);
        auto* header = get<Header>(payload - sizeof(Header));
        auto* block_begin = payload - header->payload_offset;
        auto* node = get<Node>(block_begin);
        node->size = header->block_size;
        node->next = nullptr;

        Node* previous = nullptr;
        Node* current = m_head;

        while (current != nullptr && address(current) < address(node))
        {
            previous = current;
            current = current->next;
        }

        node->next = current;

        if (previous == nullptr)
        {
            m_head = node;
        }
        else
        {
            previous->next = node;
        }

        merge(previous, node);
    }

private:
    struct Node
    {
        std::size_t size{};
        Node* next{};
    };

    struct alignas(std::max_align_t) Header
    {
        std::size_t block_size{};
        std::size_t payload_offset{};
    };

    struct Candidate
    {
        bool found{};
        Node* block{};
        Node* previous{};
        std::size_t payload_offset{};
        std::size_t consumed{};
    };

    static auto address(const void* ptr) noexcept -> std::uintptr_t
    {
        return reinterpret_cast<std::uintptr_t>(ptr);
    }

    static auto adjacent(const Node* left, const Node* right) noexcept -> bool
    {
        return get<const std::byte>(left) + left->size == get<const std::byte>(right);
    }

    static void absorb_next(Node* node) noexcept
    {
        if (node != nullptr && node->next != nullptr && adjacent(node, node->next))
        {
            node->size += node->next->size;
            node->next = node->next->next;
        }
    }

    static void merge(Node* previous, Node* node) noexcept
    {
        absorb_next(node);

        if (previous != nullptr && adjacent(previous, node))
        {
            previous->size += node->size;
            previous->next = node->next;
            absorb_next(previous);
        }
    }

    std::size_t m_capacity{};
    std::byte* m_storage{};
    Node* m_head{};
};

inline void demo_polymorphism()
{
    std::vector<std::unique_ptr<Allocator>> allocators;
    allocators.emplace_back(std::make_unique<LinearAllocator>(128));
    allocators.emplace_back(std::make_unique<StackAllocator>(128));
    allocators.emplace_back(std::make_unique<PoolAllocator>(32, 4));
    allocators.emplace_back(std::make_unique<FreeListAllocator>(128));

    std::cout << "Polymorphic call demo:\n";

    for (const auto& allocator : allocators)
    {
        void* ptr = allocator->allocate(24, alignof(std::max_align_t));
        std::cout << "  allocate(24) -> " << ptr << '\n';

        if (ptr != nullptr)
        {
            std::memset(ptr, 0x2A, 24);
        }

        allocator->deallocate(ptr);
    }

    std::cout << '\n';
}

#endif
