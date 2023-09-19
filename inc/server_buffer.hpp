#ifndef SERVER_BUFFER_MANAGER_HPP
#define SERVER_BUFFER_MANAGER_HPP

#include <stdlib.h>
#include <memory>

class BufferManager {
public:
    BufferManager() = default;
    ~BufferManager() = default;
    BufferManager(size_t a_size);

    char* buffer();
    char* buffer(size_t a_offset);

    size_t buffer_size() const;
    void clear_buffer();


private:
    std::unique_ptr<char> m_buffer;
    size_t m_buffer_size;
};


#endif // SERVER_BUFFER_MANAGER_HPP