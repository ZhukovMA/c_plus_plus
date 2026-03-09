/////////////////////////////////////////////////////////////////////////////////

// chapter : Number Processing

/////////////////////////////////////////////////////////////////////////////////

// section : Chrono Management

/////////////////////////////////////////////////////////////////////////////////

// content : Timing

/////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <print>
#include <string>
#include <vector>

/////////////////////////////////////////////////////////////////////////////////

template <typename D = std::chrono::duration<double>>
class Timer
{
public:
    Timer(std::string const& scope)
        : m_scope(scope), m_running(false)
    {
    }

//  -----------------------------------------------------------------------------

    ~Timer()
    {
        if (m_running)
        {
            stop();
        }

        std::print("{} : {:.6f}\n", m_scope, average());
    }

//  -----------------------------------------------------------------------------

    void start()
    {
        assert(!m_running);

        m_begin = clock_t::now();
        m_running = true;
    }

//  -----------------------------------------------------------------------------

    void stop()
    {
        assert(m_running);

        auto const interval =
            std::chrono::duration_cast<std::chrono::duration<double>>(clock_t::now() - m_begin);

        m_intervals.push_back(interval);
        m_running = false;
    }

//  -----------------------------------------------------------------------------

    auto elapsed() const
    {
        if (m_running)
        {
            return std::chrono::duration_cast<D>(clock_t::now() - m_begin);
        }

        if (!m_intervals.empty())
        {
            return std::chrono::duration_cast<D>(m_intervals.back());
        }

        return D::zero();
    }

//  -----------------------------------------------------------------------------

    double average() const
    {
        if (m_intervals.empty())
        {
            return 0.0;
        }

        auto total = std::chrono::duration<double>::zero();

        for (auto const& interval : m_intervals)
        {
            total += interval;
        }

        return (total / m_intervals.size()).count();
    }

private:
    using clock_t = std::chrono::steady_clock;

//  -----------------------------------------------------------------------------

    std::string m_scope;

    bool m_running;

    clock_t::time_point m_begin;

    std::vector<std::chrono::duration<double>> m_intervals;
};

/////////////////////////////////////////////////////////////////////////////////

auto calculate(std::size_t size)
{
    auto x = 0.0;

    for (auto i = 0uz; i < size; ++i)
    {
        x += std::pow(std::sin(i), 2) + std::pow(std::cos(i), 2);
    }

    return x;
}

/////////////////////////////////////////////////////////////////////////////////

auto equal(double x, double y, double epsilon = 1e-6)
{
    return std::abs(x - y) < epsilon;
}

/////////////////////////////////////////////////////////////////////////////////

int main()
{
    Timer timer("main : timer");

//  -----------------------------------------------

    for (auto i = 0uz; i < 5; ++i)
    {
        timer.start();

        assert(equal(calculate(1'000'000), 1'000'000));

        timer.stop();
    }

    assert(timer.average() > 0.0);
}

/////////////////////////////////////////////////////////////////////////////////