#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <memory>
#include <new>
#include <utility>

class Entity
{
private:
    class Implementation;
public:
    Entity();
    Entity(Entity&& other) noexcept;
    ~Entity();
    Entity(const Entity&) = delete;
    auto operator=(const Entity&) -> Entity& = delete;
    auto operator=(Entity&& other) noexcept -> Entity&;
    void test() const;
    auto get() noexcept -> Implementation*;
    auto get() const noexcept -> const Implementation*;

private:
    static constexpr std::size_t storage_size  = 16;
    static constexpr std::size_t storage_align = alignof(std::max_align_t);
    alignas(std::max_align_t) std::array<std::byte, storage_size> m_storage{};
};