#include "server.hpp"
#include "topic.hpp"
#include "timer.hpp"
#include "char_str.hpp"
#include "server_static_strings.hpp"

RemoteAIServer::RemoteAIServer(int a_port)
: m_server{a_port, 128, read_income, close_client, new_client, on_fail, *this, 1024*10}
, m_objects{}
, m_subscribers{}
, m_main_thread{RemoteAIServer::threaded_run_server, this}
, m_publisher{RemoteAIServer::threaded_notifier, this}
{}

RemoteAIServer::~RemoteAIServer()
{
    std::cout<<"ended server\n";
    m_main_thread.join();
    m_publisher.join();
}

void RemoteAIServer::read_income(Client &a_client, int _id, char *_msg, int _length, RemoteAIServer &a_server)
{
    if(a_server.handle_subscription(a_server, a_client, _msg, _length)){
       a_server.m_server.send_message(m_ok, _id);
       return;
    } 
    else if(a_server.is_publish(_msg, _length)){
        //a_server.m_server.send_message(m_ok, _id);
    } else {
        a_server.m_server.send_message("no action", _id);
    }
}
void RemoteAIServer::close_client(int _id, RemoteAIServer &_context)
{
    _context.remove_subscriber(_id);
    std::cerr << "Client " <<_id<<" closed"<<std::endl;
}

bool RemoteAIServer::new_client(int _id, RemoteAIServer &a_server)
{
    a_server.m_server.send_message(m_ok, _id);
    return true;
}

void RemoteAIServer::on_fail(int _id, std::string const &_err, RemoteAIServer &_context)
{
    _context.remove_subscriber(_id);
    std::cerr << _err<<std::endl;
}

void RemoteAIServer::threaded_notifier(RemoteAIServer *a_server)
{
    RemoteAIServer::wait_untill_available(a_server);
    try{
        for(;;){
            while(a_server->m_objects.size() == 0) __builtin_ia32_pause();
            for(std::list<Topic>::iterator topic = a_server->m_objects.begin(); topic != a_server->m_objects.end(); ++topic){
                if(topic->is_published()){
                    a_server->m_objects.erase(topic);
                    break;
                } else {
                    for(auto &client : a_server->m_subscribers[topic->name()]){
                        a_server->m_server.send_message_raw(topic->data().data(), topic->data().length(), client.socket());
                    }
                    topic->set_published();
                }
            }
            __builtin_ia32_pause();
        }
    } catch(...) {
        std::cout<<"publisher failed\n";
        threaded_notifier(a_server);
    }
}

void RemoteAIServer::threaded_run_server(RemoteAIServer *a_server)
{
    RemoteAIServer::wait_untill_available(a_server);
    a_server->m_server.run_server();
}

void RemoteAIServer::wait_untill_available(RemoteAIServer *a_server)
{
    while(!a_server){
        Timer timer;
        while (!timer.is_passed_ms(500)){
            __builtin_ia32_pause();
        }
        __builtin_ia32_pause();
    }
}

void RemoteAIServer::subscribe_topic(RemoteAIServer &a_server, Client &a_client, Topic a_topic)
{
    a_server.m_subscribers[a_topic].emplace_back(a_client);
    a_server.m_server.send_message(a_topic.name(), a_client.socket());
    if(a_topic.name()=="robot_view"){
        a_server.send_immediate_message("PUB/askstream/pub");
    }
    a_server.m_subscribers[a_topic].unique();
    std::cout<<"Subscribed to "<<a_topic.name()<<std::endl;
}

bool RemoteAIServer::handle_subscription(RemoteAIServer &a_server, Client &a_client, char *a_msg, int a_length)
{
    size_t from, to = 0;
    if(topic_from_to(a_msg, a_length, from, to, sub_pre, sub_post)){
        subscribe_topic(a_server, a_client, {a_msg, a_length});
        return true;
    }
    return false;
}

bool RemoteAIServer::is_publish(char *a_msg, int a_length)
{
    size_t from, to = 0;
    if(topic_from_to(a_msg, a_length, from, to, pub_pre, pub_post)){
        m_objects.emplace_back(&a_msg[from], to-from);
        return true;
    }
    return false;
}

bool RemoteAIServer::is_topic_listened(Topic &a_topic)
{
    auto subs_topic = m_subscribers.find(a_topic);
    if(subs_topic != m_subscribers.end()){
        return true;
    }
    return false;
}

void RemoteAIServer::notify_all_subscribers(Topic &a_topic)
{
    for(auto &client : m_subscribers[a_topic] ){
        m_server.send_message_raw(a_topic.data().data(), a_topic.data().length(), client.socket());
    }
}

void RemoteAIServer::remove_subscriber(int a_client_socket)
{
    for(auto &topic : m_subscribers){
        for(std::list<Client>::iterator itr = topic.second.begin(); itr != topic.second.end(); ++itr){
            if(a_client_socket == itr->socket()){
                topic.second.erase(itr);
                if(topic.first == "robot_view" && topic.second.empty()){
                    send_immediate_message("PUB/askstopstream/pub");
                }
                return;
            }
        }
    }
}

void RemoteAIServer::send_immediate_message(std::string const &a_message)
{
    Topic inst_msg{a_message};
    notify_all_subscribers(inst_msg);
}