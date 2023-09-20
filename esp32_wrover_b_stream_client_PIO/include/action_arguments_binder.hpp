#ifndef ACTION_ARGUMENTS_BINDER_HPP
#define ACTION_ARGUMENTS_BINDER_HPP

#include "task.hpp"
#include <initializer_list>

#include <vector>
#include <functional>
#include <tuple>
#include <cstdarg>
#include <type_traits>
#include <cstring>


template <typename... T>
class Action {

public:
  using bind_type = decltype(std::bind(std::declval<std::function<void(T...)>>(),std::declval<T>()...));

  template <typename... ActionArgumentsCoupling>
  Action(std::function<void(T...)> f, ActionArgumentsCoupling&&... args);
  void act();

private:
  bind_type m_bind;
};

template <typename... T>
template <typename... ActionArgumentsCoupling>
Action<T...>::Action(std::function<void(T...)> f, ActionArgumentsCoupling&&... args)
: m_bind(f,std::forward<ActionArgumentsCoupling>(args)...)
{ 
}

template <typename... T>
void Action<T...>::act()
{
  m_bind();
}

#endif //ACTION_ARGUMENTS_BINDER_HPP