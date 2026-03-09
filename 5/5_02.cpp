
#include <print>

class Entity
{
public:

    virtual ~Entity() = default;


    virtual void test() const = 0;
};


class Client : public virtual Entity
{
public:

    void test() const override
    {
        std::print("Client::test\n");
    }
};


class Server : public virtual Entity
{
public:

    void test() const override
    {
        std::print("Server::test\n");
    }
};


template <class T>
class Decorator : public virtual Entity, public T
{
public:

    using T::T;


    void test() const override
    {
        std::print("Decorator::test : ");

        T::test();
    }
};


int main()
{
    Entity* entity_1 = new Client;

    Entity* entity_2 = new Decorator<Client>;


    entity_1->test();
    entity_2->test();


    delete entity_2;

    delete entity_1;
}
