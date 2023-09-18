#include "server.hpp"
#include "found_object.hpp"
#include "topic.hpp"

RemoteAIServer::RemoteAIServer(int a_port)
: m_server{a_port, 128, read_income, close_client, new_client, on_fail, *this, 1024*1024}
, m_objects{}
, m_subscribers{}
{
    m_server.run_server();
}

void RemoteAIServer::read_income(Client &a_client, int _id, char *_msg, int _length, RemoteAIServer &a_server)
{
    std::string seq{_msg, size_t(_length)};
    Topic topic_name;
    if(a_server.is_subscribe(seq)){
        Topic new_sub{a_server.get_topic(seq)};
        a_server.m_subscribers[new_sub].emplace_back(_id, a_client.addr());
        a_server.m_server.send_message(new_sub.name(), _id);
        if(new_sub.name()=="robot_view"){
            a_server.send_immediate_message("@askstream");
        }
        return;
    }
    else if(a_server.is_publish(seq)){
        a_server.set_topic_object(seq,topic_name,_length);
        a_server.m_server.send_message(a_server.m_ok, _id);
    }
    if(topic_name && a_server.is_topic_listened(topic_name)){
        a_server.notify_all_subscribers(topic_name);
    }
}

void RemoteAIServer::close_client(int _id, RemoteAIServer &_context)
{
    _context.remove_subscriber(_id);
    std::cerr << "Client " <<_id<<" closed"<<std::endl;
}

int RemoteAIServer::new_client(int _id, RemoteAIServer &a_server)
{
    a_server.m_server.send_message(a_server.m_ok, _id);
    return 1;
}

void RemoteAIServer::on_fail(int _id, std::string const &_err, RemoteAIServer &_context)
{
    (void)_context;
    _context.remove_subscriber(_id);
    std::cerr << _err<<std::endl;
}

bool RemoteAIServer::is_subscribe(std::string const &a_msg)
{
    size_t data_index = a_msg.find("sub/");
    if(data_index != std::string::npos){
        return true;
    }
    return false;
}

bool RemoteAIServer::is_publish(std::string const &a_msg)
{
    size_t data_index = a_msg.find("@");
    if(data_index != std::string::npos){
        return true;
    }
    return false;
}

Topic RemoteAIServer::get_topic(std::string const &a_msg)
{
    size_t start = a_msg.find("sub/");
    std::string topic_name = a_msg.substr(start+4);
    return Topic{topic_name};
}

void RemoteAIServer::set_topic_object(std::string const &a_msg, Topic &a_topic)
{
    size_t index_start = a_msg.find('@');
    size_t data_index = a_msg.find('|');
    if(data_index != std::string::npos){
        a_topic = Topic{a_msg.substr(index_start+1, data_index-index_start-1)};
        FoundObject a_object{a_msg.substr(1)};
        m_objects[a_topic] = a_object;
    }
}

void RemoteAIServer::set_topic_object(std::string const &a_msg, Topic &a_topic, size_t a_length)
{
    (void)a_length;
    size_t index_start = a_msg.find('@');
    size_t data_index = a_msg.find('|');
    if(data_index != std::string::npos){
        std::string topic_name = a_msg.substr(index_start+1, data_index-index_start-1);
        a_topic = Topic{topic_name};
        FoundObject a_object{a_msg};
        m_objects[a_topic] = a_object;
    }
}

bool RemoteAIServer::is_topic_listened(Topic &a_topic)
{
    auto subs_topic = m_subscribers.find(a_topic);
    if(subs_topic != m_subscribers.end()){
        return true;
    }
    return false;
}

void RemoteAIServer::notify_all_subscribers(Topic const &a_topic)
{
    for(auto &client : m_subscribers[a_topic] ){
        m_server.send_message_raw(m_objects[a_topic].data().data(), m_objects[a_topic].data().length(), client.socket());
    }
}

void RemoteAIServer::notify_all_subscribers(Topic const &a_topic, std::string const &a_msg)
{
    for(auto &client : m_subscribers[a_topic] ){
        m_server.send_message(a_topic.name()+"|"+a_msg,client.socket());
    }
}

void RemoteAIServer::remove_subscriber(int a_client_socket)
{
    for(auto &topic : m_subscribers){
        for(std::vector<Client>::iterator itr = topic.second.begin(); itr != topic.second.end(); ++itr){
            if(a_client_socket == itr->socket()){
                topic.second.erase(itr);
                if(topic.first == "robot_view" && topic.second.empty()){
                    send_immediate_message("@askstopstream");
                }
                return;
            }
        }
    }
}

void RemoteAIServer::send_immediate_message(std::string const &a_message)
{
    Topic inst_msg{a_message};
    set_topic_object(a_message+"|",inst_msg,a_message.size()+1);
    notify_all_subscribers(inst_msg);
}
