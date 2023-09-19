#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include "headers_included.hpp"
#include "server_data.hpp"
#include "server_buffer.hpp"

template <typename Context>
class ServerTCP {
public:
	ServerTCP(int a_port, int a_back_log, GotMessage<Context> a_got_message, CloseClient<Context> a_close_client, NewClient<Context> a_new_client, OnFail<Context> a_on_fail, Context &a_context, size_t a_buffer_size = 1024);
	~ServerTCP();
	
	ServerTCP_Status run_server();
	ServerTCP_Status send_message(std::string const &a_data, int a_sock);
	ServerTCP_Status send_message_raw(const char *a_data, size_t a_len, int a_sock);

	void close_client(Client &a_client);
	char *get_buffer();

private:
	void set_no_delay(int a_socket, bool a_no_delay);
	void set_recieve_timeout(int a_socket, size_t a_seconds, size_t a_microseconds);

private:
	void add_client(struct sockaddr_in a_sin, int a_sock);
	void read_clients_data_income();
	void remove_client(Client &a_client);
	void remove_client(int const &);
	void clean_up();
	void close_all(Client &a_client);
	void socket_set(Client &a_client);
	void clear_buffer();

private:
	int read_incomming_data(int const &);
	int read_incomming_data(Client &a_client);
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
	static void read_incomming_heavy(ServerTCP *a_server, Client *a_client);
	size_t get_total_data_size(char *a_buffer, size_t a_read_already, int a_digit_number);
	Client *get_client_by_id(int const &a_id);

private:
	Context &m_context;
	ServerData m_server_data;
	std::list<Client> m_clients;
	BufferManager m_buffer;
	GotMessage<Context> m_got_message;
	CloseClient<Context> m_close_client;
	NewClient<Context> m_new_client;
	OnFail<Context> m_on_fail;
	std::mutex m_mutex;
	std::atomic<u_int8_t> m_threads_count;
};

template <typename Context>
ServerTCP<Context>::ServerTCP(int a_port, int a_back_log, GotMessage<Context> a_got_message, CloseClient<Context> a_close_client, NewClient<Context> a_new_client, OnFail<Context> a_on_fail, Context &a_context, size_t a_buffer_size)
: m_context(a_context)
, m_server_data(open_socket, a_port, a_back_log)
, m_clients{}
, m_buffer{a_buffer_size}
, m_got_message(a_got_message)
, m_close_client(a_close_client)
, m_new_client(a_new_client)
, m_on_fail(a_on_fail)
, m_mutex{}
, m_threads_count(0)
{
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
}

template <typename Context>
ServerTCP_Status ServerTCP<Context>::run_server()
{
	struct sockaddr_in sin;
	socklen_t addrlen;
	addrlen = sizeof(struct sockaddr_in);
	if (not m_server_data.is_socket_ok()) {
		m_on_fail(-1, std::string{strerror(errno)}, m_context);
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
						m_on_fail(new_sock, "Server busy", m_context);
						m_close_client(new_sock, m_context);
					} else {
						if (!m_new_client(new_sock, m_context)){
							close(new_sock);
						} else {
							add_client(sin, new_sock);
						}
					}
				} else {
					m_on_fail(new_sock, std::string{strerror(errno)}, m_context);
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
ServerTCP_Status ServerTCP<Context>::send_message(std::string const &a_data, int a_sock)
{
	int sent_bytes = -1;
	try{
		sent_bytes = send(a_sock, a_data.data(), a_data.size(), 0);
	} catch(...){
		if (sent_bytes < 0){
			m_on_fail(a_sock, std::string{strerror(errno)}, m_context);
			return ServerTCP_Status::SERVER_SEND_FAIL;
		}
	}
	return ServerTCP_Status::SERVER_SUCCESS;
}

template <typename Context>
ServerTCP_Status ServerTCP<Context>::send_message_raw(const char *a_data, size_t a_len, int a_sock)
{
	int sent_bytes = -1;
	try{
		sent_bytes = send(a_sock, a_data, a_len, 0);
	} catch(...) {
		if (sent_bytes < 0){
			m_on_fail(a_sock, std::string{strerror(errno)}, m_context);
			return ServerTCP_Status::SERVER_SEND_FAIL;
		}
	}
	return ServerTCP_Status::SERVER_SUCCESS;
}

template <typename Context>
void ServerTCP<Context>::close_all(Client &a_client)
{
	close(a_client.socket());
	m_close_client(a_client.socket(), m_context);
}

template <typename Context>
void ServerTCP<Context>::add_client(struct sockaddr_in a_sin, int a_sock)
{
	Client client{a_sock, a_sin};
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
				FD_CLR(client.socket(), m_server_data.fd_main());
				std::thread{read_incomming_heavy, this, &client}.detach();
			}	
		} else {
			read_incomming_data(client);
		}
	}
}

template <typename Context>
void ServerTCP<Context>::socket_set(Client &a_client)
{
	FD_SET(a_client.socket(), m_server_data.fd_main());
}

template <typename Context>
void ServerTCP<Context>::remove_client(Client &a_client)
{
	for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
		if (it->socket() == a_client.socket()) {
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
int ServerTCP<Context>::read_incomming_data(Client &a_client)
{
	int digit_number = 5;
	if (FD_ISSET(a_client.socket(), m_server_data.fd_copy())){
		try{
			size_t read_already = recieve_from_client(a_client.socket(), m_buffer.buffer(), m_buffer.buffer_size(), 0, &a_client);
			size_t read_remain = get_total_data_size(m_buffer.buffer(), read_already, digit_number);
			while(read_already < read_remain){
				read_already += recieve_from_client(a_client.socket(), m_buffer.buffer(), m_buffer.buffer_size(), read_already, &a_client);
			}
			if(read_already > 1000){
				a_client.heavy() = true;
			}
			*(m_buffer.buffer(read_already))='\0';
			if (read_already > 0){
				m_got_message(a_client, a_client.socket(), m_buffer.buffer(4+digit_number), read_already-4-digit_number, m_context);
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
	size_t buffer_size = a_server->m_buffer.buffer_size();
	BufferManager local_buff{buffer_size};
	int digit_number = 5;
	for(;;){
		try{
			size_t read_already = a_server->recieve_from_client(_client->socket(), local_buff.buffer(), buffer_size, 0, _client, true);
			std::string data{local_buff.buffer(), read_already};
			size_t read_remain = a_server->get_total_data_size(local_buff.buffer(), read_already, digit_number);
			while(read_already < read_remain){
				read_already += a_server->recieve_from_client(_client->socket(), local_buff.buffer(), buffer_size, read_already, _client, true);
			}
			*(local_buff.buffer(read_already))='\0';
			if (read_already > 0){
				a_server->m_got_message(*_client, _client->socket(), local_buff.buffer(4+digit_number), read_already-4-digit_number, a_server->m_context);
			}
		} catch(...) {
			return;
		}
	}
	
}

template <typename Context>
inline bool ServerTCP<Context>::client_no_recieve(int a_result, Client &a_client)
{
	if(a_result <= 0){
		m_on_fail(a_client.socket(), strerror(errno), m_context);
		m_close_client(a_client.socket(), m_context);
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
		m_on_fail(a_client->socket(), strerror(errno), m_context);
		m_close_client(a_client->socket(), m_context);
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
void ServerTCP<Context>::close_client(Client &a_client)
{
	close(a_client.socket());
	m_server_data.client_down();
	m_on_fail(a_client.socket(), strerror(errno), m_context);
	m_close_client(a_client.socket(), m_context);
	remove_client(a_client);
}

template <typename Context>
void ServerTCP<Context>::clear_buffer()
{
	m_buffer.clear_buffer();
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
	return m_buffer.buffer();
}

#endif /* TCP_SERVER_HPP */
