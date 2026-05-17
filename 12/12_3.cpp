#include <iostream>
#include <string>
#include <string_view>
#include <vector>

class PalindromeFinder {
public:
    static std::string_view longest(std::string_view text) {
        const std::size_t n = text.size();

        if (n == 0) {
            return {};
        }

        std::vector<bool> cache(n * n, false);

        auto cell = [n](std::size_t left, std::size_t right) {
            return left * n + right;
        };

        std::size_t best_left = 0;
        std::size_t best_length = 1;

        for (std::size_t i = 0; i < n; ++i) {
            cache[cell(i, i)] = true;
        }

        for (std::size_t length = 2; length <= n; ++length) {
            for (std::size_t left = 0; left + length <= n; ++left) {
                const std::size_t right = left + length - 1;

                const bool same_edges = text[left] == text[right];

                if (length == 2) {
                    cache[cell(left, right)] = same_edges;
                } else {
                    cache[cell(left, right)] =
                        same_edges && cache[cell(left + 1, right - 1)];
                }

                if (cache[cell(left, right)] && length > best_length) {
                    best_left = left;
                    best_length = length;
                }
            }
        }

        return text.substr(best_left, best_length);
    }
};

int main() {
    std::string input;
    std::getline(std::cin, input);

    std::string_view view(input);

    std::cout << PalindromeFinder::longest(view) << '\n';

    return 0;
}