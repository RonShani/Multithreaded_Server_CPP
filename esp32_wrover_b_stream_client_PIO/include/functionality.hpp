#ifndef REMOTE_AI_FUNCTIONALITY
#define REMOTE_AI_FUNCTIONALITY

#include <functional>

#include "task.hpp"
#include "templated_task.hpp"

class Functionality{
public:
    Functionality();
    Functionality &operator=(Functionality const&);

    template <typename Function, typename... Args>
    Functionality(Function a_func, Args&&... args);

    void run();
    
private:
    Task *m_function;
};

template <typename Function, typename... Args>
Functionality::Functionality(Function a_func, Args&&... args)
: m_function(new TaskObject<Function, Args...>{a_func,args...})
{
}

#endif // REMOTE_AI_FUNCTIONALITY

