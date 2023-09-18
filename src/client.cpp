#include "client.hpp"

Client::Client(int _socket, sockaddr_in _sin)
:m_sin(_sin)
, m_socket(_socket)
, m_to_close(false)
, m_is_heavy(false)
, m_in_action(false)
{
}

sockaddr_in &Client::addr()
{
    return m_sin;
}

int &Client::socket()
{
    return m_socket;
}

bool Client::is_closed()
{
    return m_to_close == true;
}

bool &Client::heavy()
{
    return m_is_heavy;
}

bool Client::is_action() const
{
    return m_in_action;
}

void Client::set_action()
{
    m_in_action = true;
}

void Client::close()
{
    m_to_close = true;
}
