#include <print>

class Client
{
public:
    void test() const
    {
        std::print("Client::test\n");
    }
};

class Server
{
public:
    void test() const
    {
        std::print("Server::test\n");
    }
};

template <class Implementation>
class Entity : public Implementation
{
public:
    void test() const
    {
        static_cast<const Implementation&>(*this).test();
    }
};

int main()
{
    Entity<Client> entity;
    entity.test();
}