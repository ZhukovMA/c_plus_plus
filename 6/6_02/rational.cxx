export module rational;   
import std;               
export namespace edu {   

class Rational;
std::istream& operator>>(std::istream& stream, Rational& rational);
std::ostream& operator<<(std::ostream& stream, const Rational& rational);

class Rational {
public:
    Rational(int num = 0, int den = 1) : m_num(num), m_den(den) {
        reduce();
    }

    explicit operator double() const {
        return static_cast<double>(m_num) / m_den;
    }

    Rational& operator+=(const Rational& other);
    Rational& operator*=(const Rational& other);

    Rational& operator-=(const Rational& other) {
        return *this += Rational(-other.m_num, other.m_den);
    }

    Rational& operator/=(const Rational& other) {
        return *this *= Rational(other.m_den, other.m_num);
    }

    Rational operator++(int) {
        Rational old = *this;
        ++(*this);
        return old;
    }

    Rational operator--(int) {
        Rational old = *this;
        --(*this);
        return old;
    }

    Rational& operator++() {
        return *this += 1;
    }

    Rational& operator--() {
        return *this -= 1;
    }

    friend Rational operator+(Rational lhs, const Rational& rhs) {
        return lhs += rhs;
    }

    friend Rational operator-(Rational lhs, const Rational& rhs) {
        return lhs -= rhs;
    }

    friend Rational operator*(Rational lhs, const Rational& rhs) {
        return lhs *= rhs;
    }

    friend Rational operator/(Rational lhs, const Rational& rhs) {
        return lhs /= rhs;
    }

    friend std::strong_ordering operator<=>(const Rational& lhs, const Rational& rhs) {
        const long long left = 1LL * lhs.m_num * rhs.m_den;
        const long long right = 1LL * rhs.m_num * lhs.m_den;

        if (left < right) {
            return std::strong_ordering::less;
        }
        if (left > right) {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

    friend bool operator==(const Rational& lhs, const Rational& rhs) = default;

    friend std::istream& operator>>(std::istream& stream, Rational& rational);
    friend std::ostream& operator<<(std::ostream& stream, const Rational& rational);

private:
    void reduce();

    int m_num = 0;
    int m_den = 1;
};

inline bool equal(double x, double y, double epsilon = 1e-6) {
    return std::abs(x - y) < epsilon;
}

} // namespace edu