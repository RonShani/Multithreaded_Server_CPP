#include "functionality.hpp"

#include "task.hpp"
#include "templated_task.hpp"

Functionality::Functionality()
: m_function(nullptr)
{
}

Functionality &Functionality::operator=(Functionality const &a_func)
{
    m_function = a_func.m_function;
    return *this;
}

void Functionality::run()
{
    m_function->run();
}