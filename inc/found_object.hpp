#ifndef FOUND_OBJECT_HPP
#define FOUND_OBJECT_HPP
#include <map>
#include <ctime>
#include <string>

#include "roi.hpp"

class FoundObject{
public:
    FoundObject() = default;
    FoundObject(std::string const &a_data);
    FoundObject(FoundObject &a_data) = default;
    ~FoundObject() = default;

public:
    std::string &data();


private:
    std::string m_raw;
};


#endif // FOUND_OBJECT_HPP