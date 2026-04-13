#include <iostream>
#include <iterator>
#include <boost/iterator/iterator_facade.hpp>

namespace classic {

class Iterator {
private:
    int prev_;
    int curr_;

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = int;
    using difference_type   = std::ptrdiff_t;
    using pointer           = const int*;
    using reference         = int;

    Iterator() : prev_(0), curr_(1) {}

    Iterator(int prev, int curr) : prev_(prev), curr_(curr) {}

    Iterator& operator++() {
        const int next = prev_ + curr_;
        prev_ = curr_;
        curr_ = next;
        return *this;
    }

    Iterator operator++(int) {
        Iterator old = *this;
        ++(*this);
        return old;
    }

    int operator*() const {
        return prev_;
    }

    bool operator==(const Iterator& other) const {
        return prev_ == other.prev_ && curr_ == other.curr_;
    }

    bool operator!=(const Iterator& other) const {
        return !(*this == other);
    }
};

class FibonacciRange {
private:
    int count_;

    static Iterator make_state(int steps) {
        Iterator it;
        for (int i = 0; i < steps; ++i) {
            ++it;
        }
        return it;
    }

public:
    explicit FibonacciRange(int count) : count_(count) {}

    Iterator begin() const {
        return Iterator();
    }

    Iterator end() const {
        return make_state(count_);
    }
};

} 

namespace boosted {

class Iterator
    : public boost::iterator_facade<
          Iterator,
          int,
          boost::forward_traversal_tag,
          int> {
private:
    int prev_;
    int curr_;

    friend class boost::iterator_core_access;

    void increment() {
        const int next = prev_ + curr_;
        prev_ = curr_;
        curr_ = next;
    }

    int dereference() const {
        return prev_;
    }

    bool equal(const Iterator& other) const {
        return prev_ == other.prev_ && curr_ == other.curr_;
    }

public:
    Iterator() : prev_(0), curr_(1) {}

    Iterator(int prev, int curr) : prev_(prev), curr_(curr) {}
};

class FibonacciRange {
private:
    int count_;

    static Iterator make_state(int steps) {
        Iterator it;
        for (int i = 0; i < steps; ++i) {
            ++it;
        }
        return it;
    }

public:
    explicit FibonacciRange(int count) : count_(count) {}

    Iterator begin() const {
        return Iterator();
    }

    Iterator end() const {
        return make_state(count_);
    }
};

} 

int main() {
    std::cout << "Classic iterator: ";
    for (int value : classic::FibonacciRange(12)) {
        std::cout << value << ' ';
    }

    std::cout << "\nBoost facade:    ";
    for (int value : boosted::FibonacciRange(12)) {
        std::cout << value << ' ';
    }

    std::cout << '\n';
    return 0;
}