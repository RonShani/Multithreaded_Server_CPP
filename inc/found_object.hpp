#ifndef FOUND_OBJECT_HPP
#define FOUND_OBJECT_HPP
#include <map>
#include <ctime>
#include <string>

class FoundObject{
public:
    FoundObject();
    FoundObject(std::string const &a_data);
    FoundObject(FoundObject &a_data) = default;
    ~FoundObject() = default;

    void set_published();
    bool is_published() const;

public:
    std::string &data();


private:
    std::string m_raw;
    bool m_is_published;
};


#endif // FOUND_OBJECT_HPP