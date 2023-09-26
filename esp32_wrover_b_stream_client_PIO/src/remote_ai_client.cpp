#include "remote_ai_client.hpp"
#include "functionality.hpp"

RemoteAIClient::RemoteAIClient(const char *a_ssid, const char *a_pass, ok_func a_signal_ok)
: m_wifimulti{}, m_client{}, m_topics{}, m_host{}, m_port{0}, m_signal_ok(a_signal_ok), m_topics_context{(void*)this}
{
    m_wifimulti.addAP(a_ssid, a_pass);
    initialize_client();
}

void RemoteAIClient::connect_host(const char *host, const uint16_t port)
{
    Serial.printf("connecting %s on port %d\n", host, port);
    m_client.connect(host, port);
    Serial.printf("Connected %d\n", m_client.state());
    m_client.setNoDelay(true);
    if(port != m_port){
        m_host.fromString(host);
        m_port = port;
    }
    if(m_topics.size() > 0){
        resubscribe_topics();
    }
}

void RemoteAIClient::connect_host(IPAddress const &a_ip, const uint16_t a_port)
{
    connect_host(a_ip.toString().c_str(), a_port);
}

void RemoteAIClient::begin()
{
    while(m_wifimulti.run() != WL_CONNECTED) {
        delay(500);
    }
}

void RemoteAIClient::add_topic(String const &a_topic, Functionality *a_func, bool a_is_first_time)
{
    for(auto &topic : m_topics_context.topics()){
        do{
            publish("/sub/"+topic.first);
            delay(250);
        }while(!m_topics_context.validate_subject(topic.first));
    }
    if (a_is_first_time){
        m_topics[a_topic] = a_func;
    }
    
}

void RemoteAIClient::remove_topic(String const &a_topic)
{
    for(std::map<String, Functionality*>::const_iterator i = m_topics.cbegin(); i != m_topics.cend(); ++i){
        if(i->first.compareTo(a_topic) == 0){
            m_topics.erase(i);
            return;
        }
    }
}

void RemoteAIClient::resubscribe_topics()
{
    for(auto &topic : m_topics){
        add_topic(topic.first, nullptr, false);
    }
}

void RemoteAIClient::topic_loader(std::vector<String> &a_topics, std::vector<Functionality> &a_func_vector)
{
    m_topics_context.clear_validations();
    m_topics_context.validation() = true;
    for(size_t i = 0; i < a_topics.size(); ++i){
        m_topics_context.add_subject(a_topics[i]);
        add_topic(a_topics[i], &a_func_vector[i]);
        m_signal_ok(a_topics[i]);
    }
    m_topics_context.validation() = false;
}

void RemoteAIClient::addAP(const char *a_ssid, const char *a_pass)
{
    m_wifimulti.addAP(a_ssid, a_pass);
}

size_t RemoteAIClient::topics_size() const
{
    return m_topics.size();
}

void RemoteAIClient::send_as_is(unsigned char *a_msg, unsigned long a_len)
{
    m_client.write((char*)a_msg, a_len);
}

void RemoteAIClient::check_connection()
{
    if(m_client.connected() == 0){
        connect_host(m_host, m_port);
    }
}

void RemoteAIClient::act_upon_topic(String const &a_line)
{
    if(m_topics.find(a_line) == m_topics.end()){
        return;
    }
    m_topics[a_line]->run();
}

void RemoteAIClient::subscribed_ok(String const &a_msg)
{
  Serial.print("subscribed ");
  Serial.println(a_msg);
}

void RemoteAIClient::publish(const char *buf, size_t size)
{
    write_pre_msg(size);
    m_client.write(buf, size);
}

void RemoteAIClient::publish(String const &a_string)
{
    write_pre_msg(a_string.length());
    m_client.write(a_string.c_str(), a_string.length());
}

void RemoteAIClient::publish_two(const char *a_msg, size_t a_len, const char *b_msg, size_t b_len)
{
    write_pre_msg(a_len+b_len);
    m_client.write(a_msg, a_len);
    m_client.write(b_msg, b_len);
}

size_t RemoteAIClient::pre_msg(unsigned long a_len, char * pre)
{
    a_len+=10;
    int pos = 6 - int(log10(a_len));
    sprintf(&pre[pos], "%lu<<", a_len);
    return a_len+9;
}

void RemoteAIClient::data_handler(void *a_context, AsyncClient *a_client, void *a_data, size_t len)
{
    DataContext *ctx = (DataContext*)a_context;
    if(ctx->validation()){
        ctx->validate_subject((char*)a_data, len);
    } else {
        String line = ctx->to_string((char*)a_data, len);
        if(line[0] == '@'){
            //Serial.printf("topic is: %s\n", line.substring(1, line.length()-1));
            ((RemoteAIClient*)ctx->client())->act_upon_topic(line.substring(1, line.length()-1));
        }
    }
}

void RemoteAIClient::error_handler(void *a_context, AsyncClient *a_client, int8_t error)
{
    Serial.printf("Error: %s\n", a_client->errorToString(error));
}

void RemoteAIClient::connect_handler(void *a_context, AsyncClient *a_client)
{
    DataContext *ctx = (DataContext*)a_context;
    Serial.printf("disconnected! trying to reconnect...\n");
    ((RemoteAIClient*)ctx->client())->reconnect_host();
}

void RemoteAIClient::write_pre_msg(unsigned long a_len)
{
    char pre[10] = ">>00000<<";
    pre_msg(a_len, pre);
    m_client.write(pre, 10);
}

void RemoteAIClient::initialize_client()
{
    m_client.onData(RemoteAIClient::data_handler, &m_topics_context);
    m_client.onError(RemoteAIClient::error_handler, &m_topics_context);
    m_client.onDisconnect(RemoteAIClient::connect_handler, &m_topics_context);
}

void RemoteAIClient::reconnect_host()
{
    m_client.close();
    while(!m_client.connected()){
        delay(10);
        connect_host({10,0,0,38}, m_port);   
        yield();
    }
}

const char *RemoteAIClient::rv() const
{
    return m_topics_context.rv();
}