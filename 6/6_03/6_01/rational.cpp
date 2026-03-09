#include "rational.hpp"

#include <ios>
#include <istream>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <array>
#include <tuple>
#include <utility>
#include <limits>

Rational &Rational::operator+=(const Rational &other) {
    const int lcm = std::lcm(m_den, other.m_den);
    m_num = m_num * (lcm / m_den) + other.m_num * (lcm / other.m_den);
    m_den = lcm;
    reduce();
    return *this;
}

Rational &Rational::operator*=(const Rational &other) {
    m_num *= other.m_num;
    m_den *= other.m_den;
    reduce();
    return *this;
}

std::istream &operator>>(std::istream &stream, Rational &rational) {
    int num = 0;
    int den = 1;
    char slash = '\0';

    if (!(stream >> num >> slash >> den) || slash != '/') {
        stream.setstate(std::ios::failbit);
        return stream;
    }

    try {
        rational = Rational(num, den);
    } catch (const std::invalid_argument &) {
        stream.setstate(std::ios::failbit);
    }

    return stream;
}

std::ostream &operator<<(std::ostream &stream, const Rational &rational) {
    return stream << rational.m_num << '/' << rational.m_den;
}

void Rational::reduce() {
    if (m_den == 0) {
        throw std::invalid_argument("Denominator must not be zero");
    }

    if (m_den < 0) {
        m_num = -m_num;
        m_den = -m_den;
    }

    if (m_num == 0) {
        m_den = 1;
        return;
    }

    const int gcd = std::gcd(m_num, m_den);
    m_num /= gcd;
    m_den /= gcd;
}