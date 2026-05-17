#include "06.09.hpp"

#include <cstdint>
#include <iostream>
#include <utility>

class Entity::Implementation final
{
public:
    std::uint64_t id = 0x1234'5678'9ABC'DEF0ULL;
    std::uint32_t generation = 1;
    bool alive = true;
    Implementation() = default;
    Implementation(Implementation&&) noexcept = default;
    auto operator=(Implementation&&) noexcept -> Implementation& = default;
    ~Implementation() = default;

    void test() const
    {
        std::cout  << "id = " << id << ", generation = " << generation
            << ", alive = " << std::boolalpha << alive
            << '\n';
    }
};

Entity::Entity()
{
    static_assert(sizeof(Implementation) <= storage_size, "Implementation does not fit into Entity inline storage");
    static_assert(alignof(Implementation) <= storage_align, "Implementation requires stronger alignment than Entity provides");
    
    ::new (static_cast<void*>(m_storage.data())) Implementation();
}

Entity::Entity(Entity&& other) noexcept
{
    static_assert(sizeof(Implementation) <= storage_size, "Implementation does not fit into Entity inline storage");
    static_assert(alignof(Implementation) <= storage_align, "Implementation requires stronger alignment than Entity provides");
    ::new (static_cast<void*>(m_storage.data()))
        Implementation(std::move(*other.get()));
}

Entity::~Entity()
{
    std::destroy_at(get());
}

auto Entity::operator=(Entity&& other) noexcept -> Entity&
{
    if (this != &other) {
        *get() = std::move(*other.get());
    }

    return *this;
}

void Entity::test() const
{
    get()->test();
}

auto Entity::get() noexcept -> Implementation*
{
    auto* raw = std::bit_cast<Implementation*>(m_storage.data());
    return std::launder(raw);
}

auto Entity::get() const noexcept -> const Implementation*
{
    auto* raw = std::bit_cast<const Implementation*>(m_storage.data());
    return std::launder(raw);
}