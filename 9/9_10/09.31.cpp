#include "allocator.hpp"

#include <cassert>
#include <iostream>

int main()
{
    FreeListAllocator allocator(256);

    void* a = allocator.allocate(16);
    void* x = allocator.allocate(16);
    void* y = allocator.allocate(16);
    void* b = allocator.allocate(16);

    allocator.deallocate(y);
    allocator.deallocate(x);

    void* z = allocator.allocate(32);

    std::cout << "test 09.31 :: FreeListAllocator\n";
    std::cout << "a = " << a << '\n';
    std::cout << "x = " << x << '\n';
    std::cout << "y = " << y << '\n';
    std::cout << "b = " << b << '\n';
    std::cout << "z = " << z << '\n';
    std::cout << '\n';

    assert(x != nullptr);
    assert(y != nullptr);
    assert(z == x);

    demo_polymorphism();
}
