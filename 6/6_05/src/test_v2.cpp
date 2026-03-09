#include <iostream>
#include <boost/config.hpp>

extern "C" BOOST_SYMBOL_EXPORT void test()
{
    std::cout << "This is the implementation of test() from version 2" << '\n';
}