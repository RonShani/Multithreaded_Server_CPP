#ifndef SERVER_DATA_HPP
#define SERVER_DATA_HPP

#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct sockaddr_in SocketData;

class ServerData {
public:
    ServerData() = default;
    ~ServerData() = default;

    template <typename SocketFunc>
    ServerData(SocketFunc &a_socket_func, int a_port, int a_back_log);

    void client_up();
    void client_down();
    void activity_down();

    fd_set *fd_copy();
    fd_set *fd_main();
    struct sockaddr *sin();
        
    int clients() const;
    int &activity();
    int main_socket() const;
    int back_log() const;

    bool is_activity() const;
    bool is_socket_ok() const;
    bool is_connections_full() const;
    size_t address_size() const;

private:
    int m_number_of_clients;
    int m_port;
    struct sockaddr_in m_sin;
	int m_activity;
    int m_listenningSocket;
    int m_back_log;
    fd_set m_fdSet;
    fd_set m_fd_copy;
};

template <typename SocketFunc>
inline ServerData::ServerData(SocketFunc &a_socket_func, int a_port, int a_back_log)
: m_number_of_clients(0)
, m_port(a_port)
, m_sin{}
, m_activity(0)
, m_listenningSocket(0)
, m_back_log(a_back_log)
, m_fdSet{}
, m_fd_copy{}
{
    m_sin.sin_family = AF_INET;
	m_sin.sin_port = htons(m_port);
	m_sin.sin_addr.s_addr = INADDR_ANY;
    m_listenningSocket = a_socket_func();
}

#endif // SERVER_DATA_HPP
