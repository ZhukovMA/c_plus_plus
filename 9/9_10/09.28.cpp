#include "allocator.hpp"

#include <cassert>
#include <iostream>

int main()
{
    LinearAllocator allocator(128);

    void* a = allocator.allocate(1, 1);
    void* b = allocator.allocate(2, 2);
    void* c = allocator.allocate(4, 4);
    void* d = allocator.allocate(8, 8);

    std::cout << "test of the 09.28 :: LinearAllocator\n";
    std::cout << "a = " << a << '\n';
    std::cout << "b = " << b << '\n';
    std::cout << "c = " << c << '\n';
    std::cout << "d = " << d << '\n';
    std::cout << "used = " << allocator.used() << " bytes\n\n";

    assert(a != nullptr);
    
    assert(b != nullptr);
    assert(c != nullptr);
    assert(d != nullptr);


    assert(allocator.allocate(512) == nullptr);

    demo_polymorphism();
}
