#include "server_data.hpp"

void ServerData::client_up()
{
    ++m_number_of_clients;
}

void ServerData::client_down()
{
    --m_number_of_clients;
}

void ServerData::activity_down()
{
    --m_activity;
}

fd_set *ServerData::fd_copy()
{
    m_fd_copy = m_fdSet;
    return &m_fdSet;
}

fd_set *ServerData::fd_main()
{
    return &m_fdSet;
}

sockaddr *ServerData::sin()
{
    return (struct sockaddr *)&(m_sin);
}

int ServerData::clients() const
{
    return m_number_of_clients;
}

int &ServerData::activity()
{
    return m_activity;
}

int ServerData::main_socket() const
{
    return m_listenningSocket;
}

int ServerData::back_log() const
{
    return m_back_log;
}

bool ServerData::is_activity() const
{
    return m_activity > 0;
}

bool ServerData::is_socket_ok() const
{
    return m_listenningSocket != -1;
}

bool ServerData::is_connections_full() const
{
    return m_activity >= m_back_log;
}

size_t ServerData::address_size() const
{
    return sizeof(struct sockaddr_in);
}
