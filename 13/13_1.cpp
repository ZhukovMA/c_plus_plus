#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

std::string to_hex_string(const std::vector<std::uint8_t>& bytes) {
    std::stringstream stream;

    stream << std::hex << std::right << std::setfill('0');

    for (std::uint8_t byte : bytes) {
        stream << std::setw(2) << static_cast<unsigned>(byte);
    }

    return stream.str();
}

std::uint8_t hex_digit_value(char c) {
    std::uint8_t value = static_cast<std::uint8_t>(c - '0');

    if (value <= 9) {
        return value;
    }

    value = static_cast<std::uint8_t>(c - 'a');

    if (value <= 5) {
        return static_cast<std::uint8_t>(value + 10);
    }

    throw std::invalid_argument("hex string must contain only lowercase hex digits");
}

std::vector<std::uint8_t> from_hex_string(const std::string& text) {
    if (text.size() % 2 != 0) {
        throw std::invalid_argument("hex string length must be even");
    }

    std::vector<std::uint8_t> result;
    result.reserve(text.size() / 2);

    for (std::size_t i = 0; i < text.size(); i += 2) {
        std::uint8_t high = hex_digit_value(text[i]);
        std::uint8_t low = hex_digit_value(text[i + 1]);

        std::uint8_t byte = static_cast<std::uint8_t>((high << 4) | low);
        result.push_back(byte);
    }

    return result;
}

int main() {
    std::vector<std::uint8_t> data{
        0x00, 0x01, 0x0f, 0x10, 0xab, 0xff
    };

    std::string hex = to_hex_string(data);

    std::cout << hex << '\n';

    std::vector<std::uint8_t> restored = from_hex_string(hex);

    for (std::uint8_t byte : restored) {
        std::cout << static_cast<unsigned>(byte) << ' ';
    }

    std::cout << '\n';

    return 0;
}