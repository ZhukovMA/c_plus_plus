#include <cstddef>
#include <iostream>
#include <new>

template <typename Derived>
class Entity
{
public:
    static void* operator new(std::size_t size)
    {
        trace_allocate("in Entity::operator new", size);
        return ::operator new(size);
    }

    static void operator delete(void* ptr) noexcept
    {
        trace_deallocate("in Entity::operator delete", ptr);
        ::operator delete(ptr);
    }

    static void* operator new[](std::size_t size)
    {
        trace_allocate("in Entity::operator new[]", size);
        return ::operator new[](size);
    }

    static void operator delete[](void* ptr) noexcept
    {
        trace_deallocate("in Entity::operator delete[]", ptr);
        ::operator delete[](ptr);
    }

    static void* operator new(std::size_t size, const std::nothrow_t& tag) noexcept
    {
        trace_allocate("in Entity::operator new(nothrow)", size);
        return ::operator new(size, tag);
    }

    static void operator delete(void* ptr, const std::nothrow_t& tag) noexcept
    {
        trace_deallocate("in Entity::operator delete(nothrow)", ptr);
        ::operator delete(ptr, tag);
    }

    static void* operator new[](std::size_t size, const std::nothrow_t& tag) noexcept
    {
        trace_allocate("in Entity::operator new[](nothrow)", size);
        return ::operator new[](size, tag);
    }

    static void operator delete[](void* ptr, const std::nothrow_t& tag) noexcept
    {
        trace_deallocate("in Entity::operator delete[](nothrow)", ptr);
        ::operator delete[](ptr, tag);
    }

protected:
    Entity() = default;
    ~Entity() = default;

private:
    static void trace_allocate(const char* name, std::size_t size)
    {
        std::cout << name << " " << size << " bytes\n";
    }

    static void trace_deallocate(const char* name, void* ptr)
    {
        std::cout << name << " " << static_cast<const void*>(ptr) << '\n';
    }
};

class Client : private Entity<Client>
{
public:
    Client()
    {
        std::cout << "Client constructor\n";
    }

    ~Client()
    {
        std::cout << "Client destructor\n";
    }

    using Entity<Client>::operator new;
    using Entity<Client>::operator delete;
    using Entity<Client>::operator new[];
    using Entity<Client>::operator delete[];
};

int main()
{
    Client* a = new Client;
    delete a;

    Client* arr = new Client[3];
    delete[] arr;

    Client* b = new (std::nothrow) Client;
    if (b != nullptr)
    {
        delete b;
    }

    Client* safe_arr = new (std::nothrow) Client[2];
    if (safe_arr != nullptr)
    {
        delete[] safe_arr;
    }
}