
/*

Exception возникает, когда создается дробь с нулевым знаменателем, потому что это нарушает инвариант класса. 
std::bad_alloc означает, что не удалось выделить динамическую память; std::vector::reserve может пробросить ошибку аллокатора, 
обычно это именно std::bad_alloc. std::bad_variant_access появляется, когда из std::variant пытаются достать не тот тип, 
который хранится сейчас. std::bad_optional_access появляется, когда вызывают value() у пустого std::optional.

*/


#include <cassert>
#include <cmath>
#include <compare>
#include <exception>
#include <iostream>
#include <istream>
#include <limits>
#include <numeric>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

struct empty_base {};

template <typename T, typename B = empty_base>
struct addable : B {
    friend T operator+(T lhs, T const& rhs) {
        lhs += rhs;
        return lhs;
    }
};

template <typename T, typename B = empty_base>
struct subtractable : B {
    friend T operator-(T lhs, T const& rhs) {
        lhs -= rhs;
        return lhs;
    }
};

template <typename T, typename B = empty_base>
struct multipliable : B {
    friend T operator*(T lhs, T const& rhs) {
        lhs *= rhs;
        return lhs;
    }
};

template <typename T, typename B = empty_base>
struct dividable : B {
    friend T operator/(T lhs, T const& rhs) {
        lhs /= rhs;
        return lhs;
    }
};

template <typename T, typename B = empty_base>
struct incrementable : B {
    friend T operator++(T& value, int) {
        T old(value);
        ++value;
        return old;
    }
};

template <typename T, typename B = empty_base>
struct decrementable : B {
    friend T operator--(T& value, int) {
        T old(value);
        --value;
        return old;
    }
};

template <typename T>
using rational_operators =
    decrementable<T,
    incrementable<T,
    dividable<T,
    multipliable<T,
    subtractable<T,
    addable<T>>>>>>;

class Exception : public std::exception {
  public:
    explicit Exception(std::string message) : m_message(std::move(message)) {}

    const char* what() const noexcept override {
        return m_message.c_str();
    }

  private:
    std::string m_message;
};

template <typename T>
class Rational : public rational_operators<Rational<T>> {
  public:
    Rational(T num = T{0}, T den = T{1}) : m_num(num), m_den(den) {
        if (m_den == T{0}) {
            throw Exception("zero denominator");
        }
        reduce();
    }

    explicit operator double() const {
        return 1.0 * m_num / m_den;
    }

    auto& operator+=(Rational const& other) {
        auto lcm = std::lcm(m_den, other.m_den);
        m_num = m_num * (lcm / m_den) + other.m_num * (lcm / other.m_den);
        m_den = lcm;
        reduce();
        return *this;
    }

    auto& operator-=(Rational const& other) {
        return *this += Rational(-other.m_num, other.m_den);
    }

    auto& operator*=(Rational const& other) {
        m_num *= other.m_num;
        m_den *= other.m_den;
        reduce();
        return *this;
    }

    auto& operator/=(Rational const& other) {
        if (other.m_num == T{0}) {
            throw Exception("division by zero rational");
        }
        return *this *= Rational(other.m_den, other.m_num);
    }

    auto& operator++() {
        *this += T{1};
        return *this;
    }

    auto& operator--() {
        *this -= T{1};
        return *this;
    }

    friend std::strong_ordering operator<=>(Rational const& lhs, Rational const& rhs) {
        auto L = lhs.m_num * rhs.m_den;
        auto R = rhs.m_num * lhs.m_den;

        if (L < R)
            return std::strong_ordering::less;
        if (L > R)
            return std::strong_ordering::greater;
        return std::strong_ordering::equal;
    }

    friend bool operator==(Rational const& lhs, Rational const& rhs) {
        return lhs.m_num == rhs.m_num && lhs.m_den == rhs.m_den;
    }

    friend auto& operator>>(std::istream& stream, Rational& rational) {
        T num{};
        T den{};
        char slash = 0;

        stream >> num >> slash >> den;

        if (stream && slash != '/') {
            stream.setstate(std::ios::failbit);
        }

        if (stream) {
            rational = Rational(num, den);
        }

        return stream;
    }

    friend auto& operator<<(std::ostream& stream, Rational const& rational) {
        return stream << rational.m_num << '/' << rational.m_den;
    }

  private:
    void reduce() {
        if (m_den < T{0}) {
            m_num = -m_num;
            m_den = -m_den;
        }

        auto gcd = std::gcd(m_num, m_den);
        m_num /= gcd;
        m_den /= gcd;
    }

    T m_num = T{0};
    T m_den = T{1};
};

template <typename T>
Rational(T) -> Rational<T>;

template <typename T>
Rational(T, T) -> Rational<T>;

auto equal(double x, double y, double epsilon = 1e-6) {
    return std::abs(x - y) < epsilon;
}

int main() {
    try {
        Rational x = 1, y(2, 1);

        std::vector<int> vector_2(5);
        std::vector<int> vector_3 = {1, 2, 3, 4, 5};

        assert(equal(static_cast<double>(x), 1));

        assert((x += y) == Rational(+3, 1));
        assert((x -= y) == Rational(+1, 1));
        assert((x *= y) == Rational(+2, 1));
        assert((x /= y) == Rational(+1, 1));

        assert((x++) == Rational(+1, 1));
        assert((x--) == Rational(+2, 1));
        assert((++y) == Rational(+3, 1));
        assert((--y) == Rational(+2, 1));

        [[maybe_unused]] auto z = 0;

        assert((x + y) == Rational(+3, 1));
        assert((x - y) == Rational(-1, 1));
        assert((x * y) == Rational(+2, 1));
        assert((x / y) == Rational(+1, 2));

        assert((x += 1) == Rational(+2, 1));
        assert((x + 1) == Rational(+3, 1));
        assert((1 + y) == Rational(+3, 1));
        assert((1 + 1) == Rational(+2, 1));

        assert((x < y) == 0);
        assert((x > y) == 0);
        assert((x <= y) == 1);
        assert((x >= y) == 1);
        assert((x == y) == 1);
        assert((x != y) == 0);

        std::stringstream stream_1("1/2");
        std::stringstream stream_2;

        stream_1 >> x;
        stream_2 << x;

        assert(stream_2.str() == stream_1.str());

        try {
            [[maybe_unused]] Rational invalid(1, 0);
        } catch (std::exception const& e) {
            std::cerr << "Exception: " << e.what() << '\n';
        }

        try {
            std::vector<int> huge;
            huge.reserve(huge.max_size()); 
            throw std::bad_alloc{};        
        } catch (std::bad_alloc const& e) {
            std::cerr << "std::bad_alloc: " << e.what() << '\n';
        }

        try {
            std::variant<int, std::string> value = 42;
            [[maybe_unused]] auto text = std::get<std::string>(value);
        } catch (std::bad_variant_access const& e) {
            std::cerr << "std::bad_variant_access: " << e.what() << '\n';
        }

        try {
            std::optional<int> value;
            [[maybe_unused]] auto result = value.value();
        } catch (std::bad_optional_access const& e) {
            std::cerr << "std::bad_optional_access: " << e.what() << '\n';
        }

        try {
            const auto max = vector_2.max_size();
            if (max == std::numeric_limits<std::size_t>::max()) {
                throw std::runtime_error("cannot demonstrate std::length_error on this platform");
            }
            vector_2.reserve(max + 1);
        } catch (std::length_error const& e) {
            std::cerr << "std::length_error: " << e.what() << '\n';
        }

        try {
            [[maybe_unused]] auto value = vector_3.at(vector_3.size());
        } catch (std::out_of_range const& e) {
            std::cerr << "std::out_of_range: " << e.what() << '\n';
        }
    } catch (std::exception const& e) {
        std::cerr << "Unhandled std::exception: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Unhandled unknown exception\n";
        return 1;
    }

    return 0;
}