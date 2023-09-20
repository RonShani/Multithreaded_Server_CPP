#ifndef TEMPLATED_TASK_OBJECT_HPP
#define TEMPLATED_TASK_OBJECT_HPP

#include "task.hpp"
#include "action_arguments_binder.hpp"
#include <initializer_list>
#include <vector>
#include <functional>
#include <tuple>
#include <cstdarg>
#include <type_traits>
#include <cstring>

template <typename Function, typename... Args>
class TaskObject : public Task{
public:
    TaskObject() = default;
    ~TaskObject() = default;
    TaskObject(TaskObject const &) = default;
    TaskObject(Function a_func, Args&&... args);
    void run() override;

private:
    Action<Args...> m_task_function;
};

template <typename Function, typename... Args>
TaskObject<Function, Args...>::TaskObject(Function a_func, Args&&... args)
: m_task_function{a_func,args...}
{
}

template <typename Function, typename... Args>
void TaskObject<Function, Args...>::run()
{
    m_task_function.act();
}


#endif // TEMPLATED_TASK_OBJECT_HPP
