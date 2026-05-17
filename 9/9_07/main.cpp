#include "06.09.hpp"

#include <utility>

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