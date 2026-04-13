#include <cassert>
#include <concepts>
#include <stack>
#include <type_traits>

template <class T>
requires std::totally_ordered<T> && std::is_arithmetic_v<T>
class Stack
{
public:
    void push(const T& x)
    {
        if (m_stack.empty())
        {
            m_stack.push(x);
            m_min = x;
            return;
        }

        if (x < m_min)
        {
            m_stack.push(static_cast<T>(x + x - m_min));
            m_min = x;
        }
        else
        {
            m_stack.push(x);
        }
    }

    [[nodiscard]] T top() const
    {
        assert(!m_stack.empty());

        const T t = m_stack.top();
        return (t < m_min) ? m_min : t;
    }

    void pop()
    {
        assert(!m_stack.empty());

        const T t = m_stack.top();

        if (t < m_min)
        {
            m_min = static_cast<T>(m_min + m_min - t);
        }

        m_stack.pop();

        if (m_stack.empty()) {
            m_min = T{};
        }
    }

    [[nodiscard]] T min() const
    {
        assert(!m_stack.empty());
        return m_min;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return m_stack.empty();
    }

private:
    std::stack<T> m_stack;
    T m_min{};
};

int main()
{
    Stack<int> stack;

    stack.push(3); 
    assert(stack.top() == 3 && stack.min() == 3);
    stack.push(1); 
    assert(stack.top() == 1 && stack.min() == 1);
    stack.push(2); 
    assert(stack.top() == 2 && stack.min() == 1);
    stack.push(1); 
    assert(stack.top() == 1 && stack.min() == 1);

    stack.pop();   
    assert(stack.top() == 2 && stack.min() == 1);
    stack.pop();   
    assert(stack.top() == 1 && stack.min() == 1);
    stack.pop();   
    assert(stack.top() == 3 && stack.min() == 3);

    stack.pop();
}