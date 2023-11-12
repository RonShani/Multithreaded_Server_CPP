#ifndef TOPIC_INTERFACE_HPP
#define TOPIC_INTERFACE_HPP

#include <string>

class Topic{
public:
    Topic();
    Topic(std::string &&a_raw);
    Topic(std::string const &a_raw);
    Topic(char *a_str, int a_len);
    ~Topic() = default;
    bool operator==(std::string const &a_name) const;
    bool operator<(Topic const &a_topic) const;
    operator bool() const;
    std::string name() const;
    void set_published();
    bool is_published() const;
    std::string &data();

private:
    std::string m_raw;
    bool m_is_published;
    size_t m_name_pos;
    const char *m_divider = "<|||>";
};


#endif // TOPIC_INTERFACE_HPP