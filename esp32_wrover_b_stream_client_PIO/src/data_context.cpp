#include "data_context.hpp"

DataContext::DataContext(void *a_client) : m_sub_topics{}, m_is_validation(false), m_remote_ai(a_client){}

bool DataContext::validate_subject(char *a_line, size_t a_length)
{
    String line = to_string(a_line, a_length);
    return validate_subject(line);
}

bool DataContext::validate_subject(String const &a_subject)
{
    if(m_sub_topics.find(a_subject) != m_sub_topics.end()){
        m_sub_topics[a_subject] = true;
        return m_sub_topics[a_subject];
    }
    return false;
}

void DataContext::add_subject(const char *a_subject, size_t a_length)
{
    m_sub_topics[to_string(a_subject, a_length)] = false;
}

void DataContext::add_subject(String const &a_subject)
{
    m_sub_topics[a_subject] = false;
}

void DataContext::clear_validations()
{
    for(auto &sub : m_sub_topics){
        sub.second = false;
    }
}

String DataContext::to_string(const char *a_line, size_t a_length)
{
    String out;
    for(size_t i = 0; i < a_length; ++i){
        out += a_line[i];
    }
    return out;
}

std::map<String, bool> &DataContext::topics()
{
    return m_sub_topics;
}

bool &DataContext::validation()
{
    return m_is_validation;
}

void *DataContext::client()
{
    return m_remote_ai;
}

const char *DataContext::rv() const
{
    return m_rv;
}
