#include <array>
#include <bit>
#include <boost/noncopyable.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <new>
#include <utility>

class Entity : private boost::noncopyable
{
private:
    class Implementation;

public:
    Entity();
    Entity(Entity&& other) noexcept;
    ~Entity();

    auto& operator=(Entity&& other) noexcept;

    void test() const;

    auto get() noexcept -> Implementation*;
    auto get() const noexcept -> const Implementation*;

private:
    alignas(std::max_align_t) std::array<std::byte, 16> m_storage{};
};

class Entity::Implementation final
{
public:
    std::uint64_t id = 0x1234'5678'9ABC'DEF0ULL;
    std::uint32_t generation = 23;
    bool active = true;

    Implementation() = default;
    Implementation(Implementation&&) noexcept = default;
    auto operator=(Implementation&&) noexcept -> Implementation& = default;
    ~Implementation() = default;

    void test() const
    {
        std::cout << "id = " << id << ", generation = " << generation << ", active = " << std::boolalpha << active << '\n';
    }
};

Entity::Entity()
{
    static_assert(sizeof(Implementation) <= 16, "Implementation is too large for Entity storage");

    static_assert(alignof(Implementation) <= alignof(std::max_align_t), "Implementation alignment is too strict for Entity storage");

    ::new (static_cast<void*>(m_storage.data())) Implementation();
}

Entity::Entity(Entity&& other) noexcept
{
    static_assert(sizeof(Implementation) <= 16, "Implementation is too large for Entity storage");

    static_assert(alignof(Implementation) <= alignof(std::max_align_t), "Implementation alignment is too strict for Entity storage");

    ::new (static_cast<void*>(m_storage.data())) Implementation(std::move(*other.get()));
}

Entity::~Entity()
{
    std::destroy_at(get());
}

auto& Entity::operator=(Entity&& other) noexcept
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

auto Entity::get() noexcept -> Implementation* {
    auto* ptr = std::bit_cast<Implementation*>(m_storage.data());
    return std::launder(ptr);
}

auto Entity::get() const noexcept -> const Implementation* {
    auto* ptr = std::bit_cast<const Implementation*>(m_storage.data());
    return std::launder(ptr);
}

int main()
{
    Entity a;
    a.test();

    Entity b(std::move(a));
    b.test();

    Entity c;
    c = std::move(b);
    c.test();
}