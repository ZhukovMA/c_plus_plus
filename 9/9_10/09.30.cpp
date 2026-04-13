#include "allocator.hpp"

#include <cassert>
#include <iostream>

int main()
{
    PoolAllocator allocator(32, 4);

    void* a = allocator.allocate(24);
    void* b = allocator.allocate(24);
    void* x = allocator.allocate(24);
    void* y = allocator.allocate(24);
    void* extra = allocator.allocate(24);

    allocator.deallocate(x);
    allocator.deallocate(y);

    void* z = allocator.allocate(24);

    std::cout << "test 09.30 :: PoolAllocator\n";
    std::cout << "a = " << a << '\n';
    std::cout << "b = " << b << '\n';
    std::cout << "x = " << x << '\n';
    std::cout << "y = " << y << '\n';
    std::cout << "extra = " << extra << '\n';
    std::cout << "z = " << z << '\n';
    std::cout << "chunks = " << allocator.chunk_count() << "\n\n";

    assert(x != nullptr);
    assert(y != nullptr);
    assert(extra != nullptr);
    assert(z == y);

    demo_polymorphism();
}
