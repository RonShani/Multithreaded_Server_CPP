#ifndef REMOTE_AI_SERVER_HPP
#define REMOTE_AI_SERVER_HPP

#include <map>
#include <list>
#include <string>
#include <mutex>
#include <thread>

#include "tcp_server.hpp"
#include "topic.hpp"
#include "client.hpp"


class RemoteAIServer{
public:
    RemoteAIServer(int a_port);
    ~RemoteAIServer();

private:
    static void read_income(Client &a_client, int _id, char *_msg, int _length, RemoteAIServer &_context);
    static void close_client(int _id, RemoteAIServer &_context);
    static bool new_client(int _id, RemoteAIServer &_context);
    static void on_fail(int _id, std::string const &_err, RemoteAIServer &_context);
    static void threaded_notifier(RemoteAIServer *a_server);
    static void threaded_run_server(RemoteAIServer *a_server);
    static void wait_untill_available(RemoteAIServer *a_server);
    bool handle_subscription(RemoteAIServer &a_server, Client &a_client, char *a_msg, int a_length);
    bool is_publish(char *a_msg, int a_length);
    bool is_topic_listened(Topic &a_topic);
    void notify_all_subscribers(Topic &a_topic);
    void remove_subscriber(int a_client_socket);
    void send_immediate_message(std::string const &a_message);
    void subscribe_topic(RemoteAIServer &a_server, Client &a_client, Topic a_topic);

private:
    ServerTCP<RemoteAIServer> m_server;
    std::list<Topic> m_objects;
    std::map<Topic, std::list<Client>> m_subscribers;
    std::thread m_main_thread;
    std::thread m_publisher;
};


#endif //REMOTE_AI_SERVER_HPP