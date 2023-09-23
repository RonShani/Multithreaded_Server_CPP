#include "found_object.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <memory>

FoundObject::FoundObject()
: m_raw{}
, m_is_published(false)
{
}

FoundObject::FoundObject(std::string const &a_data)
: m_raw(a_data)
, m_is_published(false)
{
}

void FoundObject::set_published()
{
    m_is_published = true;
}

bool FoundObject::is_published() const
{
    return m_is_published;
}

std::string &FoundObject::data()
{
    return m_raw;
}
