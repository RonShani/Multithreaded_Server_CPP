#include "topic.hpp"
#include <memory>

Topic::Topic()
: m_raw{}
, m_is_published(false)
, m_name_pos(0)
{
}

Topic::Topic(std::string &&a_raw)
: m_raw(std::move(a_raw))
, m_is_published(false)
{
    m_name_pos = a_raw.find(m_divider);
    if (m_name_pos == std::string::npos){
        m_name_pos = 0; 
    }
}

Topic::Topic(std::string const &a_raw)
: m_raw(a_raw)
, m_is_published(false)
{
    m_name_pos = a_raw.find(m_divider);
    if (m_name_pos == std::string::npos){
        m_name_pos = 0; 
    }
}

Topic::Topic(char *a_str, int a_len)
: m_raw(a_str, a_len)
, m_is_published(false)
{
    m_name_pos = m_raw.find(m_divider);
    if (m_name_pos == std::string::npos){
        m_name_pos = 0; 
    }
}

bool Topic::operator==(std::string const &a_name) const
{
    return name() == a_name;
}

bool Topic::operator<(Topic const &a_topic) const
{
    return name().compare(a_topic.name()) < 0;
}

Topic::operator bool() const
{
    return m_raw.size() > 0;
}

std::string Topic::name() const
{
    try{
        return m_raw.substr(0, m_name_pos);
    } catch(...) {
        return {};
    }
    
}

void Topic::set_published()
{
    m_is_published = true;
}

bool Topic::is_published() const
{
    return m_is_published;
}

std::string &Topic::data()
{
    return m_raw;
}
