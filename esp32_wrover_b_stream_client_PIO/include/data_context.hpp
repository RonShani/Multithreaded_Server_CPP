#ifndef DATA_CONTEXT_HPP
#define DATA_CONTEXT_HPP

#include <map>
#include <stdlib.h>
#include <Arduino.h>

class DataContext{
public:
    DataContext(void *a_client);
    ~DataContext() = default;

    bool validate_subject(char *a_line, size_t a_length);
    bool validate_subject(String const &a_subject);

    void add_subject(const char *a_subject, size_t a_length);
    void add_subject(String const &a_subject);
    void clear_validations();
    
    String to_string(const char *a_line, size_t a_length);
    std::map<String, bool> &topics();
    bool &validation();
    void *client();
    const char *rv() const;

private:
    std::map<String, bool> m_sub_topics;
    bool m_is_validation;
    void *m_remote_ai;
    const char *m_rv = "PUB/robot_view<|||>";
};

#endif // DATA_CONTEXT_HPP

