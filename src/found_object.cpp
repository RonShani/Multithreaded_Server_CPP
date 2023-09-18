#include "found_object.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <memory>

FoundObject::FoundObject(std::string const &a_data)
{
    m_raw = a_data;
}

std::string &FoundObject::data()
{
    return m_raw;
}
