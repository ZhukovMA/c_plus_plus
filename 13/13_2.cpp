#include <cmath>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

class Lexer {
public:
    enum class Kind {
        end,
        number,
        name,
        symbol
    };

    struct Token {
        Kind kind = Kind::end;
        double number = 0.0;
        std::string text;
        char symbol = '\0';
    };

    explicit Lexer(std::string_view source)
        : source_(source) {}

    Token next() {
        skip_spaces();

        if (pos_ >= source_.size()) {
            return Token{Kind::end};
        }

        char c = source_[pos_];

        if (c == ';') {
            pos_ = source_.size();
            return Token{Kind::end};
        }

        if (is_digit(c) || c == '.') {
            return read_number();
        }

        if (is_alpha(c) || c == '_') {
            return read_name();
        }

        if (is_symbol(c)) {
            ++pos_;

            Token token;
            token.kind = Kind::symbol;
            token.symbol = c;
            return token;
        }

        throw std::runtime_error("invalid character");
    }

private:
    std::string_view source_;
    std::size_t pos_ = 0;

    static bool is_space(char c) {
        return std::isspace(static_cast<unsigned char>(c));
    }

    static bool is_digit(char c) {
        return std::isdigit(static_cast<unsigned char>(c));
    }

    static bool is_alpha(char c) {
        return std::isalpha(static_cast<unsigned char>(c));
    }

    static bool is_alnum(char c) {
        return std::isalnum(static_cast<unsigned char>(c));
    }

    static bool is_symbol(char c) {
        return std::string_view("+-*/%^!()[]{}=").contains(c);
    }

    void skip_spaces() {
        while (pos_ < source_.size() && is_space(source_[pos_])) {
            ++pos_;
        }
    }

    Token read_name() {
        std::size_t begin = pos_;

        while (pos_ < source_.size()) {
            char c = source_[pos_];

            if (!is_alnum(c) && c != '_') {
                break;
            }

            ++pos_;
        }

        Token token;
        token.kind = Kind::name;
        token.text = std::string(source_.substr(begin, pos_ - begin));
        return token;
    }

    Token read_number() {
        std::size_t begin = pos_;
        bool has_digits = false;

        while (pos_ < source_.size() && is_digit(source_[pos_])) {
            has_digits = true;
            ++pos_;
        }

        if (pos_ < source_.size() && source_[pos_] == '.') {
            ++pos_;

            while (pos_ < source_.size() && is_digit(source_[pos_])) {
                has_digits = true;
                ++pos_;
            }
        }

        if (!has_digits) {
            throw std::runtime_error("invalid number");
        }

        if (pos_ < source_.size() && (source_[pos_] == 'e' || source_[pos_] == 'E')) {
            ++pos_;

            if (pos_ < source_.size() && (source_[pos_] == '+' || source_[pos_] == '-')) {
                ++pos_;
            }

            std::size_t exponent_begin = pos_;

            while (pos_ < source_.size() && is_digit(source_[pos_])) {
                ++pos_;
            }

            if (exponent_begin == pos_) {
                throw std::runtime_error("invalid exponent");
            }
        }

        Token token;
        token.kind = Kind::number;
        token.number = std::stod(std::string(source_.substr(begin, pos_ - begin)));
        return token;
    }
};

class Parser {
public:
    using Variables = std::unordered_map<std::string, double>;

    Parser(std::string_view source, Variables& variables)
        : lexer_(source), variables_(variables) {
        take();
    }

    double statement() {
        if (token_.kind == Lexer::Kind::name &&
            (token_.text == "set" || token_.text == "let")) {
            take();
            return declaration();
        }

        double result = expression();
        expect_end();
        return result;
    }

private:
    Lexer lexer_;
    Lexer::Token token_;
    Variables& variables_;

    void take() {
        token_ = lexer_.next();
    }

    bool accept(char symbol) {
        if (token_.kind == Lexer::Kind::symbol && token_.symbol == symbol) {
            take();
            return true;
        }

        return false;
    }

    void expect(char symbol) {
        if (!accept(symbol)) {
            std::string message = "expected '";
            message += symbol;
            message += "'";
            throw std::runtime_error(message);
        }
    }

    void expect_end() {
        if (token_.kind != Lexer::Kind::end) {
            throw std::runtime_error("unexpected text after expression");
        }
    }

    double declaration() {
        if (token_.kind != Lexer::Kind::name) {
            throw std::runtime_error("expected variable name after set");
        }

        std::string name = token_.text;
        take();

        accept('=');

        double value = expression();
        variables_[name] = value;

        expect_end();
        return value;
    }

    double expression() {
        return addition();
    }

    double addition() {
        double left = multiplication();

        while (true) {
            if (accept('+')) {
                left += multiplication();
            } else if (accept('-')) {
                left -= multiplication();
            } else {
                return left;
            }
        }
    }

    double multiplication() {
        double left = unary();

        while (true) {
            if (accept('*')) {
                left *= unary();
            } else if (accept('/')) {
                double right = unary();

                if (right == 0.0) {
                    throw std::runtime_error("division by zero");
                }

                left /= right;
            } else if (accept('%')) {
                double right = unary();

                if (right == 0.0) {
                    throw std::runtime_error("remainder by zero");
                }

                left = std::fmod(left, right);
            } else {
                return left;
            }
        }
    }

    double unary() {
        if (accept('+')) {
            return unary();
        }

        if (accept('-')) {
            return -unary();
        }

        return power();
    }

    double power() {
        double left = postfix();

        if (accept('^')) {
            double right = unary();
            left = std::pow(left, right);
        }

        return left;
    }

    double postfix() {
        double value = primary();

        while (accept('!')) {
            value = factorial(value);
        }

        return value;
    }

    double primary() {
        if (token_.kind == Lexer::Kind::number) {
            double value = token_.number;
            take();
            return value;
        }

        if (token_.kind == Lexer::Kind::name) {
            std::string name = token_.text;
            take();

            auto it = variables_.find(name);

            if (it == variables_.end()) {
                throw std::runtime_error("unknown variable: " + name);
            }

            return it->second;
        }

        if (accept('(')) {
            double value = expression();
            expect(')');
            return value;
        }

        if (accept('[')) {
            double value = expression();
            expect(']');
            return value;
        }

        if (accept('{')) {
            double value = expression();
            expect('}');
            return value;
        }

        throw std::runtime_error("expected number, variable, or bracketed expression");
    }

    static double factorial(double value) {
        if (value < 0.0 || std::trunc(value) != value) {
            throw std::runtime_error("factorial requires a non-negative integer");
        }

        double result = 1.0;

        for (double i = 2.0; i <= value; ++i) {
            result *= i;
        }

        return result;
    }
};

class Calculator {
public:
    double evaluate(std::string_view source) {
        Parser parser(source, variables_);
        return parser.statement();
    }

private:
    std::unordered_map<std::string, double> variables_;
};

std::string trim(std::string_view text) {
    std::size_t left = 0;
    std::size_t right = text.size();

    while (left < right && std::isspace(static_cast<unsigned char>(text[left]))) {
        ++left;
    }

    while (right > left && std::isspace(static_cast<unsigned char>(text[right - 1]))) {
        --right;
    }

    return std::string(text.substr(left, right - left));
}

bool almost_equal(double a, double b) {
    constexpr double eps = 1e-9;
    return std::fabs(a - b) <= eps * std::max(1.0, std::max(std::fabs(a), std::fabs(b)));
}

int main(int argc, char* argv[]) {
    std::string file_name = "tests.txt";

    if (argc > 1) {
        file_name = argv[1];
    }

    std::fstream input(file_name, std::ios::in);

    if (!input.is_open()) {
        std::cout << "Cannot open file: " << file_name << '\n';
        return 1;
    }

    Calculator calculator;

    std::string line;
    int line_number = 0;
    int passed = 0;
    int failed = 0;

    while (std::getline(input, line)) {
        ++line_number;

        std::string current = trim(line);

        if (current.empty() || current.starts_with('#')) {
            continue;
        }

        if (current == ";") {
            break;
        }

        std::size_t arrow_pos = current.find("=>");

        if (arrow_pos == std::string::npos) {
            std::cout << "[SKIP] line " << line_number
                      << ": expected separator =>\n";
            continue;
        }

        std::string instruction = trim(std::string_view(current).substr(0, arrow_pos));
        std::string expected_text = trim(std::string_view(current).substr(arrow_pos + 2));

        bool expected_error = expected_text == "ERROR";

        try {
            double actual = calculator.evaluate(instruction);

            if (expected_error) {
                ++failed;
                std::cout << "[FAIL] line " << line_number
                          << ": " << instruction
                          << " expected ERROR, got " << actual << '\n';
                continue;
            }

            double expected = std::stod(expected_text);

            if (almost_equal(actual, expected)) {
                ++passed;
                std::cout << "[OK]   line " << line_number
                          << ": " << instruction
                          << " = " << actual << '\n';
            } else {
                ++failed;
                std::cout << "[FAIL] line " << line_number
                          << ": " << instruction
                          << " expected " << expected
                          << ", got " << actual << '\n';
            }
        } catch (const std::exception& error) {
            if (expected_error) {
                ++passed;
                std::cout << "[OK]   line " << line_number
                          << ": " << instruction
                          << " produced error: " << error.what() << '\n';
            } else {
                ++failed;
                std::cout << "[FAIL] line " << line_number
                          << ": " << instruction
                          << " unexpected error: " << error.what() << '\n';
            }
        }
    }

    std::cout << "\nPassed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';

    return failed == 0 ? 0 : 2;
}