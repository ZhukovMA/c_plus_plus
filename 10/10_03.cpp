#include <boost/multi_array.hpp>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

class GameOfLife {
public:
    static constexpr std::size_t rows = 10;
    static constexpr std::size_t cols = 10;

    using Cell = std::uint8_t;
    using Field = boost::multi_array<Cell, 2>;

    GameOfLife() : current_(boost::extents[rows][cols]), next_(boost::extents[rows][cols]) {
        clear(current_);
        clear(next_);
    }

    void set_alive(std::size_t r, std::size_t c) {
        if (r < rows && c < cols) {
            current_[r][c] = 1;
        }
    }

    void step() {
        for (std::size_t r = 0; r < rows; ++r) {
            for (std::size_t c = 0; c < cols; ++c) {
                const int neighbors = count_neighbors(r, c);
                const bool alive_now = current_[r][c] != 0;

                if (alive_now) {
                    next_[r][c] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
                } else {
                    next_[r][c] = (neighbors == 3) ? 1 : 0;
                }
            }
        }

        current_.swap(next_);
        clear(next_);
    }

    void print(std::ostream& os, int generation) const {
        os << "Generation " << generation << '\n';
        os << "  ";
        for (std::size_t c = 0; c < cols; ++c) {
            os << c << ' ';
        }
        os << '\n';

        for (std::size_t r = 0; r < rows; ++r) {
            os << r << ' ';
            for (std::size_t c = 0; c < cols; ++c) {
                os << (current_[r][c] ? '#' : '.') << ' ';
            }
            os << '\n';
        }
        os << '\n';
    }

private:
    Field current_;
    Field next_;

    static void clear(Field& field) {
        for (auto& value : field.data()) {
            value = 0;
        }
    }

    int count_neighbors(std::size_t r, std::size_t c) const {
        int total = 0;

        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) {
                    continue;
                }

                const int nr = static_cast<int>(r) + dr;
                const int nc = static_cast<int>(c) + dc;

                if (nr >= 0 && nr < static_cast<int>(rows) &&
                    nc >= 0 && nc < static_cast<int>(cols)) {
                    total += current_[nr][nc] ? 1 : 0;
                }
            }
        }

        return total;
    }
};

int main() {
    GameOfLife game;

    game.set_alive(1, 2);
    game.set_alive(2, 3);
    game.set_alive(3, 1);
    game.set_alive(3, 2);
    game.set_alive(3, 3);

    game.set_alive(6, 6);
    game.set_alive(6, 7);
    game.set_alive(7, 6);
    game.set_alive(7, 7);

    game.set_alive(1, 7);
    game.set_alive(2, 7);
    game.set_alive(3, 7);

    constexpr int iterations = 12;

    for (int gen = 0; gen <= iterations; ++gen) {
        game.print(std::cout, gen);

        if (gen != iterations) {
            game.step();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }

    return 0;
}