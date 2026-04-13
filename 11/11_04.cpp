#include <cmath>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

using Roots = std::variant<double, std::pair<double, double>, std::monostate>;

class Visitor {
public:
    void operator()(double root) const {
        std::cout << root << '\n';
    }

    void operator()(const std::pair<double, double>& roots) const {
        std::cout << roots.first << '\n' << roots.second << '\n';
    }

    void operator()(std::monostate) const {
    }
};

std::optional<Roots> solve(double a, double b, double c) {
    const double epsilon = 1e-9;

    auto is_zero = [epsilon](double x) {
        return std::abs(x) <= epsilon;
    };

    if (is_zero(a)) {
        if (is_zero(b)) {
            if (is_zero(c)) {
                return Roots{std::monostate{}};
            }
            return std::nullopt;
        }

        return Roots{-c / b};
    }

    double D = b * b - 4.0 * a * c;

    if (D > epsilon) {
        double s = std::sqrt(D);
        double x1 = (-b - s) / (2.0 * a);
        double x2 = (-b + s) / (2.0 * a);

        if (x1 > x2) {
            std::swap(x1, x2);
        }

        return Roots{std::pair<double, double>{x1, x2}};
    }

    if (is_zero(D)) {
        return Roots{-b / (2.0 * a)};
    }

    return std::nullopt;
}

int main() {
    double a, b, c;
    if (!(std::cin >> a >> b >> c)) {
        return 0;
    }

    auto result = solve(a, b, c);

    if (!result.has_value()) {
        return 0;
    }

    std::visit(Visitor{}, *result);

    return 0;
}