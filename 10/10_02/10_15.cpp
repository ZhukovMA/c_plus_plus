#include <algorithm>
#include <cassert>
#include <stack>
#include <utility>

template <class T>
class Stack
{
public:
    void push(const T& x)
    {
        const T current_min =
            m_stack.empty() ? x : std::min(x, m_stack.top().second);

        m_stack.emplace(x, current_min);
    }

    [[nodiscard]] const T& top() const {
        assert(!m_stack.empty());
        return m_stack.top().first;
    }

    void pop() {
        assert(!m_stack.empty());
        m_stack.pop();
    }

    [[nodiscard]] const T& min() const
    {
        assert(!m_stack.empty());
        return m_stack.top().second;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return m_stack.empty();
    }

private:
    std::stack<std::pair<T, T>> m_stack;
};

int main() {
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