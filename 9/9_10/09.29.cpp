#include "allocator.hpp"

#include <cassert>
#include <iostream>

int main()
{
    StackAllocator allocator(256);

    void* a = allocator.allocate(4, 4);
    void* b = allocator.allocate(8, 8);
    void* x = allocator.allocate(16, 8);
    void* y = allocator.allocate(24, 16);


    
    allocator.deallocate(y);
    allocator.deallocate(x);

    void* z = allocator.allocate(16, 8);

    std::cout << "test 09.29 :: StackAllocator\n";
    std::cout << "a = " << a << '\n';
    std::cout << "b = " << b << '\n';
    std::cout << "x = " << x << '\n';
    std::cout << "y = " << y << '\n';
    std::cout << "z = " << z << '\n';
    std::cout << "used = " << allocator.used() << " bytes\n\n";

    assert(x != nullptr);
    assert(y != nullptr);
    assert(z == x);

    demo_polymorphism();
}
