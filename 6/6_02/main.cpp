import std;
import rational;

int main() {
    using edu::Rational;
    using edu::equal;

    Rational x = 1;
    Rational y(2, 1);

    assert(equal(static_cast<double>(x), 1.0));

    assert((x += y) == Rational(3, 1));
    assert((x -= y) == Rational(1, 1));
    assert((x *= y) == Rational(2, 1));
    assert((x /= y) == Rational(1, 1));

    assert((x++) == Rational(1, 1));
    assert((x--) == Rational(2, 1));
    assert((++y) == Rational(3, 1));
    assert((--y) == Rational(2, 1));

    assert((x + y) == Rational(3, 1));
    assert((x - y) == Rational(-1, 1));
    assert((x * y) == Rational(2, 1));
    assert((x / y) == Rational(1, 2));

    assert((x += 1) == Rational(2, 1));
    assert((x + 1) == Rational(3, 1));
    assert((1 + y) == Rational(3, 1));
    assert((1 + 1) == Rational(2, 1));

    assert((x < y) == false);
    assert((x > y) == false);
    assert((x <= y) == true);
    assert((x >= y) == true);
    assert((x == y) == true);
    assert((x != y) == false);

    std::stringstream input("1/2");
    std::stringstream output;

    input >> x;
    output << x;

    assert(output.str() == "1/2");

    const Rational a(2, 4);
    const Rational b(-6, -8);
    const Rational c(7, -21);

    std::cout << "2/4 = " << a << '\n';
    std::cout << "-6/-8 = " << b << '\n';
    std::cout << "7/-21 = " << c << '\n';
    std::cout << "1/2 + 3/4 = " << Rational(1, 2) + Rational(3, 4) << '\n';
    std::cout << "3/5 * 10/9 = " << Rational(3, 5) * Rational(10, 9) << '\n';

    try {
        [[maybe_unused]] Rational bad(1, 0);
        assert(false);
    } catch (const std::invalid_argument&) {
        std::cout << "zero division.\n";
    }

    try {
        [[maybe_unused]] Rational bad_division = Rational(1, 2) / Rational(0, 1);
        assert(false);
    } catch (const std::invalid_argument&) {
        std::cout << "devision incorrect.\n";
    }

}