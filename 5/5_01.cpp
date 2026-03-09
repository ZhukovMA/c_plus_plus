#include <cassert>
#include <string>

class Person
{
public:
    void set_name(const std::string& value) {
        m_name = value;
    }

    void set_age(int value) {
        m_age = value;
    }

    void set_grade(int value)
    {
        m_grade = value;
    }

    const std::string& name() const
    {
        return m_name;
    }

    int age() const
    {
        return m_age;
    }

    int grade() const
    {
        return m_grade;
    }

private:
    std::string m_name;
    int m_age = 0;
    int m_grade = 0;
};

class Builder
{
public:
    Builder()
        : m_person(new Person)
    {
    }

    ~Builder()
    {
        delete m_person;
    }



    Builder(const Builder&) = delete;
    Builder& operator=(const Builder&) = delete;



    Builder& name(const std::string& value)
    {
        m_person->set_name(value);
        return *this;
    }

    Builder& age(int value)
    {
        m_person->set_age(value);
        return *this;
    }

    Builder& grade(int value)
    {
        m_person->set_grade(value);
        return *this;
    }



    Person* get()
    {
        Person* result = m_person;
        m_person = new Person;   
        return result;
    }

private:
    Person* m_person = nullptr;
};



int main()
{
    Builder builder;

    auto person = builder.name("Ivan").age(25).grade(10).get();



    assert(person->name() == "Ivan");
    assert(person->age() == 25);
    assert(person->grade() == 10);

    delete person;
}

