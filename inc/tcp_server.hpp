#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include "headers_included.hpp"
#include "server_data.hpp"

template <typename Context>
class ServerTCP {
public:
	ServerTCP(int _port, int _backLog, GotMessage<Context> _gotMessage, CloseClient<Context> _closeClient, NewClient<Context> _newClient, OnFail<Context> _onFail, Context &_context, size_t a_buffer_size = 1024);
	~ServerTCP();
	
	ServerTCP_Status run_server();
	ServerTCP_Status send_message(std::string const &_data, int _sock);
	ServerTCP_Status send_message_raw(const char *_data, size_t a_len, int _sock);

	void close_client(Client &_client);
	char *get_buffer();

private:
	void set_no_delay(int a_socket, bool a_no_delay);
	void set_recieve_timeout(int a_socket, size_t a_seconds, size_t a_microseconds);

private:
	void add_client(struct sockaddr_in _sin, int _sock);
	void read_clients_data_income();
	void remove_client(Client &_client);
	void remove_client(int const &);
	void clean_up();
	void close_all(Client &_client);
	void socket_set(Client &_client);
	void clear_buffer();

private:
	int read_incomming_data(int const &);
	int read_incomming_data(Client &_client);
	int listen_main_socket();
	int bind_socket();
	int open_incomming_connections_socket();
	
	int decrease_activity();
	int recieve_from_client(int a_socket, char *a_buffer, size_t a_buffer_size, size_t a_starting_point, Client *a_client, bool a_is_threaded = false);

private:
	bool client_no_recieve(int a_result, Client &a_client);
	bool thread_safe_client_no_recieve(int a_result, Client *a_client);

private:
	static int open_socket();
	static void read_incomming_heavy(ServerTCP *a_server, Client *_client);
	size_t get_total_data_size(char *a_buffer, size_t a_read_already, int a_digit_number);
	Client *get_client_by_id(int const &a_id);

private:
	Context &m_context;
	ServerData m_server_data;
	std::list<Client> m_clients;
	char *m_buffer;
	GotMessage<Context> m_gotMessage;
	CloseClient<Context> m_closeClient;
	NewClient<Context> m_newClient;
	OnFail<Context> m_onFail;
	std::mutex m_mutex;
	std::condition_variable m_cv;
	std::atomic<u_int8_t> m_threads_count;
	size_t m_buffer_size;
};

template <typename Context>
ServerTCP<Context>::ServerTCP(int _port, int _backLog, GotMessage<Context> _gotMessage, CloseClient<Context> _closeClient, NewClient<Context> _newClient, OnFail<Context> _onFail, Context &_context, size_t a_buffer_size)
: m_context(_context)
, m_server_data(open_socket, _port, _backLog)
, m_clients{}
{
	m_closeClient = _closeClient;
	m_newClient = _newClient;
	m_gotMessage = _gotMessage;
	m_onFail = _onFail;
	m_threads_count = 0;
	m_buffer_size = a_buffer_size;
	m_buffer = new char[m_buffer_size];
	open_incomming_connections_socket();
	bind_socket();
	listen_main_socket();
	clear_buffer();
}

template <typename Context>
ServerTCP<Context>::~ServerTCP()
{
	for (auto &client : m_clients){
		close_all(client);
	}
	close(m_server_data.main_socket());
	delete [] m_buffer;
}

template <typename Context>
ServerTCP_Status ServerTCP<Context>::run_server()
{
	struct sockaddr_in sin;
	socklen_t addrlen;
	addrlen = sizeof(struct sockaddr_in);
	if (not m_server_data.is_socket_ok()) {
		m_onFail(-1, std::string{strerror(errno)}, m_context);
		return ServerTCP_Status::SERVER_SOCKET_ERROR;
	}

	for(;;){
		FD_ZERO(m_server_data.fd_main());
		FD_SET(m_server_data.main_socket(), m_server_data.fd_main());
		for (auto &client : m_clients){
			socket_set(client);
		}
		m_server_data.activity() = select(FD_SETSIZE, m_server_data.fd_main(), NULL, NULL, NULL);
		if (m_server_data.is_activity()){
			if (FD_ISSET(m_server_data.main_socket(), m_server_data.fd_main())){
				int new_sock = accept(m_server_data.main_socket(), m_server_data.sin(), &addrlen);
				set_no_delay(new_sock, true);
				if (new_sock >= 0){
					if (m_server_data.is_connections_full()){
						close(new_sock);
						m_onFail(new_sock, "Server busy", m_context);
						m_closeClient(new_sock, m_context);
					} else {
						if (!m_newClient(new_sock, m_context)){
							close(new_sock);
						} else {
							add_client(sin, new_sock);
						}
					}
				} else {
					m_onFail(new_sock, std::string{strerror(errno)}, m_context);
				}
				m_server_data.activity_down();
				if (not m_server_data.is_activity()) {
					clean_up();
				}
			}
			read_clients_data_income();
		} else {
			return ServerTCP_Status::SERVER_SELECT_ERROR;
		}
	}
	for (auto client : m_clients) {
		close_all(client);
	}
	close(m_server_data.main_socket());
	return ServerTCP_Status::SERVER_SUCCESS;
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
			return ServerTCP_Status::SERVER_SEND_FAIL;
		}
	}
	return ServerTCP_Status::SERVER_SUCCESS;
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
			return ServerTCP_Status::SERVER_SEND_FAIL;
		}
	}
	return ServerTCP_Status::SERVER_SUCCESS;
}

template <typename Context>
void ServerTCP<Context>::close_all(Client &_client)
{
	close(_client.socket());
	m_closeClient(_client.socket(), m_context);
}

template <typename Context>
void ServerTCP<Context>::add_client(struct sockaddr_in _sin, int _sock)
{
	Client client{_sock, _sin};
	m_clients.push_back(client);
	m_server_data.client_up();
}

template <typename Context>
void ServerTCP<Context>::clean_up()
{
	for (auto &client : m_clients) {
		if (client.is_closed()) {
			close(client.socket());
			FD_CLR(client.socket(), m_server_data.fd_main());
			m_server_data.client_down();
			remove_client(client);
		}
	}
}

template <typename Context>
void ServerTCP<Context>::read_clients_data_income()
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
	FD_SET(_client.socket(), m_server_data.fd_main());
}

template <typename Context>
void ServerTCP<Context>::remove_client(Client &_client)
{
	for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
		if (it->socket() == _client.socket()) {
			m_clients.erase(it);
			return;
		}
	}
}

template <typename Context>
void ServerTCP<Context>::remove_client(int const &a_id)
{
	for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
		if (it->socket() == a_id) {
			m_clients.erase(it);
			return;
		}
	}
}

template <typename Context>
int ServerTCP<Context>::read_incomming_data(Client &_client)
{
	int digit_number = 5;
	if (FD_ISSET(_client.socket(), m_server_data.fd_copy())){
		try{
			size_t read_already = recieve_from_client(_client.socket(), m_buffer, m_buffer_size, 0, &_client);
			size_t read_remain = get_total_data_size(m_buffer, read_already, digit_number);
			while(read_already < read_remain){
				read_already += recieve_from_client(_client.socket(), m_buffer, m_buffer_size, read_already, &_client);
			}
			if(read_already > 1000){
				_client.heavy() = true;
			}
			m_buffer[read_already]='\0';
			if (read_already > 0){
				m_gotMessage(_client, _client.socket(), &m_buffer[4+digit_number], read_already-4-digit_number, m_context);
			}
		} catch(...) {
			return decrease_activity();
		}
	}
	return 1;
}

template <typename Context>
int ServerTCP<Context>::recieve_from_client(int a_socket, char *a_buffer, size_t a_buffer_size, size_t a_starting_point, Client *a_client, bool a_is_threaded)
{
	int result = recv(a_socket, a_buffer, a_buffer_size, a_starting_point);
	if (a_is_threaded){
		if(thread_safe_client_no_recieve(result, a_client)){
			throw "error recieving";
		}
	} else if(client_no_recieve(result, *a_client)){
		throw "error recieving";
	}
	return result;
}

template <typename Context>
size_t ServerTCP<Context>::get_total_data_size(char *a_buffer, size_t a_read_already, int a_digit_number)
{
	std::string data{a_buffer, a_read_already};
	if(data.size() > 0){
		data = data.substr(data.find(">>")+2, a_digit_number);
	}
	return std::stol(data);
}

template <typename Context>
void ServerTCP<Context>::read_incomming_heavy(ServerTCP *a_server, Client *_client)
{
	char *local_buff = new char[a_server->m_buffer_size];
	int digit_number = 5;
	for(;;){
		try{
			size_t read_already = a_server->recieve_from_client(_client->socket(), local_buff, a_server->m_buffer_size, 0, _client, true);
			std::string data{local_buff, read_already};
			size_t read_remain = a_server->get_total_data_size(local_buff, read_already, digit_number);
			while(read_already < read_remain){
				read_already += a_server->recieve_from_client(_client->socket(), local_buff, a_server->m_buffer_size, read_already, _client, true);
			}
			local_buff[read_already]='\0';
			if (read_already > 0){
				a_server->m_gotMessage(*_client, _client->socket(), &local_buff[4+digit_number], read_already-4-digit_number, a_server->m_context);
			}
		} catch(...) {
			delete [] local_buff;
			return;
		}
	}
	
}

template <typename Context>
inline bool ServerTCP<Context>::client_no_recieve(int a_result, Client &a_client)
{
	if(a_result <= 0){
		m_onFail(a_client.socket(), strerror(errno), m_context);
		m_closeClient(a_client.socket(), m_context);
		FD_CLR(a_client.socket(), m_server_data.fd_main());
		a_client.close();
		return true;
	}
	return false;
}

template <typename Context>
bool ServerTCP<Context>::thread_safe_client_no_recieve(int a_result, Client *a_client)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if(a_result <= 0){
		m_onFail(a_client->socket(), strerror(errno), m_context);
		m_closeClient(a_client->socket(), m_context);
		a_client->close();
		--m_threads_count;
		return true;
	}
	return false;
}

template <typename Context>
Client *ServerTCP<Context>::get_client_by_id(int const &a_id)
{
	for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
		if (it->socket() == a_id) {
			return &*it;
		}
	}
	return nullptr;
}

template <typename Context>
inline int ServerTCP<Context>::decrease_activity()
{
	m_server_data.activity_down();
	if (m_server_data.is_activity()){
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
int ServerTCP<Context>::open_incomming_connections_socket()
{
	int optval = 1;
	if (setsockopt(m_server_data.main_socket(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
		perror("reuse failed");
		return 0;
	}
	return 1;
}

template <typename Context>
int ServerTCP<Context>::bind_socket()
{
	if ((bind(m_server_data.main_socket(), m_server_data.sin(), m_server_data.address_size())) != 0) {
		perror("bind failed");
		close(m_server_data.main_socket());
		return 0;
	}
	return 1;
}

template <typename Context>
int ServerTCP<Context>::listen_main_socket()
{
	if ((listen(m_server_data.main_socket(), m_server_data.back_log())) < 0) {
		perror("listen failed");
		close(m_server_data.main_socket());
		return 0;
	}
	return 1;
}

template <typename Context>
int ServerTCP<Context>::open_socket()
{
	int sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("failed:");
		return -1;
	}
	return sock;
}

template <typename Context>
void ServerTCP<Context>::close_client(Client &_client)
{
	close(_client.socket());
	m_server_data.client_down();
	m_onFail(_client.socket(), strerror(errno), m_context);
	m_closeClient(_client.socket(), m_context);
	remove_client(_client);
}

template <typename Context>
void ServerTCP<Context>::clear_buffer()
{
	for (size_t i = 0; i < m_buffer_size; ++i){
		if (m_buffer[i] == '\0'){
			break;
		} else {
			m_buffer[i] = '\0';
		}
	}
}

template <typename Context>
inline void ServerTCP<Context>::set_no_delay(int a_socket, bool a_no_delay)
{
	int yes = (a_no_delay ? 1 : 0);
	setsockopt(a_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof(int));
}

template <typename Context>
inline void ServerTCP<Context>::set_recieve_timeout(int a_socket, size_t a_seconds, size_t a_microseconds)
{
	struct timeval tv;
	tv.tv_sec = a_seconds;
	tv.tv_usec = a_microseconds;
	setsockopt(a_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
}

template <typename Context>
char *ServerTCP<Context>::get_buffer()
{
	return m_buffer;
}

#endif /* TCP_SERVER_HPP */
