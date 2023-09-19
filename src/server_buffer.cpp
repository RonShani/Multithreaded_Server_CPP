#include "server_buffer.hpp"

BufferManager::BufferManager(size_t a_size)
: m_buffer{new char[a_size]}
, m_buffer_size{a_size}
{
}

char *BufferManager::buffer()
{
    return &*m_buffer;
}

char *BufferManager::buffer(size_t a_offset)
{
    return &(&*m_buffer)[a_offset];
}

size_t BufferManager::buffer_size() const
{
    return m_buffer_size;
}

void BufferManager::clear_buffer()
{
	for (size_t i = 0; i < m_buffer_size; ++i){
		if ((&*m_buffer)[i] == '\0'){
			break;
		} else {
			(&*m_buffer)[i] = '\0';
		}
	}
}