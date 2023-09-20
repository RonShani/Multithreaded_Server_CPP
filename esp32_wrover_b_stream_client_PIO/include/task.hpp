#ifndef TASK_HPP
#define TASK_HPP
#include <iostream>
#include <functional>
#include <ctime>
#include <chrono>

class Task
{
public:
    Task() = default;
    virtual void run() = 0;
    virtual ~Task() = default;
};

#endif //TASK_HPP