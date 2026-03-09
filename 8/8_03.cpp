#include <iostream>
#include <stdexcept>
#include <limits>

static_assert(sizeof(int) == 4, "int must be 4 bytes");
static_assert(sizeof(float) == 4, "float must be 4 bytes");
static_assert(sizeof(unsigned int) == 4, "unsigned int must be 4 bytes");

int ilog2_int(int x)
{
    if (x <= 0)
    {
        throw std::domain_error("ilog2_int: x must be positive");
    }

    unsigned int u = static_cast<unsigned int>(x);

    int result = -1;
    while (u != 0u)
    {
        u >>= 1;
        ++result;
    }

    return result;
}

int ilog2_float(float x)
{
    union FloatBits
    {
        float f;
        unsigned int u;
    };

    FloatBits value;
    value.f = x;

    unsigned int sign = value.u >> 31;
    unsigned int exponent = (value.u >> 23) & 0xFFu;
    unsigned int fraction = value.u & 0x7FFFFFu;

    if (sign != 0u)
    {
        throw std::domain_error("ilog2_float: x must be positive");
    }

    if (exponent == 0u && fraction == 0u)
    {
        throw std::domain_error("ilog2_float: log2(0) is undefined");
    }

    if (exponent == 0xFFu)
    {
        if (fraction == 0u)
        {
            throw std::domain_error("ilog2_float: log2(inf) is not finite");
        }
        else
        {
            throw std::domain_error("ilog2_float: log2(NaN) is undefined");
        }
    }

    if (exponent != 0u)
    {
        return static_cast<int>(exponent) - 127;
    }

    int msb = -1;
    unsigned int m = fraction;

    while (m != 0u)
    {
        m >>= 1;
        ++msb;
    }

    return msb - 149;
}

int main()
{
    std::cout << "ilog2_int(1)    = " << ilog2_int(1) << '\n';
    std::cout << "ilog2_int(37)   = " << ilog2_int(37) << '\n';
    std::cout << "ilog2_int(1024) = " << ilog2_int(1024) << '\n';
    std::cout << "ilog2_float(1.0f)      = " << ilog2_float(1.0f) << '\n';
    std::cout << "ilog2_float(37.625f)   = " << ilog2_float(37.625f) << '\n';
    std::cout << "ilog2_float(0.75f)     = " << ilog2_float(0.75f) << '\n';
    std::cout << "ilog2_float(0.5f)      = " << ilog2_float(0.5f) << '\n';
    std::cout << "ilog2_float(denormMin) = " << ilog2_float(std::numeric_limits<float>::denorm_min()) << '\n';
    return 0;
}