#include "timer.hpp"

Timer::Timer()
: m_time{std::chrono::steady_clock::now()}
{
}

bool Timer::is_passed_ms(int64_t const &a_ms)
{
  int64_t m = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-m_time).count();
  if(m > a_ms){
    m_time = std::chrono::steady_clock::now();
    return true;
  }
  return false;
}
