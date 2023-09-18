#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <iostream>
#include "client.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

const size_t BUF_SIZE = 1024 * 1024;

typedef struct sockaddr_in SocketData;

enum ServerTCP_Status
{
	SERVER_SUCCESS,
	SERVER_SELECT_ERROR,
	SERVER_SOCKET_ERROR,
	SERVER_UNINITIALIZED,
	SERVER_SEND_FAIL
};

template <typename Context>
class ServerTCP
{
	using GotMessage = void (*)(ServerTCP<Context> &_server, Client &a_client, int _id, char *_msg, int _length, Context &_context);
	using CloseClient = void (*)(ServerTCP<Context> &_server, int _id, Context &_context);
	using NewClient = int (*)(ServerTCP<Context> &_server, int _id, Context &_context);
	using OnFail = void (*)(int _id, std::string const &_err, Context &_context);

public:
	ServerTCP(int _port, int _backLog, GotMessage _gotMessage, CloseClient _closeClient, NewClient _newClient, OnFail _onFail, Context &_context);
	~ServerTCP();
	void close_all(Client &_client);
	void socket_set(Client &_client);
	int read_incomming_data(Client &_client);
	static void read_incomming_heavy(ServerTCP *a_server, Client *_client);
	void client_no_recieve(int a_result, Client &a_client);
	bool thread_safe_client_no_recieve(int a_result, Client *a_client);
	int read_incomming_data(int const &);
	void get_clients_map(std::map<int, int> &_clients);
	ServerTCP_Status run_server();
	ServerTCP_Status send_message(std::string const &_data, int _sock);
	ServerTCP_Status send_message_raw(const char *_data, size_t a_len, int _sock);
	static void thread_server_run(ServerTCP *a_server);
	void close_client(Client &_client);
	char *get_buffer();
	void ClearBuffer();

private:
	void AddClient(struct sockaddr_in _sin, int _sock);
	void ReadClientsDataIn();
	void remove_client(Client &_client);
	void remove_client(int const &);
	int ListenMainSocket();
	int BindSockect();
	int OpenIncommingConnectionsSocket();
	void clean_up();
	int OpenSocket();
	Client *get_client_by_id(int const &a_id);
	bool is_temination(size_t &a_total, int a_pos);
	bool thread_safe_is_termination(char *a_buffer, size_t &a_total, int a_pos) const;
	int decrease_activity();
	int thread_safe_decrease_activity();


private:
	Context &m_context;
	struct sockaddr_in m_sin;
	int m_listenningSocket;
	std::list<Client> m_clients;
	int m_port;
	int m_backLog;
	fd_set m_fdSet;
	char m_buffer[BUF_SIZE];
	int m_activity;
	int m_numberOfClients;
	GotMessage m_gotMessage;
	CloseClient m_closeClient;
	NewClient m_newClient;
	OnFail m_onFail;
	std::mutex m_mutex;
	std::condition_variable m_cv;
	std::atomic<u_int8_t> m_threads_count;
};

template <typename Context>
ServerTCP<Context>::ServerTCP(int _port, int _backLog, GotMessage _gotMessage, CloseClient _closeClient, NewClient _newClient, OnFail _onFail, Context &_context)
: m_context(_context)
{
	if (!_port || !_backLog || !_gotMessage){
		return;
	}
	m_listenningSocket = OpenSocket();
	if (m_listenningSocket == -1){
		return;
	}
	m_closeClient = _closeClient;
	m_newClient = _newClient;
	m_gotMessage = _gotMessage;
	m_onFail = _onFail;
	m_port = _port;
	m_backLog = _backLog;
	m_numberOfClients = 0;
	m_sin.sin_family = AF_INET;
	m_sin.sin_port = htons(_port);
	m_sin.sin_addr.s_addr = INADDR_ANY;
	m_threads_count = 0;
	OpenIncommingConnectionsSocket();
	BindSockect();
	ListenMainSocket();
	std::list<Client> m_clients = std::list<Client>{};
	ClearBuffer();
}

template <typename Context>
ServerTCP<Context>::~ServerTCP()
{
	for (auto &client : m_clients)
	{
		close_all(client);
	}
	close(m_listenningSocket);
}

template <typename Context>
ServerTCP_Status ServerTCP<Context>::run_server()
{
	int new_sock;
	int yes = 1;
	struct sockaddr_in sin;
	socklen_t addrlen;
	addrlen = sizeof(struct sockaddr_in);
	/*
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	*/
	if (m_listenningSocket == -1) {
		m_onFail(-1, std::string{strerror(errno)}, m_context);
		return SERVER_SOCKET_ERROR;
	}

	for(;;){
		FD_ZERO(&(this->m_fdSet));
		m_activity = 0;
		FD_SET(this->m_listenningSocket, &(this->m_fdSet));
		for (auto &client : m_clients){
			socket_set(client);
		}
		m_activity = select(FD_SETSIZE, &(m_fdSet), NULL, NULL, NULL);
		if (m_activity >= 0){
			if (FD_ISSET(this->m_listenningSocket, &(this->m_fdSet))){
				new_sock = accept(m_listenningSocket, (struct sockaddr *)&sin, &addrlen);
				setsockopt(new_sock, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof(int));
				//setsockopt(new_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
				if (new_sock >= 0){
					if (m_numberOfClients >= m_backLog){
						close(new_sock);
						m_onFail(new_sock, "Server busy", m_context);
						m_closeClient(*this, new_sock, m_context);
					} else {
						if (!m_newClient(*this, new_sock, m_context)){
							close(new_sock);
						} else {
							AddClient(sin, new_sock);
						}
					}
				} else {
					m_onFail(new_sock, std::string{strerror(errno)}, m_context);
				}
				--m_activity;
				if (m_activity <= 0) {
					clean_up();
				}
			}
			ReadClientsDataIn();
		} else {
			return SERVER_SELECT_ERROR;
		}
	}
	for (auto client : m_clients) {
		close_all(client);
	}
	close(m_listenningSocket);
	return SERVER_SUCCESS;
}

template <typename Context>
ServerTCP_Status ServerTCP<Context>::send_message(std::string const &_data, int _sock)
{
	int sent_bytes = -1;
	try{
		sent_bytes = send(_sock, _data.data(), _data.size(), 0);
	} catch(...){
		if (sent_bytes < 0){
			m_onFail(_sock, std::string{strerror(errno)}, m_context);
			return SERVER_SEND_FAIL;
		}
	}
	return SERVER_SUCCESS;
}

template <typename Context>
ServerTCP_Status ServerTCP<Context>::send_message_raw(const char *_data, size_t a_len, int _sock)
{
	int sent_bytes = -1;
	try{
		sent_bytes = send(_sock, _data, a_len, 0);
	} catch(...) {
		if (sent_bytes < 0){
			m_onFail(_sock, std::string{strerror(errno)}, m_context);
			return SERVER_SEND_FAIL;
		}
	}
	return SERVER_SUCCESS;
}

template <typename Context>
void ServerTCP<Context>::close_all(Client &_client)
{
	close(_client.socket());
	m_closeClient(*this, _client.socket(), m_context);
}

template <typename Context>
void ServerTCP<Context>::AddClient(struct sockaddr_in _sin, int _sock)
{
	Client client{_sock, _sin};
	m_clients.push_back(client);
	++m_numberOfClients;
}

template <typename Context>
void ServerTCP<Context>::clean_up()
{
	for (auto &client : m_clients) {
		if (client.is_closed()) {
			close(client.socket());
			FD_CLR(client.socket(), &(m_fdSet));
			--m_numberOfClients;
			remove_client(client);
		}
	}
}

template <typename Context>
void ServerTCP<Context>::ReadClientsDataIn()
{
	for (auto &client : m_clients){
		if (client.is_closed()){
			continue;
		}
		else if(client.heavy()) {
			if(client.is_action()){
				continue;
			} else {
				client.set_action();
				++m_threads_count;
				std::thread{read_incomming_heavy, this, &client}.detach();
			}
			
		} else {

			read_incomming_data(client);
		}
	}
}

template <typename Context>
void ServerTCP<Context>::socket_set(Client &_client)
{
	FD_SET(_client.socket(), &(this->m_fdSet));
}

template <typename Context>
void ServerTCP<Context>::remove_client(Client &_client)
{
	for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		if (it->socket() == _client.socket())
		{
			m_clients.erase(it);
			return;
		}
	}
}

template <typename Context>
void ServerTCP<Context>::remove_client(int const &a_id)
{
	for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		if (it->socket() == a_id)
		{
			m_clients.erase(it);
			return;
		}
	}
}

template <typename Context>
int ServerTCP<Context>::read_incomming_data(Client &_client)
{
	int result = 0;
	int digit_number = 5;
	fd_set m_fdSet_copy = this->m_fdSet;
	if (FD_ISSET(_client.socket(), &(m_fdSet_copy))){
		result = recv(_client.socket(), m_buffer, BUF_SIZE, 0);
		if(result <= 0){
			client_no_recieve(result, _client);
			return decrease_activity();
		}
		std::string data{m_buffer, size_t(result)};
		size_t read_remain = 0;
		if(data.size() > 0){
			data = data.substr(data.find(">>")+2, digit_number);
		}
		try{
			read_remain = std::stol(data);
		} catch(...){};
		size_t read_already = result;
		while(read_already < read_remain){
			result = recv(_client.socket(), m_buffer+read_already, BUF_SIZE, 0);
			if(result <= 0)break;
			read_already += result;
		}
		if(read_already > 1000){
			_client.heavy() = true;
		}
		m_buffer[read_already]='\0';
		if (read_already > 0 && result > 0){
			m_gotMessage(*this, _client, _client.socket(), &m_buffer[4+digit_number], read_already-4-digit_number, m_context);
		} else {
			client_no_recieve(result, _client);
			return decrease_activity();
		}
	}
	return 1;
}

template <typename Context>
void ServerTCP<Context>::read_incomming_heavy(ServerTCP *a_server, Client *_client)
{
	char local_buff[BUF_SIZE] = {0};
	int digit_number = 5;
	for(;;){
		int result = recv(_client->socket(), local_buff, BUF_SIZE, 0);
		if(a_server->thread_safe_client_no_recieve(result, _client)){
			return;
		}
		std::string data{local_buff, size_t(result)};
		size_t read_remain = 0;
		if(data.size() > 0){
			data = data.substr(data.find(">>")+2, digit_number);
		}
		try{
			read_remain = std::stol(data);
		} catch(...){};
		size_t read_already = result;
		while(read_already < read_remain){
			result = recv(_client->socket(), local_buff+read_already, BUF_SIZE, 0);
			if(a_server->thread_safe_client_no_recieve(result, _client)){
				return;
			}
			read_already += result;
		}
		local_buff[read_already]='\0';
		if (read_already > 0 && result > 0){
			a_server->m_gotMessage(*a_server, *_client, _client->socket(), &local_buff[4+digit_number], read_already-4-digit_number, a_server->m_context);
		}
	}
}

template <typename Context>
inline void ServerTCP<Context>::client_no_recieve(int a_result, Client &a_client)
{
	if(a_result == 0){
		m_onFail(a_client.socket(), strerror(errno), m_context);
		m_closeClient(*this, a_client.socket(), m_context);
		FD_CLR(a_client.socket(), &(this->m_fdSet));
		a_client.close();
	}
	else if(a_result < 0){
		m_onFail(a_client.socket(), std::string{strerror(errno)}, m_context);
		a_client.close();
	}
}

template <typename Context>
bool ServerTCP<Context>::thread_safe_client_no_recieve(int a_result, Client *a_client)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if(a_result <= 0){
		m_onFail(a_client->socket(), strerror(errno), m_context);
		m_closeClient(*this, a_client->socket(), m_context);
		a_client->close();
		--m_threads_count;
		return true;
	}
	return false;
}

template <typename Context>
Client *ServerTCP<Context>::get_client_by_id(int const &a_id)
{
	for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		if (it->socket() == a_id)
		{
			return &*it;
		}
	}
	return nullptr;
}

template <typename Context>
inline bool ServerTCP<Context>::is_temination(size_t &a_total, int a_pos)
{
	if(a_pos>3){
		/*
		std::cout<<"\n";
		for(int i=1;i<5;++i){
			std::cout<<i<<":"<<m_buffer[a_total-i]<<",";
		}
		*/
		if(!m_buffer[a_total-1]&&!m_buffer[a_total-2]&&!m_buffer[a_total-3]&&!m_buffer[a_total-4]){
			a_total-=4;
			return true;
		}
	}
	return false;
}

template <typename Context>
inline bool ServerTCP<Context>::thread_safe_is_termination(char *a_buffer, size_t &a_total, int a_pos) const
{
	if(a_pos>3){
		if(!a_buffer[a_total-1]&&!a_buffer[a_total-2]&&!a_buffer[a_total-3]&&!a_buffer[a_total-4]){
			a_total-=4;
			return true;
		}
	}
	return false;
}

template <typename Context>
inline int ServerTCP<Context>::decrease_activity()
{
	--m_activity;
	if (m_activity <= 0){
		return 0;
	}
	return 1;
}

template <typename Context>
inline int ServerTCP<Context>::thread_safe_decrease_activity()
{
	--m_activity;
	if (m_activity <= 0){
		--m_threads_count;
		return 0;
	}
	return 1;
}

template <typename Context>
int ServerTCP<Context>::read_incomming_data(int const &a_id)
{
	return read_incomming_data(*get_client_by_id(a_id));
}

template <typename Context>
void ServerTCP<Context>::get_clients_map(std::map<int, int> &a_clients_map)
{
	a_clients_map.clear();
	for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		a_clients_map[it->socket()] = it->socket();
	}
}

template <typename Context>
int ServerTCP<Context>::OpenIncommingConnectionsSocket()
{
	int optval = 1;
	if (setsockopt(m_listenningSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
	{
		perror("reuse failed");
		return 0;
	}

	setsockopt(m_listenningSocket, /* socket affected */
			   IPPROTO_TCP,		   /* set option at TCP level */
			   TCP_NODELAY,		   /* name of option */
			   (char *)&optval,	   /* the cast is historical cruft */
			   sizeof(int));
	return 1;
}

template <typename Context>
int ServerTCP<Context>::BindSockect()
{
	if ((bind(m_listenningSocket, (struct sockaddr *)&(m_sin), sizeof(struct sockaddr_in))) != 0)
	{
		perror("bind failed");
		close(m_listenningSocket);
		return 0;
	}
	return 1;
}

template <typename Context>
int ServerTCP<Context>::ListenMainSocket()
{
	if ((listen(m_listenningSocket, m_backLog)) < 0)
	{
		perror("listen failed");
		close(m_listenningSocket);
		return 0;
	}
	return 1;
}

template <typename Context>
int ServerTCP<Context>::OpenSocket()
{
	int sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		perror("failed:");
		return -1;
	}
	return sock;
}

template <typename Context>
void ServerTCP<Context>::thread_server_run(ServerTCP *a_server)
{
	try
	{
		a_server->run_server();
	}
	catch (...)
	{
		thread_server_run(a_server);
	}
}

template <typename Context>
void ServerTCP<Context>::close_client(Client &_client)
{
	close(_client.socket());
	--(m_numberOfClients);
	if (m_onFail)
	{
		m_onFail(_client.socket(), strerror(errno), m_context);
	}
	if (m_closeClient)
	{
		m_closeClient(*this, _client.socket(), m_context);
	}
	remove_client(_client);
}

template <typename Context>
void ServerTCP<Context>::ClearBuffer()
{
	for (auto &c : m_buffer)
	{
		if (c == '\0')
		{
			break;
		}
		else
		{
			c = '\0';
		}
	}
}

template <typename Context>
char *ServerTCP<Context>::get_buffer()
{
	return m_buffer;
}

#endif /* TCP_SERVER_HPP */
