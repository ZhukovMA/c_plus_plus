#include <cstring>
#include <iostream>
#include <type_traits>

class Entity_v1 {
private:
    int value;

public:
    explicit Entity_v1(int v = 0) : value(v) {}

    int get() const {
        return value;
    }
};

class Entity_v2 {
public:
    int value;
};

// first way 
void hack_by_reinterpret_cast(Entity_v1& obj, int new_value) {
    static_assert(std::is_standard_layout_v<Entity_v1>);
    static_assert(std::is_standard_layout_v<Entity_v2>);
    static_assert(sizeof(Entity_v1) == sizeof(Entity_v2));

    auto& fake = reinterpret_cast<Entity_v2&>(obj);
    fake.value = new_value;
}

// additional way
void hack_by_memcpy(Entity_v1& target, int new_value) {
    static_assert(std::is_trivially_copyable_v<Entity_v1>);

    Entity_v1 donor{new_value};
    std::memcpy(&target, &donor, sizeof(Entity_v1));
}

int main() {
    Entity_v1 e{10};
    std::cout << "start: " << e.get() << '\n';

    hack_by_reinterpret_cast(e, 111);
    std::cout << "after reinterpret_cast hack: " << e.get() << '\n';

    hack_by_memcpy(e, 222);
    std::cout << "after memcpy hack: " << e.get() << '\n';

    return 0;
}