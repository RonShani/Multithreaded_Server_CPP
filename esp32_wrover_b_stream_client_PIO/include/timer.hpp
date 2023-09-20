#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

class Timer{
public:
    Timer();
    bool is_passed_ms(int64_t const &a_ms);

private:
    std::chrono::steady_clock::time_point m_time;
};

#endif // TIMER_HPP