#include <iostream>

class Wrapper;
Wrapper test();

class Wrapper {
public:
    using FunctionPointer = Wrapper (*)();

    constexpr Wrapper(FunctionPointer pointer = nullptr) noexcept
        : pointer_(pointer) {}

    constexpr operator FunctionPointer() const noexcept {
        return pointer_;
    }

private:
    FunctionPointer pointer_;
};

Wrapper test() {
    std::cout << "test() called\n";
    return Wrapper{&test};
}

int main() {
    Wrapper function = test();
    (*function)();

    return 0;
}