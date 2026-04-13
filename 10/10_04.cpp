#include <iostream>
#include <stdexcept>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/operation.hpp>

class FibonacciMatrixCalculator {
public:
    using ull = unsigned long long;
    using Matrix2x2 = boost::numeric::ublas::bounded_matrix<ull, 2, 2>;

    static ull fibonacci(ull n) {
        if (n == 0) {
            return 0;
        }

        if (n > 93) {
            throw std::overflow_error( "Overflow: unsigned long long can store Fibonacci numbers only up to F(93).");
        }

        const Matrix2x2 base = makeBaseMatrix();
        const Matrix2x2 result = fastPower(base, n);

        return result(0, 1);
    }

private:
    static Matrix2x2 makeBaseMatrix() {
        Matrix2x2 m{};
        m(0, 0) = 1; m(0, 1) = 1;
        m(1, 0) = 1; m(1, 1) = 0;
        return m;
    }

    static Matrix2x2 makeIdentityMatrix() {
        Matrix2x2 m{};
        m(0, 0) = 1; m(0, 1) = 0;
        m(1, 0) = 0; m(1, 1) = 1;
        return m;
    }

    static Matrix2x2 multiply(const Matrix2x2& a, const Matrix2x2& b) {
        Matrix2x2 result{};
        noalias(result) = prod(a, b);
        return result;
    }

    static Matrix2x2 fastPower(Matrix2x2 base, ull exponent) {
        Matrix2x2 result = makeIdentityMatrix();

        while (exponent > 0) {
            if (exponent & 1ULL) {
                result = multiply(result, base);
            }

            base = multiply(base, base);
            exponent >>= 1ULL;
        }

        return result;
    }
};

int main() {
    using ull = FibonacciMatrixCalculator::ull;

    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    ull n{};
    std::cin >> n;

    try {
        std::cout << FibonacciMatrixCalculator::fibonacci(n) << '\n';
    } catch (const std::overflow_error& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}