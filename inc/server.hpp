#ifndef REMOTE_AI_SERVER_HPP
#define REMOTE_AI_SERVER_HPP

#include <map>
#include <string>

#include "tcp_server.hpp"
#include "topic.hpp"
#include "found_object.hpp"
#include "subscriber.hpp"
#include "client.hpp"

class RemoteAIServer{
public:
    RemoteAIServer(int a_port);

private:
    static void read_income(Client &a_client, int _id, char *_msg, int _length, RemoteAIServer &_context);
    static void close_client(int _id, RemoteAIServer &_context);
    static bool new_client(int _id, RemoteAIServer &_context);
    static void on_fail(int _id, std::string const &_err, RemoteAIServer &_context);
    bool is_subscribe(std::string const &a_msg);
    bool is_publish(std::string const &a_msg);
    Topic get_topic(std::string const &a_msg);
    void set_topic_object(std::string const &a_msg, Topic &a_topic);
    void set_topic_object(std::string const &a_msg, Topic &a_topic, size_t a_length);
    bool is_topic_listened(Topic &a_topic);
    void notify_all_subscribers(Topic const &a_topic);
    void notify_all_subscribers(Topic const &a_topic, std::string const &a_msg);
    void remove_subscriber(int a_client_socket);
    friend class ServerTCP<RemoteAIServer>;

private:
    ServerTCP<RemoteAIServer> m_server;
    std::map<Topic, FoundObject> m_objects;
    std::map<Topic, std::vector<Client>> m_subscribers;
    void send_immediate_message(std::string const &a_message);
    const std::string m_ok = "ok";
};


#endif //REMOTE_AI_SERVER_HPP